# Outdated
This document was used for starting the project and is no longer up to date. 

# Needs
Parsing a string into a struct than i can go through an get values associated to keys using C

Receiving a string, going trough it and extracting names/values, associating names and values using a dictionnary
for easier values retrieving.

Being able to build a string from names/values.

Dictionnary using a hashmap ? Efficient value retrieving, ease of input validation.

Error handling: looking for an inexistant name/value pair, ...

Input validation, malformed, ...

# Code ideas
Single header file library (stb style)
defining 'JACON_IMPLEMENTATION' gives you access to functions definitions,
only including the header file gives you access to declarations only
```C
#define JACON_IMPLEMENTATION
#include "jacon.h"
```

Handle multiple error codes such as:
```C
// Error codes
typedef enum {
    JACON_OK,
    JACON_END_OF_INPUT,
    JACON_NULL_PARAM,
    JACON_ERR_INVALID_JSON,
    JACON_ERR_CHAR_NOT_FOUND,
    JACON_ALLOC_ERROR,
} Jacon_Error;
```

Type definitions for tokenizing:
```C
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
```

Type definitions for storing and querying:
```C
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
```

Example Json to parse and use:
```Json
{
    "person": {
        "age": 18,
        "address": "123 Maple Street",
        "contacts": []
    },
    "otherperson": {
        "age": 22,
        "address": "456 Elm Avenue",
        "contacts": []
    },
    "example": true
}
```

When storing name/values pairs in the dictionnary, create a unique key using names from the root.
Ex: person.age, otherperson.age
That way we can't ever have the same key storred.

Parse a string into a queryable object (keep root in mind, we use it everywhere after):
```C
Jacon_Node root;
Jacon_parse_string(&root, "{ \"example\": true }");
```

Functions to validate the presence of a name/value pair of the right type.
```C
if (Jacon_exists(&root, "person.age", JACON_VALUE_INT)) {
    // do something usefull...
}
```

Functions to retrieve a value for a given key, will need a function for each possible type.
```C
int i = Jacon_get_int(&root, "person.age"); 
Jacon_Array arr = Jacon_get_array(&root, "person.contacts");

// Query an object property
char* address = Jacon_get_string(&root, "person.adress");
```

Parse an object into a string:
```C
char* parsed_object = Jacon_parse_object(&root);
```

# Parsing Json
Tokenize the input string.

Next, validate the input string if no error where encountered while tokenizing.

If no errors where encountered during input validation, parse the json into a queryable object

# References
- [json.org](https://www.json.org/json-en.html)
- [stb libs](https://github.com/nothings/stb)
