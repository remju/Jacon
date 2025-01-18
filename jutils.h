#ifndef JUTILS_H
#define JUTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#define LOG(lvl, fmt, ...) printf("[" lvl "] " fmt "\n", ##__VA_ARGS__)
#define INFO(fmt, ...) LOG("INFO", fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) LOG("WARN", fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) LOG("ERROR", fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) LOG("DEBUG", fmt, ##__VA_ARGS__)
#define CHECK(cond, msg) do { if (!(cond)) { LOG("ERROR", "%s", msg); exit(EXIT_FAILURE); } } while (0)

typedef struct StringBuilder StringBuilder;

struct StringBuilder {
    char* items;
    size_t count;
    size_t capacity;
};

void 
Ju_str_append(StringBuilder* builder, ...);

void
Ju_str_free(StringBuilder *builder);

#endif // JUTILS_H

#ifdef JUTILS_IMPLEMENTATION

#include <stdbool.h>
#include <string.h>

#define Ju_str_append_null(builder, ...) (Ju_str_append(builder, __VA_ARGS__, NULL))

void 
Ju_str_append(StringBuilder* builder, ...) 
{
    va_list args;
    va_start(args, builder);

    const char* arg = va_arg(args, const char*);
    while (arg != NULL) 
    {
        size_t size = strlen(arg);
        printf("Appending string builder: %s + %s , %ld -> %ld\n", builder->items, arg, builder->count, builder->count + size);
        if (builder->count + size + 1 > builder->capacity)
        {
            size_t new_capacity = builder->count + size + 1;
            builder->items = realloc(builder->items, new_capacity * sizeof(char));
            CHECK(builder->items != NULL, "Ju_str_append realloc error");
            builder->capacity = new_capacity;
        }
        strncpy(builder->items + builder->count, arg, size);
        builder->count += size;
        arg = va_arg(args, const char*);
    }
    // Add null terminating byte if len > 0
    if (builder->count > 0)
        builder->items[builder->count] = '\0';

    va_end(args);
}

void
Ju_str_free(StringBuilder *builder)
{
    if (builder->items != NULL) 
    {
        free(builder->items);
        builder->items = NULL;
    }
}

#endif // JUTILS_IMPLEMENTATION