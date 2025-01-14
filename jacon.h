#ifndef JACON_H
#define JACON_H

#include "data_structures.h"
#include <stddef.h>
#include <stdbool.h>

// Error codes
typedef enum {
    JACON_OK,
    JACON_END_OF_INPUT,
    JACON_NULL_PARAM,
    JACON_ERR_INVALID_JSON,
    JACON_ERR_CHAR_NOT_FOUND,
    JACON_ALLOC_ERROR,
} Jacon_Error;

// Value types
typedef enum {
    JACON_VALUE_OBJECT,
    JACON_VALUE_ARRAY,
    JACON_VALUE_STRING,
    JACON_VALUE_INT,
    JACON_VALUE_FLOAT,
    JACON_VALUE_DOUBLE,
    JACON_VALUE_BOOLEAN,
    JACON_VALUE_NULL,
} Jacon_ValueType;

typedef struct {
    union {
        char* string_val;
        union {
            int int_val;
            float float_val;
            double double_val;
        };
        bool* bool_val;
    };
} Jacon_Value;

typedef struct Jacon_Node Jacon_Node;
struct Jacon_Node {
    Jacon_Node* parent;
    char* name;
    Jacon_Value value;
    Jacon_Node* childs;
    size_t child_count;
    // Dictionary for efficient value retrieving
    HashMap entries;
};

// Tokenizer
#define JACON_TOKENIZER_DEFAULT_CAPACITY 256
#define JACON_TOKENIZER_DEFAULT_RESIZE_FACTOR 2

typedef enum {
    JACON_TOKEN_STRING,
    JACON_TOKEN_INT,
    JACON_TOKEN_FLOAT,
    JACON_TOKEN_DOUBLE,
    JACON_TOKEN_BOOLEAN,
    JACON_TOKEN_ARRAY_START,
    JACON_TOKEN_ARRAY_END,
    JACON_TOKEN_OBJECT_START,
    JACON_TOKEN_OBJECT_END,
    JACON_TOKEN_NULL,
    JACON_TOKEN_COLON,
    JACON_TOKEN_COMMA,
} Jacon_TokenType;

typedef struct {
    Jacon_TokenType type;
    union {
        char* string_val;
        int int_val;
        float float_val;
        double double_val;
        struct {
            double base;
            double exponent;
        } exponential;
        bool bool_val;
    };
} Jacon_Token;

typedef struct {
    size_t count;
    size_t capacity;
    Jacon_Token* tokens;
} Jacon_Tokenizer;

// Parser
typedef struct {
    Jacon_Tokenizer tokenizer;
    Jacon_Node* root;
} Jacon_Parser;

/**
 * Validate a Json string input
 * Returns:
 * - JACON_OK if valid
 * - According error code otherwise
 */
Jacon_Error
Jacon_validate_input(Jacon_Tokenizer* tokenizer);

/**
 * Parse a Json string input into a queryable object
 */
Jacon_Error
Jacon_parse_input(Jacon_Node* root, const char* str);

#endif // JACON_H
#define JACON_IMPLEMENTATION
#ifdef JACON_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

// Debug print token
void 
Jacon_print_token(const Jacon_Token* token) 
{
    switch (token->type) {
        case JACON_TOKEN_STRING:
            printf("Token Type: STRING, Value: %s\n", token->string_val);
            break;
        case JACON_TOKEN_INT:
            printf("Token Type: INT, Value: %d\n", token->int_val);
            break;
        case JACON_TOKEN_FLOAT:
            printf("Token Type: FLOAT, Value: %f\n", token->float_val);
            break;
        case JACON_TOKEN_DOUBLE:
            printf("Token Type: DOUBLE, Value: %lf\n", token->double_val);
            break;
        case JACON_TOKEN_BOOLEAN:
            printf("Token Type: BOOLEAN, Value: %s\n", token->bool_val ? "true" : "false");
            break;
        case JACON_TOKEN_ARRAY_START:
            printf("Token Type: ARRAY_START\n");
            break;
        case JACON_TOKEN_ARRAY_END:
            printf("Token Type: ARRAY_END\n");
            break;
        case JACON_TOKEN_OBJECT_START:
            printf("Token Type: OBJECT_START\n");
            break;
        case JACON_TOKEN_OBJECT_END:
            printf("Token Type: OBJECT_END\n");
            break;
        case JACON_TOKEN_NULL:
            printf("Token Type: NULL VALUE\n");
            break;
        case JACON_TOKEN_COLON:
            printf("Token Type: COLON\n");
            break;
        case JACON_TOKEN_COMMA:
            printf("Token Type: COMMA\n");
            break;
        default:
            printf("Unknown Token Type\n");
            break;
    }
}

