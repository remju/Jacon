#ifndef JACON_H
#define JACON_H

#include "data_structures.h"
#include <stddef.h>
#include <stdbool.h>

#define JACON_TOKENIZER_DEFAULT_CAPACITY 256
#define JACON_TOKENIZER_DEFAULT_RESIZE_FACTOR 2
#define JACON_NODE_DEFAULT_CHILD_CAPACITY 1
#define JACON_NODE_DEFAULT_RESIZE_FACTOR 2

// Error codes
typedef enum {
    JACON_OK,
    JACON_END_OF_INPUT,
    JACON_NO_MORE_TOKENS,
    JACON_ERR_INDEX_OUT_OF_BOUND,
    JACON_ERR_NULL_PARAM,
    JACON_ERR_INVALID_VALUE_TYPE,
    JACON_ERR_EMPTY_INPUT,
    JACON_ERR_INVALID_JSON,
    JACON_ERR_INVALID_ESCAPE_SEQUENCE,
    JACON_ERR_CHAR_NOT_FOUND,
    JACON_ERR_MEMORY_ALLOCATION,
    JACON_ERR_UNREACHABLE_STATEMENT,
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
        bool bool_val;
    };
} Jacon_Value;

typedef struct Jacon_Node Jacon_Node;
struct Jacon_Node {
    Jacon_Node* parent;
    char* name;
    Jacon_ValueType type;
    Jacon_Value value;
    Jacon_Node* childs;
    size_t child_count;
    size_t child_capacity;
};

typedef struct Jacon_content {
    Jacon_Node* root;
    // Dictionary for efficient value retrieving
    HashMap entries;
} Jacon_content;

// Tokenizer
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
Jacon_parse_input(Jacon_content* content, const char* str);

/**
 * Get the string representation of a value type
 */
Jacon_Error
Jacon_value_type_to_str(Jacon_ValueType type, char** str);

#endif // JACON_H

#ifdef JACON_IMPLEMENTATION

#define JUTILS_IMPLEMENTATION
#include "jutils.h"
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

void 
Jacon_print_node_value(Jacon_Value value, Jacon_ValueType type) 
{
    switch (type) {
        case JACON_VALUE_STRING:
            printf("\"%s\"", value.string_val);
            break;
        case JACON_VALUE_INT:
            printf("%d", value.int_val);
            break;
        case JACON_VALUE_FLOAT:
            printf("%f", value.float_val);
            break;
        case JACON_VALUE_DOUBLE:
            printf("%lf", value.double_val);
            break;
        case JACON_VALUE_BOOLEAN:
            printf("%s", value.bool_val ? "true" : "false");
            break;
        case JACON_VALUE_NULL:
            printf("null");
            break;
        case JACON_VALUE_OBJECT:
        case JACON_VALUE_ARRAY:
            break;
    }
}

void 
Jacon_print_node(Jacon_Node* node, int indent) 
{
    // Print indentation
    for (int i = 0; i < indent; ++i) {
        printf("  ");
    }

    // Print node name
    if (node->name) {
        printf("\"%s\": ", node->name);
    }

    // Print node value
    switch (node->type) {
        case JACON_VALUE_OBJECT:
            printf("{\n");
            for (size_t i = 0; i < node->child_count; ++i) {
                Jacon_print_node(&node->childs[i], indent + 1);
                if (i < node->child_count - 1) {
                    printf(",");
                }
                printf("\n");
            }
            for (int i = 0; i < indent; ++i) {
                printf("  ");
            }
            printf("}");
            break;
        case JACON_VALUE_ARRAY:
            printf("[\n");
            for (size_t i = 0; i < node->child_count; ++i) {
                Jacon_print_node(&node->childs[i], indent + 1);
                if (i < node->child_count - 1) {
                    printf(",");
                }
                printf("\n");
            }
            for (int i = 0; i < indent; ++i) {
                printf("  ");
            }
            printf("]");
            break;
        case JACON_VALUE_STRING:
        case JACON_VALUE_INT:
        case JACON_VALUE_FLOAT:
        case JACON_VALUE_DOUBLE:
        case JACON_VALUE_BOOLEAN:
        case JACON_VALUE_NULL:
        default:
            Jacon_print_node_value(node->value, node->type);
            break;
    }
}

