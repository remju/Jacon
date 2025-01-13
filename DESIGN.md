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
```C
#define JACON_IMPLEMENTATION
#include "jacon.h"
```

defining 'JACON_IMPLEMENTATION' gives you access to functions implementations
only including the header file gives you access to definitions only

```C
typedef enum Jacon_NodeType {
    JACON_NODE_ROOT,
    JACON_NODE_VALUE,
};

typedef enum Jacon_ValueType {
    JACON_VALUE_OBJECT,
    JACON_VALUE_ARRAY,
    JACON_VALUE_STRING,
    JACON_VALUE_INT,
    JACON_VALUE_FLOAT,
    JACON_VALUE_DOUBLE,
    JACON_VALUE_BOOLEAN,
    JACON_VALUE_NULL,
};

typedef Jacon_Array Jacon_Value*;
typedef Jacon_Object Jacon_Node;

typedef struct Jacon_Value {
    union {
        char* string_val;
        union {
            int int_val;
            float float_val;
            double double_val;
            struct {
                double base;
                double exponent;
            } exponential;
        };
        bool* bool_val;
        Jacon_Value* array;
        Jacon_Node object;
    };
};

typedef struct Jacon_Node {
    Jacon_Node parent;
    Jacon_NodeType type;
    char* name;
    Jacon_Value value;
    Jacon_Node* childs;
    size_t child_count;
    // Dictionnary for efficient value retrieving
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

# References
- [json.org](https://www.json.org/json-en.html)
- [stb libs](https://github.com/nothings/stb)