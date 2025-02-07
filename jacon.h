#ifndef JACON_H
#define JACON_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

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
    JACON_ERR_INVALID_SIZE,
    JACON_ERR_APPEND_FSTRING,
    JACON_ERR_KEY_NOT_FOUND,
    JACON_ERR_UNREACHABLE_STATEMENT,
} Jacon_Error;

typedef struct Jacon_StringBuilder Jacon_StringBuilder;

struct Jacon_StringBuilder {
    char* string;
    size_t count;
    size_t capacity;
};

#define JACON_MAP_DEFAULT_SIZE 10
#define JACON_MAP_RESIZE_FACTOR 2
typedef struct Jacon_HashMap Jacon_HashMap;
typedef struct Jacon_HashMapEntry Jacon_HashMapEntry;
typedef struct Jacon_Node Jacon_Node;

struct Jacon_HashMapEntry {
    char* key;
    Jacon_Node* value;
    // Next entry in the linked list, 
    // NULL by default or if last entry in list
    Jacon_HashMapEntry* next_entry;
};

struct Jacon_HashMap {
    size_t size;
    size_t entries_count;
    Jacon_HashMapEntry** entries;
};

Jacon_Error
Jacon_hm_create(Jacon_HashMap* map, size_t size);

/**
 * Get value for a specific key
 * Returns 
 *  - The stored value associated to the key
 *  - NULL if the key is not present, or the map is is NULL allocated
 */
void*
Jacon_hm_get(Jacon_HashMap* map, const char* key);

/**
 * Put the key, value pair in the hashmap
 * Returns
 *  - 0 if the key, value pair was successfully added
 *  - -1 if the map is NULL allocated
 */
Jacon_Error
Jacon_hm_put(Jacon_HashMap* map, const char* key, void* value);

/**
 * Remove a key, value pair from the hashtable
 * Returns 
 *  - The removed value associated to the key
 *  - NULL if the key is not present, or the map is is NULL allocated
 */
void*
Jacon_hm_remove(Jacon_HashMap* map, const char* key);

/**
 * Free the memory allocated for the map
 */
void
Jacon_hm_free(Jacon_HashMap* map);

#define JACON_TOKENIZER_DEFAULT_CAPACITY 256
#define JACON_TOKENIZER_DEFAULT_RESIZE_FACTOR 2
#define JACON_NODE_DEFAULT_CHILD_CAPACITY 1
#define JACON_NODE_DEFAULT_RESIZE_FACTOR 2

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

struct Jacon_Node {
    Jacon_Node* parent;
    char* name;
    Jacon_ValueType type;
    Jacon_Value value;
    Jacon_Node** childs;
    size_t child_count;
    size_t child_capacity;
};

typedef struct Jacon_content {
    Jacon_Node* root;
    // Dictionary for efficient value retrieving
    Jacon_HashMap entries;
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
 * Append a node to another node's childs
 */
Jacon_Error
Jacon_append_child(Jacon_Node* node, Jacon_Node* child);

/**
 * Parse a Json string input into a queryable object
 */
Jacon_Error
Jacon_deserialize(Jacon_content* content, const char* str);

/**
 * Parse a node into its Json representation
 */
Jacon_Error
Jacon_serialize(Jacon_Node* node, char** str);

/**
 * Parse a node into its unformatted (compact) Json representation
 */
Jacon_Error
Jacon_serialize_unformatted(Jacon_Node* node, char** str);

/**
 * Duplicate a node
 */
Jacon_Node* 
Jacon_duplicate_node(const Jacon_Node* node);

/**
 * Free a node's content
 */
void
Jacon_free_node(Jacon_Node* node);

Jacon_Error
Jacon_init_content(Jacon_content* content);

Jacon_Error
Jacon_free_content(Jacon_content* content);

// Used to create a named node
// Please use these if you plan to add the node to an object
#define Jacon_string_prop(node_name, node_value) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_STRING , \
    .value.string_val = strdup(node_value) }

#define Jacon_int_prop(node_name, node_value) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_INT , \
    .value.int_val = node_value }

#define Jacon_float_prop(node_name, node_value) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_FLOAT , \
    .value.float_val = node_value }