Jacon_Error
Jacon_value_type_to_str(Jacon_ValueType type, char** str)
{
    switch (type) {
        case JACON_VALUE_STRING:
            *str = "string";
            break;
        case JACON_VALUE_INT:
            *str = "int";
            break;
        case JACON_VALUE_FLOAT:
            *str = "float";
            break;
        case JACON_VALUE_DOUBLE:
            *str = "double";
            break;
        case JACON_VALUE_BOOLEAN:
            *str = "boolean";
            break;
        case JACON_VALUE_NULL:
            *str = "null";
            break;
        case JACON_VALUE_ARRAY:
            *str = "array";
            break;
        case JACON_VALUE_OBJECT:
            *str = "object";
            break;
        default:
            return JACON_ERR_INVALID_VALUE_TYPE;
    }
    return JACON_OK;
}

Jacon_Error
Jacon_tokenizer_init(Jacon_Tokenizer* tokenizer)
{
    tokenizer->tokens = (Jacon_Token*)calloc(
        JACON_TOKENIZER_DEFAULT_CAPACITY, sizeof(Jacon_Token));
    if (tokenizer->tokens == NULL) {
        perror("Jacon_append_token array alloc error");
        return JACON_ERR_MEMORY_ALLOCATION;
    }
    tokenizer->capacity = JACON_TOKENIZER_DEFAULT_CAPACITY;
    tokenizer->count = 0;
    return JACON_OK;
}

Jacon_Error
Jacon_init_content(Jacon_content* content)
{
    content->root = (Jacon_Node*)calloc(1, sizeof(Jacon_Node));
    if (content->root == NULL) return JACON_ERR_MEMORY_ALLOCATION;
    return JACON_OK;
}

void
Jacon_free_tokenizer(Jacon_Tokenizer* tokenizer)
{
    for (size_t i = 0; i < tokenizer->count; i++)
    {
        if (tokenizer->tokens[i].type == JACON_TOKEN_STRING)
            free(tokenizer->tokens[i].string_val);
    }
    if (tokenizer->tokens) {
        free(tokenizer->tokens);
        tokenizer->tokens = NULL;
    }
}


void
Jacon_free_node(Jacon_Node* node)
{
    if (node->name != NULL) {
        free(node->name);
    }
    if (node->type == JACON_VALUE_STRING) {
        free(node->value.string_val);
        return;
    }
    if (node->type == JACON_VALUE_ARRAY || node->type == JACON_VALUE_OBJECT) {
        for (size_t i = 0; i < node->child_count; i++)
        {
            Jacon_free_node(&node->childs[i]);
        }
        free(node->childs);
        node->childs = NULL;
        return;
    }
}

Jacon_Error
Jacon_append_token(Jacon_Tokenizer* tokenizer, Jacon_Token token)
{
    if (tokenizer == NULL) {
        return JACON_ERR_NULL_PARAM;
    }
    if (tokenizer->tokens == NULL) {
        tokenizer->tokens = (Jacon_Token*)calloc(
            JACON_TOKENIZER_DEFAULT_CAPACITY, sizeof(Jacon_Token));
        if (tokenizer->tokens == NULL) {
            perror("Jacon_append_token array alloc error");
            return JACON_ERR_MEMORY_ALLOCATION;
        }
        tokenizer->capacity = JACON_TOKENIZER_DEFAULT_CAPACITY;
    }
    if (tokenizer->count >= tokenizer->capacity) {
        size_t new_capacity = tokenizer->capacity * 2;
        Jacon_Token* new_tokens = realloc(tokenizer->tokens, new_capacity * sizeof(Jacon_Token));
        if (!new_tokens) {
            return JACON_ERR_MEMORY_ALLOCATION;
        }
        tokenizer->tokens = new_tokens;
        tokenizer->capacity = new_capacity;
    }

    tokenizer->tokens[tokenizer->count++] = token;
    return JACON_OK;
}

