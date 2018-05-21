#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "filesystem/filesystem.h"

#define INFO_STRING "-info"
#define SEARCH_STRING "-search"
#define DEEP_SEARCH_STRING "-deepsearch"
#define SHOW_STRING "-show"
#define READ "-r"
#define WRITE "-w"
#define HIDE "-h"
#define SHIDE "-s"
#define DATEN "-d"

#define INFO_CODE 0
#define SEARCH_CODE 1
#define DEEP_CODE 2
#define SHOW_CODE 3
#define ERROR_CODE -1





#define N_ARGS_INFO      3
#define N_ARGS_SEARCH    4
#define N_ARGS_DATE      5

#define MSG_ERR_ARGS        "Error: Incorrect parameter number.\n"
#define MSG_ERR_OPERATION   "Error: Unknown operation.\n"
#define MSG_ERR_DATE        "Error: Invalid date format, must be ddmmyyyy.\n"


int switchOperation(char * string) {

    if ( strcmp(INFO_STRING, string) == 0) {
        return INFO_CODE;
    } else {
        if (strcmp(SEARCH_STRING, string) == 0) {
            return SEARCH_CODE;
        }

        if (strcmp(DEEP_SEARCH_STRING, string) == 0) {
            return DEEP_CODE;
        }

        if (strcmp(SHOW_STRING, string) == 0) {
            return SHOW_CODE;
        }

        if (strcmp(READ, string) == 0) {
            return READ_CODE;
        }

        if (strcmp(WRITE, string) == 0) {
            return WRITE_CODE;
        }

        if (strcmp(HIDE, string) == 0) {
            return HIDE_CODE;
        }

        if (strcmp(SHIDE, string) == 0) {
            return SHIDE_CODE;
        }

        if (strcmp(DATEN, string) == 0) {
            return DATE_CODE;
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
            chooseFilesystem(argv[2], argv[3],  "0", INFO_CODE);

            break;
        case SEARCH_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3],  "0", SEARCH_CODE);
            break;
        case DEEP_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3], "0",  DEEP_CODE);
            break;
        case SHOW_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3],  "0", SHOW_CODE);
            break;
        case READ_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3],  "0", READ_CODE);
            break;
        case WRITE_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3],  "0", WRITE_CODE);
            break;
        case HIDE_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3], "0", HIDE_CODE);
            break;
        case SHIDE_CODE:
            if (argc != N_ARGS_SEARCH) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[2], argv[3], "0", SHIDE_CODE);
            break;
        case DATE_CODE:
            if (argc != N_ARGS_DATE) {
                printf(MSG_ERR_ARGS);
                return EXIT_FAILURE;
            }

            if (strlen(argv[2]) != 8) {
                printf(MSG_ERR_DATE);
                return EXIT_FAILURE;
            }
            chooseFilesystem(argv[3], argv[4], argv[2], DATE_CODE);
            break;

        default:
            printf(MSG_ERR_OPERATION);
            break;
    }

}