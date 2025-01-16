#define JACON_IMPLEMENTATION
#include "jacon.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void
print_error(Jacon_Error error)
{
    switch (error) { 
        case JACON_ERR_INDEX_OUT_OF_BOUND:
            puts("Jacon error: JACON_INDEX_OUT_OF_BOUND");
            break;
        case JACON_ERR_ALLOC:
            puts("Jacon error: JACON_ALLOC_ERROR");
            break;
        case JACON_ERR_CHAR_NOT_FOUND:
            puts("Jacon error: JACON_ERR_CHAR_NOT_FOUND");
            break;
        case JACON_ERR_INVALID_JSON:
            puts("Jacon error: JACON_ERR_INVALID_JSON");
            break;
        case JACON_NO_MORE_TOKENS:
            puts("Jacon error: JACON_NO_MORE_TOKENS");
            break;
        case JACON_ERR_NULL_PARAM:
            puts("Jacon error: JACON_NULL_PARAM");
            break;
        case JACON_ERR_INVALID_ESCAPE_SEQUENCE:
            puts("Jacon error: JACON_ERR_INVALID_ESCAPE_SEQUENCE");
            break;
        case JACON_ERR_UNREACHABLE_STATEMENT:
            puts("Jacon error: JACON_ERROR_UNREACHABLE_STATEMENT");
            break;
        case JACON_END_OF_INPUT:
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

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        perror("fstat failed");
        return -1;
    }
    off_t file_size = fileStat.st_size;

    // Allocate a buffer according to the file size 
    // (dont use astronomically big files for testing or buy more ram)
    char* json_str = (char*)malloc(file_size + 1 * sizeof(char));
    if (json_str == NULL) {
        perror("malloc error");
        return JACON_ERR_ALLOC;
    }

    ssize_t nrread = read(fd, json_str, sizeof(char) * file_size);
    if (nrread != file_size) {
        perror("Failed to read file");
        free(json_str);
        close(fd);
        return 1;
    }

    // TODO : move in parsing process
    const char *found = memchr(json_str, '\0', file_size);
    if (found != NULL) exit(EXIT_FAILURE);
    json_str[file_size] = '\0';

    Jacon_Node root = {0};
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int ret = Jacon_parse_input(&root, json_str);
    gettimeofday(&end, NULL);
    
    double parse_timing =
        (double)((end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec) / 1000;
    
    print_error(ret);

    free(json_str);
    Jacon_free_node(&root);
    if (ret != JACON_OK) return ret;

    printf("Parse time: %lf ms\n", parse_timing);

    return JACON_OK;
}