Jacon_Error
Jacon_append_child(Jacon_Node* node, Jacon_Node child)
{
    if (node == NULL) {
        return JACON_ERR_NULL_PARAM;
    }
    if (node->childs == NULL) {
        node->childs = (Jacon_Node*)calloc(
            JACON_NODE_DEFAULT_CHILD_CAPACITY, sizeof(Jacon_Node));
        if (node->childs == NULL) {
            perror("Jacon_append_node_child array alloc error");
            return JACON_ERR_MEMORY_ALLOCATION;
        }
        node->child_capacity = JACON_NODE_DEFAULT_CHILD_CAPACITY;
    }
    if (node->child_count == node->child_capacity) {
        Jacon_Node* tmp = (Jacon_Node*)realloc(
            node->childs, 
            node->child_capacity * JACON_NODE_DEFAULT_RESIZE_FACTOR * sizeof(Jacon_Node));
        if (tmp == NULL) {
            perror("Jacon_append_node_child array realloc error");
            return JACON_ERR_MEMORY_ALLOCATION;
        }
        node->childs = tmp;
        node->child_capacity *= JACON_NODE_DEFAULT_RESIZE_FACTOR;
    }

    node->childs[node->child_count++] = child;
    return JACON_OK;
}

// Check if is a valid hex char
bool 
Jacon_is_hex_digit(char c) 
{
    return isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
}

// Only allow json allowed whitespace chars
bool
Jacon_is_whitespace(char c) 
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\n';
}

Jacon_Error
Jacon_validate_string(const char* str)
{
    const char *ptr = str;
    while (*ptr) {
        if (*ptr == '\n'
            || *ptr == '\t') return JACON_ERR_INVALID_ESCAPE_SEQUENCE;
        else if (*ptr == '\\') {
            ptr++;
            if (*ptr == 'u') {
                for (int i = 1; i < 5; ++i) {
                    if (!Jacon_is_hex_digit(*(ptr + i))) {
                        return JACON_ERR_INVALID_ESCAPE_SEQUENCE;
                    }
                }
                ptr += 4; // Skip valid escape sequence
            } else if (*ptr == '"' 
                || *ptr == '\\' 
                || *ptr == '/' 
                || *ptr == 'b' 
                || *ptr == 'f' 
                || *ptr == 'n' 
                || *ptr == 'r' 
                || *ptr == 't') {
                ptr++;
            } else {
                return JACON_ERR_INVALID_ESCAPE_SEQUENCE; // Invalid escape sequence
            }
        } else {
            ptr++; // Move to the next character
        }
    }
    return JACON_OK;
}

