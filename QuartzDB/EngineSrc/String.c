#include "String.h"

int strcpyA(char* dst, char* src)
{
    int charCnt = 0;

    do{
        *dst = *src;
        
        if (*dst == 0)
            break;

        dst++; src++; charCnt++;
    }while(1);

    return charCnt;
}

int strcpyW(short* dst, short* src)
{
    int charCnt = 0;

    do{
        *dst     = *src;
        *(dst+1) = *(src+1);

        if ((*dst == 0) && (*(dst+1)==0))
            break;

        dst+=2; src+=2; charCnt++;
    }while(1);

    return charCnt;
}