#define Jacon_double_prop(node_name, node_value) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_DOUBLE , \
    .value.double_val = node_value }

#define Jacon_boolean_prop(node_name, node_value) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_BOOLEAN , \
    .value.bool_val = node_value }

#define Jacon_null_prop(node_name) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_NULL }

#define Jacon_array_prop(node_name) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_ARRAY }

#define Jacon_object_prop(node_name) (Jacon_Node){ \
    .name = strdup(node_name), \
    .type = JACON_VALUE_OBJECT }

// Used to create a single value node
// Should not be used to create nodes that will be put as object property
// Please use Jacon_type_prop for that
#define Jacon_string(node_value) (Jacon_Node){ \
    .type = JACON_VALUE_STRING , \
    .value.string_val = strdup(node_value) }

#define Jacon_int(node_value) (Jacon_Node){ \
    .type = JACON_VALUE_INT , \
    .value.int_val = node_value }

#define Jacon_float(node_value) (Jacon_Node){ \
    .type = JACON_VALUE_FLOAT , \
    .value.float_val = node_value }

#define Jacon_double(node_value) (Jacon_Node){ \
    .type = JACON_VALUE_DOUBLE , \
    .value.double_val = node_value }

#define Jacon_boolean(node_value) (Jacon_Node){ \
    .type = JACON_VALUE_BOOLEAN , \
    .value.bool_val = node_value }

#define Jacon_null() (Jacon_Node){ \
    .type = JACON_VALUE_NULL }

#define Jacon_array() (Jacon_Node){ \
    .type = JACON_VALUE_ARRAY }

#define Jacon_object() (Jacon_Node){ \
    .type = JACON_VALUE_OBJECT }

Jacon_Error
Jacon_get_string_by_name(Jacon_content* content, const char* name, char** value);

Jacon_Error
Jacon_get_int_by_name(Jacon_content* content, const char* name, int* value);

Jacon_Error
Jacon_get_float_by_name(Jacon_content* content, const char* name, float* value);

Jacon_Error
Jacon_get_double_by_name(Jacon_content* content, const char* name, double* value);
Jacon_Error
Jacon_get_bool_by_name(Jacon_content* content, const char* name, bool* value);

/**
 * Get single string value
 */
Jacon_Error
Jacon_get_string(Jacon_content* content, char** value);

/**
 * Get single int value
 */
Jacon_Error
Jacon_get_int(Jacon_content* content, int* value);

/**
 * Get single float value
 */
Jacon_Error
Jacon_get_float(Jacon_content* content, float* value);

/**
 * Get single double value
 */
Jacon_Error
Jacon_get_double(Jacon_content* content, double* value);

/**
 * Get single boolean value
 */
Jacon_Error
Jacon_get_bool(Jacon_content* content, bool* value);

bool
Jacon_exist_by_name(Jacon_content* content, const char* name, Jacon_ValueType type);

/**
 * Verify the existence of a single string value
 */
Jacon_Error
Jacon_exist_string_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single int value
 */
Jacon_Error
Jacon_exist_int_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single float value
 */
Jacon_Error
Jacon_exist_float_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single double value
 */
Jacon_Error
Jacon_exist_double_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single boolean value
 */
Jacon_Error
Jacon_exist_bool_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single null value
 */
Jacon_Error
Jacon_exist_null_by_name(Jacon_content* content, const char* name);

/**
 * Verify the existence of a single value of given type
 */
bool
Jacon_exist(Jacon_content* content, Jacon_ValueType type);

/**
 * Verify the existence of a single string value
 */
Jacon_Error
Jacon_exist_string(Jacon_content* content);

/**
 * Verify the existence of a single int value
 */
Jacon_Error
Jacon_exist_int(Jacon_content* content);

/**
 * Verify the existence of a single float value
 */
Jacon_Error
Jacon_exist_float(Jacon_content* content);

/**
 * Verify the existence of a single double value
 */
Jacon_Error
Jacon_exist_double(Jacon_content* content);

/**
 * Verify the existence of a single boolean value
 */
Jacon_Error
Jacon_exist_bool(Jacon_content* content);

/**
 * Verify the existence of a single null value
 */
Jacon_Error
Jacon_exist_null(Jacon_content* content);

#endif // JACON_H