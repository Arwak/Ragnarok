#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filesystem/filesystem.h"

#define INFO "-info"
#define INFO_CODE 0
#define ERROR_CODE -1
#define N_ARGS      3
#define MSG_ERR_ARGS        "Invalid number of arguments.\n"


int switchOperation(char * string) {

    if ( strcmp(INFO, string) == 0) {
        return INFO_CODE;
    } else {
        return ERROR_CODE;
    }

}

int main(int argc, char * argv[]) {
    if (argc != N_ARGS) {
        printf(MSG_ERR_ARGS);
        return EXIT_FAILURE;
    }
    switch (switchOperation(argv[1])) {
        case INFO_CODE:
            chooseFilesystem(argv[2]);
            break;
        default:

            break;
    }

}