Jacon_Error 
Jacon_parse_token(Jacon_Token* token, const char** str) 
{
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

            // While the double quote is escaped, find the next one
            while (string_end[-1] == '\\') {
                string_end = strchr(string_end + 1, '"');
                if (string_end == NULL) return JACON_ERR_CHAR_NOT_FOUND;
            }
            token->type = JACON_TOKEN_STRING;
            size_t string_size = string_end - *str;
            token->string_val = (char*)calloc(string_size + 1, sizeof(char));
            if (token->string_val == NULL) return JACON_ERR_MEMORY_ALLOCATION;

            strncpy(token->string_val, *str, string_size);
            token->string_val[string_size] = '\0';

            int ret = Jacon_validate_string(token->string_val);
            if (ret != JACON_OK) {
                free(token->string_val);
                return ret;
            }

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
            // Check if whitespace
            if(Jacon_is_whitespace(**str)) {
                (*str)++;
                return Jacon_parse_token(token, str);
            }
            // Invalidate hex values
            else if (**str == '0' && (*str)[1] == 'x') return JACON_ERR_INVALID_JSON;
            // Invalidate leading 0
            else if (**str == '0' && (*str)[1] >= '0' && (*str)[1] <= '9') return JACON_ERR_INVALID_JSON;
            // Check if char is a number or a negative number
            else if ((**str >= '0' && **str <= '9') 
                || (**str == '-' && (((*str)[1] >= '0' && (*str)[1] <= '9')))) {
                if (**str == '-' && (*str)[1] == '0' && ((*str)[2] != '.' || !isdigit((*str)[2]))) {
                    const char* p = *str + 1;
                    while (*p == '0') p++;
                    if (isdigit(*p)) return JACON_ERR_INVALID_JSON;
                }
                char* dot = strchr(*str, '.');
                if (dot) {
                    // Check for leading 0, if real, check if distance between last '0' and '.' > 1
                    char* last_zero = strrchr(*str, '0');
                    if (last_zero && dot - last_zero > 1) return JACON_ERR_INVALID_JSON;
                    // Check if there is a dot and if so if there are numbers after
                    char* after_dot = dot + 1;
                    if (!isdigit(*after_dot)) return JACON_ERR_INVALID_JSON;
                }
                
                char *endptr;
                // Parse an int
                int ival = (int)strtol(*str, &endptr, 10);
                if ((isspace(*endptr) || *endptr == ',' || *endptr == ']' || *endptr == '}' || *endptr == '\0')
                    && ival >= INT_MIN && ival <= INT_MAX) {
                    token->int_val = ival;
                    token->type = JACON_TOKEN_INT;
                    *str = endptr;
                    break;
                }

                // Parse a double and check for float cast
                double dval = strtod(*str, &endptr);
                if(!isspace(*endptr) && *endptr != ',' && *endptr != ']' && *endptr != '}' && *endptr != '\0')
                    return JACON_ERR_INVALID_JSON;

                else if (fabs(dval) <= FLT_MAX && fabs(dval) >= FLT_MIN) {
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

Jacon_Error 
Jacon_tokenize(Jacon_Tokenizer* tokenizer, const char* str) 
{
    Jacon_Error ret;

    while (*str) {
        Jacon_Token token = {0};
        ret = Jacon_parse_token(&token, &str);
        if (ret == JACON_END_OF_INPUT) return JACON_OK;
        if (ret != JACON_OK) return ret;
        ret = Jacon_append_token(tokenizer, token);
        if (ret != JACON_OK) return ret;
    }

    return JACON_OK;
}

Jacon_Error
Jacon_current_token(Jacon_Token* token, Jacon_Tokenizer* tokenizer, size_t current_index)
{
    if (current_index >= tokenizer->count) return JACON_ERR_INDEX_OUT_OF_BOUND;
    *token = tokenizer->tokens[current_index];
    return JACON_OK;
}

Jacon_Error
Jacon_consume_token(Jacon_Token* token, Jacon_Tokenizer* tokenizer, size_t* current_index)
{
    if (*current_index >= tokenizer->count) return JACON_ERR_INDEX_OUT_OF_BOUND;
    *token = tokenizer->tokens[*current_index + 1];
    *current_index += 1;
    return JACON_OK;
}

Jacon_Error
Jacon_validate_object(Jacon_Tokenizer* tokenizer, size_t* index);
Jacon_Error
Jacon_validate_array(Jacon_Tokenizer* tokenizer, size_t* index);

Jacon_Error
Jacon_validate_array(Jacon_Tokenizer* tokenizer, size_t* index)
{
    int ret = JACON_OK;
    Jacon_Token* current = NULL;
    bool last_value = false;
    
    if (*index < tokenizer->count) {
        current = &tokenizer->tokens[*index];
    } else {
        return JACON_ERR_INVALID_JSON;
    }

    while (current->type != JACON_TOKEN_ARRAY_END) {
        current = &tokenizer->tokens[*index];
        switch (current->type) {
            case JACON_TOKEN_ARRAY_START:
                (*index)++;
                ret = Jacon_validate_array(tokenizer, index);
                if (ret != JACON_OK) return ret;
                last_value = true;
                break;
            case JACON_TOKEN_OBJECT_START:
                (*index)++;
                ret = Jacon_validate_object(tokenizer, index);
                if (ret != JACON_OK) return ret;
                last_value = true;
                break;
            case JACON_TOKEN_COMMA:
                if (!last_value) {
                    return JACON_ERR_INVALID_JSON;
                }
                else {
                    last_value = false;
                    (*index)++;    
                }
                break;
            case JACON_TOKEN_ARRAY_END:
                return JACON_ERR_UNREACHABLE_STATEMENT;
            case JACON_TOKEN_OBJECT_END:
            case JACON_TOKEN_COLON:
                return JACON_ERR_INVALID_JSON;
            case JACON_TOKEN_STRING:
            case JACON_TOKEN_INT:
            case JACON_TOKEN_DOUBLE:
            case JACON_TOKEN_FLOAT:
            case JACON_TOKEN_BOOLEAN:
            case JACON_TOKEN_NULL:
            default:
                if(last_value) return JACON_ERR_INVALID_JSON;
                last_value = true;
                (*index)++;
                break;
        }
        if (*index >= tokenizer->count) {
            return JACON_ERR_INVALID_JSON;
        }
        current = &tokenizer->tokens[*index];
    }
    (*index)++;
    return JACON_OK;
}

Jacon_Error
Jacon_validate_object(Jacon_Tokenizer* tokenizer, size_t* index)
{
    int ret = JACON_OK;
    if (*index >= tokenizer->count) return JACON_ERR_INVALID_JSON;
    if (*index >= tokenizer->count) {
        return JACON_ERR_INVALID_JSON;
    }
    Jacon_Token* current = &tokenizer->tokens[*index];
    Jacon_Token* last = NULL;
    bool last_value = false;
    while (current->type != JACON_TOKEN_OBJECT_END) {
        current = &tokenizer->tokens[*index];
        switch (current->type) {
            case JACON_TOKEN_ARRAY_START:
                if (last == NULL || last->type != JACON_TOKEN_COLON)
                    return JACON_ERR_INVALID_JSON;

                (*index)++;
                ret = Jacon_validate_array(tokenizer, index);
                if (ret != JACON_OK) return ret;
                last_value = true;
                break;
            case JACON_TOKEN_OBJECT_START:
                if (last == NULL || last->type != JACON_TOKEN_COLON)
                    return JACON_ERR_INVALID_JSON;

                (*index)++;
                ret = Jacon_validate_object(tokenizer, index);
                if (ret != JACON_OK) return ret;
                last_value = true;
                break;
            case JACON_TOKEN_COMMA:
                if (last == NULL) return JACON_ERR_INVALID_JSON;
                else if (!last_value) {
                    return JACON_ERR_INVALID_JSON;
                }
                last = &tokenizer->tokens[*index];
                (*index)++;
                last_value = false;
                break;
            case JACON_TOKEN_ARRAY_END:
                return JACON_ERR_INVALID_JSON;
            case JACON_TOKEN_OBJECT_END:
                return JACON_ERR_UNREACHABLE_STATEMENT;
            case JACON_TOKEN_COLON:
                if (last == NULL) return JACON_ERR_INVALID_JSON;
                if (last->type != JACON_TOKEN_STRING) return JACON_ERR_INVALID_JSON;
                last = &tokenizer->tokens[*index];
                (*index)++;
                break;
            case JACON_TOKEN_STRING:
                if (last == NULL) {
                    last = &tokenizer->tokens[*index];
                    (*index)++;
                }
                else if (last->type == JACON_TOKEN_COLON || last->type == JACON_TOKEN_COMMA) {
                    last = &tokenizer->tokens[*index];
                    (*index)++;
                    last_value = true;
                }
                else {
                    return JACON_ERR_UNREACHABLE_STATEMENT;
                }
                break;
            case JACON_TOKEN_INT:
            case JACON_TOKEN_DOUBLE:
            case JACON_TOKEN_FLOAT:
            case JACON_TOKEN_BOOLEAN:
            case JACON_TOKEN_NULL:
                if (last == NULL || last->type != JACON_TOKEN_COLON)
                    return JACON_ERR_INVALID_JSON;
                last = &tokenizer->tokens[*index];
                (*index)++;
                last_value = true;
                break;
            default:
                return JACON_ERR_UNREACHABLE_STATEMENT;
        }
        if (*index >= tokenizer->count) {
            return JACON_ERR_INVALID_JSON;
        }
        current = &tokenizer->tokens[*index];
    }
    (*index)++;
    return JACON_OK;
}

Jacon_Error
Jacon_validate_value(Jacon_Token token)
{
    switch (token.type) {
        case JACON_TOKEN_STRING:
        case JACON_TOKEN_INT:
        case JACON_TOKEN_DOUBLE:
        case JACON_TOKEN_FLOAT:
        case JACON_TOKEN_BOOLEAN:
        case JACON_TOKEN_NULL:
            return JACON_OK;
        case JACON_TOKEN_ARRAY_START:
        case JACON_TOKEN_ARRAY_END:
        case JACON_TOKEN_OBJECT_START:
        case JACON_TOKEN_OBJECT_END:
        case JACON_TOKEN_COMMA:
        case JACON_TOKEN_COLON:
            return JACON_ERR_INVALID_JSON;
    }
    return JACON_ERR_UNREACHABLE_STATEMENT;
}

/**
 * Validate a Jacon tokenizer result
 */
Jacon_Error
Jacon_validate_input(Jacon_Tokenizer* tokenizer)
{
    int ret = JACON_OK;

    if (tokenizer->count == 1) {
        ret = Jacon_validate_value(tokenizer->tokens[0]);
        return ret;
    }

    Jacon_Token* current = NULL;
    size_t index = 0;
    current = &tokenizer->tokens[index];
    switch (current->type) {
        case JACON_TOKEN_ARRAY_START:
            index++;
            ret = Jacon_validate_array(tokenizer, &index);
            if (ret != JACON_OK) return ret;
            break;
        case JACON_TOKEN_OBJECT_START:
            index++;
            ret = Jacon_validate_object(tokenizer, &index);
            if (ret != JACON_OK) return ret;
            break;
        case JACON_TOKEN_STRING:
        case JACON_TOKEN_INT:
        case JACON_TOKEN_DOUBLE:
        case JACON_TOKEN_FLOAT:
        case JACON_TOKEN_BOOLEAN:
        case JACON_TOKEN_NULL:
        case JACON_TOKEN_ARRAY_END:
        case JACON_TOKEN_OBJECT_END:
        case JACON_TOKEN_COMMA:
        case JACON_TOKEN_COLON:
        default:
            return JACON_ERR_UNREACHABLE_STATEMENT;
    }
    if (index < tokenizer->count) return JACON_ERR_INVALID_JSON;

    return JACON_OK;
}

Jacon_Error
Jacon_parse_node(Jacon_Node* node, Jacon_Tokenizer* tokenizer, size_t* current_index)
{
    int ret = JACON_OK;
    Jacon_Token current_token;
    ret = Jacon_current_token(&current_token, tokenizer, *current_index);
    if (ret != JACON_OK) return ret;
    switch (current_token.type) {
        case JACON_TOKEN_OBJECT_START:
            node->type = JACON_VALUE_OBJECT;

            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            while (current_token.type != JACON_TOKEN_OBJECT_END) {
                Jacon_Node child = {0};
                child.parent = node;
                ret = Jacon_parse_node(&child, tokenizer, current_index);
                if (ret != JACON_OK) {
                    Jacon_free_node(&child);
                    return ret;
                }

                ret = Jacon_append_child(node, child);
                if (ret != JACON_OK) return ret;
                ret = Jacon_current_token(&current_token, tokenizer, *current_index);
                if (ret != JACON_OK) return ret;
            }
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_ARRAY_START:
            node->type = JACON_VALUE_ARRAY;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            while (current_token.type != JACON_TOKEN_ARRAY_END) {
                Jacon_Node child = {0};
                child.parent = node;
                ret = Jacon_parse_node(&child, tokenizer, current_index);
                if (ret != JACON_OK) {
                    Jacon_free_node(&child);
                    return ret;
                }

                ret = Jacon_append_child(node, child);
                if (ret != JACON_OK) return ret;
                ret = Jacon_current_token(&current_token, tokenizer, *current_index);
                if (ret != JACON_OK) return ret;
            }
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_STRING:
            if (node->type == JACON_VALUE_STRING) {
                node->value.string_val = strdup(current_token.string_val);
                if (node->value.string_val == NULL) return JACON_ERR_MEMORY_ALLOCATION;

                ret = Jacon_consume_token(&current_token, tokenizer, current_index);
                if (ret != JACON_OK) return ret;
            }
            else if (node->parent != NULL && node->parent->type == JACON_VALUE_ARRAY) {
                node->type = JACON_VALUE_STRING;
                node->value.string_val = strdup(current_token.string_val);
                if (node->value.string_val == NULL) return JACON_ERR_MEMORY_ALLOCATION;

                ret = Jacon_consume_token(&current_token, tokenizer, current_index);
                if (ret != JACON_OK) return ret;
            }
            else {
                node->type = JACON_VALUE_STRING;
                node->name = strdup(current_token.string_val);
                if (node->name == NULL) return JACON_ERR_MEMORY_ALLOCATION;

                ret = Jacon_consume_token(&current_token, tokenizer, current_index);
                if (ret != JACON_OK) return ret;

                ret = Jacon_parse_node(node, tokenizer, current_index);
                if (ret != JACON_OK) return ret;
            }
            break;

        case JACON_TOKEN_BOOLEAN:
            node->type = JACON_VALUE_BOOLEAN;
            node->value.bool_val = current_token.bool_val;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_INT:
            node->type = JACON_VALUE_INT;
            node->value.int_val = current_token.int_val;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_FLOAT:
            node->type = JACON_VALUE_FLOAT;
            node->value.float_val = current_token.float_val;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_DOUBLE:
            node->type = JACON_VALUE_DOUBLE;
            node->value.double_val = current_token.double_val;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;

        case JACON_TOKEN_NULL:
            node->type = JACON_VALUE_NULL;
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            break;
            
        case JACON_TOKEN_COLON:
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            return Jacon_parse_node(node, tokenizer, current_index);
        case JACON_TOKEN_COMMA:
            ret = Jacon_consume_token(&current_token, tokenizer, current_index);
            if (ret != JACON_OK) return ret;
            return Jacon_parse_node(node, tokenizer, current_index);
        case JACON_TOKEN_OBJECT_END:
        case JACON_TOKEN_ARRAY_END:
            // Both of these cases should never happen
            // We skip this token when we are done parsing an array / object
        default:
            return JACON_ERR_UNREACHABLE_STATEMENT;
    }
    return JACON_OK;
}

Jacon_Error
Jacon_parse_value(Jacon_Node* root, Jacon_Token token)
{
    switch (token.type) {
        case JACON_TOKEN_STRING:
            root->type = JACON_VALUE_STRING;
            root->value.string_val = strdup(token.string_val);
            if (root->value.string_val == NULL) return JACON_ERR_MEMORY_ALLOCATION;
            break;
        case JACON_TOKEN_INT:
            root->type = JACON_VALUE_INT;
            root->value.int_val = token.int_val;
            break;
        case JACON_TOKEN_DOUBLE:
            root->type = JACON_VALUE_DOUBLE;
            root->value.double_val = token.double_val;
            break;
        case JACON_TOKEN_FLOAT:
            root->type = JACON_VALUE_FLOAT;
            root->value.float_val = token.float_val;
            break;
        case JACON_TOKEN_BOOLEAN:
            root->type = JACON_VALUE_BOOLEAN;
            root->value.bool_val = token.bool_val;
            break;
        case JACON_TOKEN_NULL:
            root->type = JACON_VALUE_NULL;
            break;
        case JACON_TOKEN_ARRAY_START:
        case JACON_TOKEN_ARRAY_END:
        case JACON_TOKEN_OBJECT_START:
        case JACON_TOKEN_OBJECT_END:
        case JACON_TOKEN_COLON:
        case JACON_TOKEN_COMMA:
        default:
            return JACON_ERR_UNREACHABLE_STATEMENT;
    }
    return JACON_OK;
}

/**
 * Parse a Jacon tokenizer content into a queryable C variable
 */
Jacon_Error
Jacon_parse_tokens(Jacon_Node* root, Jacon_Tokenizer* tokenizer)
{
    int ret = JACON_OK;
    if (tokenizer->count == 1) 
        return Jacon_parse_value(root, tokenizer->tokens[0]);
    size_t current_index = 0;
    while(current_index < tokenizer->count) {
        ret = Jacon_parse_node(root, tokenizer, &current_index);
        if (ret != JACON_OK && ret != JACON_NO_MORE_TOKENS) return ret;
    }
    return JACON_OK;
}

Jacon_Error
Jacon_build_content(Jacon_content* content)
{
    (void)content;
    return JACON_OK;
}

Jacon_Error
Jacon_get_value_by_name(Jacon_content* content)
{
    (void)content;
    return JACON_OK;
}

/**
 * Get single value by type
 */
Jacon_Error
Jacon_get_value(Jacon_content* content, Jacon_ValueType type, void* value)
{
    if (content == NULL || value == NULL) 
        return JACON_ERR_NULL_PARAM;
    switch (type) {
        case JACON_VALUE_STRING:
            *(char**)value = strdup(content->root->value.string_val);
            if (value == NULL) return JACON_ERR_MEMORY_ALLOCATION;
            break;
        case JACON_VALUE_INT:
            *(int*)value = content->root->value.int_val;
            break;
        case JACON_VALUE_FLOAT:
            *(float*)value = content->root->value.float_val;
            break;
        case JACON_VALUE_DOUBLE:
            *(double*)value = content->root->value.double_val;
            break;
        case JACON_VALUE_BOOLEAN:
            *(bool*)value = content->root->value.bool_val;
            break;
        case JACON_VALUE_NULL:
            // Who tf would use that
            *(void**)value = NULL;
            break;
        case JACON_VALUE_ARRAY:
        case JACON_VALUE_OBJECT:
        default:
            return JACON_ERR_INVALID_VALUE_TYPE;
    }
    return JACON_OK;
}

/**
 * Get single string value
 */
Jacon_Error
Jacon_get_string(Jacon_content* content, char* value)
{
    return Jacon_get_value(content, JACON_VALUE_STRING, value);
}

/**
 * Get single int value
 */
Jacon_Error
Jacon_get_int(Jacon_content* content, int* value)
{
    return Jacon_get_value(content, JACON_VALUE_INT, value);
}

/**
 * Get single float value
 */
Jacon_Error
Jacon_get_float(Jacon_content* content, float* value)
{
    return Jacon_get_value(content, JACON_VALUE_FLOAT, value);
}

/**
 * Get single double value
 */
Jacon_Error
Jacon_get_double(Jacon_content* content, double* value)
{
    return Jacon_get_value(content, JACON_VALUE_DOUBLE, value);
}

/**
 * Get single boolean value
 */
Jacon_Error
Jacon_get_bool(Jacon_content* content, bool* value)
{
    return Jacon_get_value(content, JACON_VALUE_BOOLEAN, value);
}

/**
 * Get single null value
 * 
 * This is insane wtf
 */
Jacon_Error
Jacon_get_null(Jacon_content* content, void* value)
{
    return Jacon_get_value(content, JACON_VALUE_NULL, value);
}

Jacon_Error
Jacon_value_exist_by_name(Jacon_content* content, const char* name, Jacon_ValueType type)
{
    (void)content;
    StringBuilder builder = {0};
    char* str_Value;
    int ret = Jacon_value_type_to_str(type, &str_Value);
    if (ret != JACON_OK) return ret;

    Ju_str_append_null(&builder, "(", str_Value, ")", name);
    Ju_str_free(&builder);
    return JACON_OK;
}

/**
 * Verify the existence of a single value of given type
 */
Jacon_Error
Jacon_exist(Jacon_content* content, Jacon_ValueType type, bool* result)
{
    void* value = NULL;
    int ret = Jacon_get_value(content, type, value);
    if (ret != JACON_OK) return ret;
    if (type == JACON_VALUE_NULL) *result = value == NULL;
    else *result = value == NULL;
    return JACON_OK;
}

/**
 * Verify the existence of a single string value
 */
Jacon_Error
Jacon_exist_string(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_STRING, result);
}

/**
 * Verify the existence of a single int value
 */
Jacon_Error
Jacon_exist_int(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_INT, result);
}

/**
 * Verify the existence of a single float value
 */
Jacon_Error
Jacon_exist_float(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_FLOAT, result);
}

