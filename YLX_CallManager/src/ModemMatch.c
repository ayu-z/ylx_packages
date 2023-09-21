#include "ModemMatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum model{
    RM500U,
    SRM821,
    SRM825,
};

int matchlist[][3] = {
    {RM500U},{0x2c7c,0x0900,2},
    {SRM821},{0x2dee,0x4d52,2},
};

int ModemInMatchList(int idProduct, int idVendor)
{


}