// Debug print tokenizer
void 
Jacon_print_tokenizer(const Jacon_Tokenizer* tokenizer) 
{
    printf("Tokenizer contains %zu tokens:\n", tokenizer->count);
    for (size_t i = 0; i < tokenizer->count; ++i) {
        printf("Token %zu: ", i);
        Jacon_print_token(&tokenizer->tokens[i]);
    }
}

Jacon_Error
Jacon_tokenizer_init(Jacon_Tokenizer* tokenizer)
{
    tokenizer->tokens = (Jacon_Token*)calloc(
        JACON_TOKENIZER_DEFAULT_CAPACITY, sizeof(Jacon_Token));
    if (tokenizer->tokens == NULL) {
        perror("Jacon_append_token array alloc error");
        return JACON_ALLOC_ERROR;
    }
    tokenizer->capacity = JACON_TOKENIZER_DEFAULT_CAPACITY;
    tokenizer->count = 0;
    return JACON_OK;
}

Jacon_Error
Jacon_append_token(Jacon_Tokenizer* tokenizer, Jacon_Token token)
{
    if (tokenizer == NULL) {
        return JACON_NULL_PARAM;
    }
    if (tokenizer->tokens == NULL) {
        tokenizer->tokens = (Jacon_Token*)calloc(
            JACON_TOKENIZER_DEFAULT_CAPACITY, sizeof(Jacon_Token));
        if (tokenizer->tokens == NULL) {
            perror("Jacon_append_token array alloc error");
            return JACON_ALLOC_ERROR;
        }
        tokenizer->capacity = JACON_TOKENIZER_DEFAULT_CAPACITY;
    }
    if (tokenizer->count == tokenizer->capacity) {
        Jacon_Token* tmp = (Jacon_Token*)realloc(
            tokenizer->tokens, 
            tokenizer->capacity * JACON_TOKENIZER_DEFAULT_RESIZE_FACTOR * sizeof(Jacon_Token*));
        if (tmp == NULL) {
            perror("Jacon_append_token array realloc error");
            return JACON_ALLOC_ERROR;
        }
        tokenizer->tokens = tmp;
        tokenizer->capacity *= JACON_TOKENIZER_DEFAULT_RESIZE_FACTOR;
    }

    tokenizer->tokens[tokenizer->count++] = token;
    return JACON_OK;
}

