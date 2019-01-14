#ifndef __NES_GAME_H
#define __NES_GAME_H

#include <string.h>
//#include <stdlib.h>

#define GAME_FILE_NAME_MAXLEN       20
#define GAME_FILE_NUM               1

typedef struct {
    char gameName[GAME_FILE_NAME_MAXLEN];
    const unsigned char *gameFileSrc;
} nesGameFile;


typedef struct {
    const nesGameFile  *gameFile;
    unsigned int  index;
} nesGame, *pNesGame;


typedef struct {
    pNesGame *pGameList;
    pNesGame  pCurrentGame;
} nesGameFIL, pNesGameFIL;


extern nesGame nes_game[];
extern const unsigned char acSuperMario[];
extern const unsigned char acCatAndMouse[];
extern const unsigned char acTanks[];
extern const unsigned char acMacrossSeries[];

int nesReadFile(void *buf, unsigned int len, unsigned short num, pNesGame png);



#endif
