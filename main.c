#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filesystem/filesystem.h"

#define INFO "-info"
#define SEARCH "-search"
#define DEEP_SEARCH "-deepsearch"

#define INFO_CODE 0
#define SEARCH_CODE 1
#define DEEP_CODE 2
#define ERROR_CODE -1

#define N_ARGS_INFO      3
#define N_ARGS_SEARCH    4

#define MSG_ERR_ARGS        "Error: Incorrect parameter number.\n"
#define MSG_ERR_OPERATION   "Error: Unknown operation.\n"


int switchOperation(char * string) {

    if ( strcmp(INFO, string) == 0) {
        return INFO_CODE;
    } else {
        if (strcmp(SEARCH, string) == 0) {
            return SEARCH_CODE;
        }

        if (strcmp(DEEP_SEARCH, string) == 0) {
            return DEEP_CODE;
        }

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
            chooseFilesystem(argv[2], argv[3], INFO_CODE);

            break;
        case SEARCH_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3], SEARCH_CODE);
            break;
        case DEEP_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3], DEEP_CODE);
            break;

        default:
            printf(MSG_ERR_OPERATION);
            break;
    }

}