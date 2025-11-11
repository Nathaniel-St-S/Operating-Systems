#ifndef DMA_H
#define DMA_H

#include "types.h"

//Starts the DMA transfer
void initiateDMA(dword* source, dword* destination, int size);

#endif
