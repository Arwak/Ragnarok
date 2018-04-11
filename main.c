#include <stdio.h>
#include <stdlib.h>


#define N_ARGS      3

#define MSG_ERR_ARGS        "Invalid number of arguments.\n"
#define MSG_ERR_FITXER      "Error amb el fitxer\n"


int main(int argc, char * argv[]) {

    if (argc != N_ARGS) {
        printf(MSG_ERR_ARGS);
        return EXIT_FAILURE;
    }
}