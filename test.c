#define JACON_IMPLEMENTATION
#include "jacon.h"
#include "stdio.h"
#include <string.h>

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

int
main(void)
{
    puts("Tests executed successfully !");
}