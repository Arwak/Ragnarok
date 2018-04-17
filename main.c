#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filesystem/filesystem.h"

#define INFO "-info"
#define SEARCH "-search"

#define INFO_CODE 0
#define SEARCH_CODE 1
#define ERROR_CODE -1

#define N_ARGS_INFO      3
#define N_ARGS_SEARCH    4

#define MSG_ERR_ARGS        "Invalid number of arguments.\n"
#define MSG_ERR_OPERATION   "Unknown operation.\n"


int switchOperation(char * string) {

    if ( strcmp(INFO, string) == 0) {
        return INFO_CODE;
    } else {
        return ERROR_CODE;
    }

}

int main(int argc, char * argv[]) {

    switch (switchOperation(argv[1])) {
        case INFO_CODE:
            if (argc != N_ARGS_INFO) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], INFO_CODE);

            break;
        case SEARCH_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }

            break;
        default:
            printf(MSG_ERR_OPERATION);
            break;
    }

}