#include "nes_game.h"

const nesGameFile gameFileList[4] = {
    {"SuperMario",      acSuperMario},
    {"CatAndMouse",     acCatAndMouse},
    {"Tanks",           acTanks},
    {"MacrossSeries",   acMacrossSeries},
};

nesGame nes_game[4] = {
    {(&gameFileList[0]),   0},
    {(&gameFileList[1]),   0},
    {(&gameFileList[2]),   0},
    {(&gameFileList[3]),   0},
};

//nesGameFIL  NesFile;

//char nesOpenFile(char * fileName, pNesGameFIL pngf)
//{
//    
//}

int nesReadFile(void *buf, unsigned int len, unsigned short num, pNesGame png) 
{
    volatile char *p = buf;
    
    if (png->gameFile->gameFileSrc == 0) {
        return -1;
    }
    
    for (int i = 0; i < num; i++ ) {
        memcpy((char *)p, png->gameFile->gameFileSrc + png->index , len );
        png->index += len;
        p += len;
    }
    
    return 0;
}



