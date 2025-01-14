#define JACON_IMPLEMENTATION
#include "jacon.h"
#include <stdio.h>
#include <sys/time.h>

void
print_error(Jacon_Error error)
{
    switch (error)
    {
        case JACON_INDEX_OUT_OF_BOUND:
            puts("Jacon error: index out of bound.");
            break;
        case(JACON_ALLOC_ERROR):
            puts("Jacon error: allocation error.");
            break;
        case(JACON_ERR_CHAR_NOT_FOUND):
            puts("Jacon error: char not found.");
            break;
        case(JACON_ERR_INVALID_JSON):
            puts("Jacon error: invalid json.");
            break;
        case(JACON_NO_MORE_TOKENS):
            puts("Jacon error: no more tokens.");
            break;
        case(JACON_NULL_PARAM):
            puts("Jacon error: null param.");
            break;
        case(JACON_END_OF_INPUT):
        case JACON_OK:
        default:
            break;
    }
}

int
main(int argc, const char** argv)
{
    if (argc <= 1) {
        printf("Please specify an input file\n");
        exit(EXIT_SUCCESS);
    }
    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    // Look for the file size
    fseek(input_file, 0, SEEK_END);
    size_t file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    // Allocate a buffer according to the file size 
    // (dont use astronomically big files for testing or buy more ram)
    char* json_str = (char*)malloc(file_size + 1);
    if (json_str == NULL) {
        perror("Failed to allocate memory");
        fclose(input_file);
        return 1;
    }

    // Read the content of the file into the buffer
    size_t nread = fread(json_str, sizeof(char), file_size, input_file);
    if (nread != file_size) {
        perror("Failed to read file");
        free(json_str);
        fclose(input_file);
        return 1;
    }
    json_str[file_size] = '\0';

    Jacon_Node root = {0};
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int ret = Jacon_parse_input(&root, json_str);
    gettimeofday(&end, NULL);
    double parse_timing =
        (double)((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000;
    printf("Parse time: %lf ms\n", parse_timing);

    print_error(ret);
    if (ret != JACON_OK) return ret;

    return JACON_OK;
}