/**
 * Verify the existence of a single double value
 */
Jacon_Error
Jacon_exist_double(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_DOUBLE, result);
}

/**
 * Verify the existence of a single boolean value
 */
Jacon_Error
Jacon_exist_bool(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_BOOLEAN, result);
}

/**
 * Verify the existence of a single null value
 */
Jacon_Error
Jacon_exist_null(Jacon_content* content, bool* result)
{
    return Jacon_exist(content, JACON_VALUE_NULL, result);
}

/**
 * Parse a Jacon node into a valid json string
 */
Jacon_Error
Jacon_parse_object()
{
    return JACON_OK;
}

Jacon_Error
Jacon_parse_input(Jacon_content* content, const char* str)
{
    if (content == NULL || str == NULL) return JACON_ERR_NULL_PARAM;
    size_t len = strlen(str);
    if (len == 0) return JACON_ERR_EMPTY_INPUT;

    Jacon_Tokenizer tokenizer;
    Jacon_tokenizer_init(&tokenizer);
    Jacon_Error ret;
    ret = Jacon_tokenize(&tokenizer, str);
    if (ret != JACON_OK) {
        Jacon_free_tokenizer(&tokenizer);
        return ret;
    }
    
    // Invalidate empty input
    if (tokenizer.count == 0) {
        Jacon_free_tokenizer(&tokenizer);
        return JACON_ERR_EMPTY_INPUT;
    }

    ret = Jacon_validate_input(&tokenizer);
    if (ret != JACON_OK) {
        Jacon_free_tokenizer(&tokenizer);
        return ret;
    }

    ret = Jacon_parse_tokens(content->root, &tokenizer);
    if (ret != JACON_OK) {
        Jacon_free_tokenizer(&tokenizer);
        return ret;
    }
    Jacon_free_tokenizer(&tokenizer);

    return JACON_OK;
}

#endif // JACON_IMPLEMENTATION