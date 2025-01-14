#define JACON_IMPLEMENTATION
#include "jacon.h"
#include <stdio.h>

int
main(int argc, const char** argv)
{
    // if (argc <= 1) {
    //     printf("Please specify an input file\n");
    //     exit(EXIT_SUCCESS);
    // } 
    // FILE* input_file = fopen(argv[1], "r");
    FILE* input_file = fopen("test-files/default.json", "r");
    if (input_file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    // Look for the file size
    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
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
    int ret = Jacon_parse_input(&root, json_str);
    if (ret != JACON_OK) return ret;
    // Do things

    return JACON_OK;
}