Jacon_Error 
Jacon_parse_token(Jacon_Token* token, const char** str) 
{
    // printf("working str: %s\n", *str);
    switch (**str) {
        case ',':
            token->type = JACON_TOKEN_COMMA;
            (*str)++;
            break;
        case ':':
            token->type = JACON_TOKEN_COLON;
            (*str)++;
            break;
        case '{':
            token->type = JACON_TOKEN_OBJECT_START;
            (*str)++;
            break;
        case '}':
            token->type = JACON_TOKEN_OBJECT_END;
            (*str)++;
            break;
        case '[':
            token->type = JACON_TOKEN_ARRAY_START;
            (*str)++;
            break;
        case ']':
            token->type = JACON_TOKEN_ARRAY_END;
            (*str)++;
            break;
        case '"':
            (*str)++; // Move past the initial quote
            const char* string_end = strchr(*str, '"');
            if (string_end == NULL) return JACON_ERR_CHAR_NOT_FOUND;
            token->type = JACON_TOKEN_STRING;
            size_t string_size = string_end - *str;
            token->string_val = (char*)calloc(string_size + 1, sizeof(char));
            if (token->string_val == NULL) return JACON_ALLOC_ERROR;
            strncpy(token->string_val, *str, string_size);
            token->string_val[string_size] = '\0';
            *str = string_end + 1; // Move past the closing quote
            break;
        case('n'):
            if (strncmp(*str, "null", 4) != 0) return JACON_ERR_INVALID_JSON;
            token->type = JACON_TOKEN_NULL;
            (*str) += 4;
            break;
        case('t'):
            if (strncmp(*str, "true", 4) != 0) return JACON_ERR_INVALID_JSON;
            token->type = JACON_TOKEN_BOOLEAN;
            token->bool_val = true;
            (*str) += 4;
            break;
        case('f'):
            if (strncmp(*str, "false", 5) != 0) return JACON_ERR_INVALID_JSON;
            token->type = JACON_TOKEN_BOOLEAN;
            token->bool_val = false;
            (*str) += 5;
            break;
        case('\0'):
            return JACON_END_OF_INPUT;
        default:
            if(isspace(**str)) {
                (*str)++;
                return Jacon_parse_token(token, str);
            }
            if ((**str >= '0' && **str <= '9') || **str == '-') {
                char *endptr;

                // Parse an int
                int ival = (int)strtol(*str, &endptr, 10);
                printf("%zu\n", *endptr);
                if (isspace(*endptr) || *endptr == ',' || *endptr == ']' || *endptr == '}'
                 && ival >= INT_MIN && ival <= INT_MAX) {
                    token->int_val = ival;
                    token->type = JACON_TOKEN_INT;
                    *str = endptr;
                    puts("===========");
                    printf("%c\n", *endptr);
                    break;
                }

                // Parse a double and check for float cast
                double dval = strtod(*str, &endptr);
                if(!isspace(*endptr) && *endptr != ',' && *endptr != ']' && *endptr != '}')
                    return JACON_ERR_INVALID_JSON;

                if (fabs(dval) <= FLT_MAX && fabs(dval) >= FLT_MIN) {
                    float fval = (float)dval;
                    if (fabs(dval - fval) <= FLT_EPSILON) {
                        token->type = JACON_TOKEN_FLOAT;
                        token->float_val = fval;
                    } else {
                        token->type = JACON_TOKEN_DOUBLE;
                        token->double_val = dval;
                    }
                } else {
                    token->type = JACON_TOKEN_DOUBLE;
                    token->double_val = dval;
                }
                *str = endptr;
            }
            else return JACON_ERR_INVALID_JSON;
    }
    return JACON_OK;
}

Jacon_Error Jacon_tokenize(Jacon_Tokenizer* tokenizer, const char* str) {
    Jacon_Error ret;

    while (*str) {
        Jacon_Token token;
        ret = Jacon_parse_token(&token, &str);
        if (ret != JACON_OK) return ret;
        if (token.type == JACON_TOKEN_INT) puts("found int");
        ret = Jacon_append_token(tokenizer, token);
        if (ret != JACON_OK) return ret;
    }

    return JACON_OK;
}

Jacon_Error
Jacon_validate_input(Jacon_Tokenizer* tokenizer)
{
    return JACON_OK;
}

Jacon_Error
Jacon_parse_input(Jacon_Node* root, const char* str)
{
    Jacon_Tokenizer tokenizer;
    Jacon_tokenizer_init(&tokenizer);
    Jacon_Error ret;
    ret = Jacon_tokenize(&tokenizer, str);
    if (ret != JACON_OK) return ret;

    Jacon_print_tokenizer(&tokenizer);

    // TODO : validate json input
    ret = Jacon_validate_input(&tokenizer);
    if (ret != JACON_OK) return ret;

    // For now we will say that every things that goes through here is a valid json input

    return JACON_OK;
}

#endif //JACON_IMPLEMENTATION