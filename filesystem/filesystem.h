//
// Created by Clàudia Peiró Vidal on 13/4/18.
//

#ifndef RAGNAROK_FILESYSTEM_H
#define RAGNAROK_FILESYSTEM_H


#include "ext.h"
#include "fat.h"

#define MSG_ERR_FITXER                  "Invalid File\n"





void chooseFilesystem (char * pathFile, char * pathFileToFind, char * date,int operation);


#endif //RAGNAROK_FILESYSTEM_H
