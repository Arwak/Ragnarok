#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INFO "-info"
#define INFO_CODE 0
#define ERROR_CODE -1
#define N_ARGS      3
#define MSG_ERR_ARGS        "Invalid number of arguments.\n"
#define MSG_ERR_FITXER      "Error amb el fitxer\n"
#define EXT4_PADDING_SUPER_BLOCK        1024
#define EXT4_MAGIC_NUMBER_OFFSET        0x38

int switchOperation(char * string) {

    if ( strcmp(INFO, string) == 0) {
        return INFO_CODE;
    } else {
        return ERROR_CODE;
    }

}

void showInformation(char * string) {
    FILE* file;
    char out[16];

    file = fopen(string, "rb");

    if (file == NULL) {
        printf(MSG_ERR_FITXER);
    } else {


        fseek(file, EXT4_MAGIC_NUMBER_OFFSET + EXT4_PADDING_SUPER_BLOCK, SEEK_SET);
        fread(out, 16, 1, file);
        printf(out);
        fclose(file);
    }

}

int main(int argc, char * argv[]) {

    if (argc != N_ARGS) {
        printf(MSG_ERR_ARGS);
        return EXIT_FAILURE;
    }
    switch (switchOperation(argv[1])) {
        case INFO_CODE:
            showInformation(argv[2]);
            break;
        default:

            break;
    }

}