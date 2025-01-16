#define JACON_IMPLEMENTATION
#include "jacon.h"
#include "stdio.h"
#include <string.h>

bool test_Jacon_value_type_to_str(void);
void expect(bool (*tested_func)(), bool expected, const char* tested_func_name);

#define EXPECT(tested_func, expected) expect(tested_func, expected, #tested_func)

void
expect(bool (*tested_func)(), bool expected, const char* tested_func_name)
{
    bool result = tested_func();
    if (result != expected) {
        printf("Test %s failed: expected %d, got %d\n", tested_func_name, expected, result);
    } else {
        printf("Test %s passed\n", tested_func_name);
    }
}

bool
test_Jacon_value_type_to_str()
{
    char* str_value;
    #define TEST_VALUE_TYPE_TO_STR(type, str) do { \
        int ret = Jacon_value_type_to_str(type, &str_value); \
        if (ret != JACON_OK) return false; \
        if (strcmp(str_value, str) != 0) return false; \
    } while(0)
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_OBJECT, "object");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_ARRAY, "array");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_STRING, "string");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_INT, "int");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_FLOAT, "float");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_DOUBLE, "double");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_BOOLEAN, "boolean");
    TEST_VALUE_TYPE_TO_STR(JACON_VALUE_NULL, "null");
    return true;
    #undef TEST_VALUE_TYPE_TO_STR
}

int
main(void)
{
    EXPECT(test_Jacon_value_type_to_str, true);
    puts("Tests executed successfully !");
}