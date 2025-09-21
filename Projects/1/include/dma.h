#ifndef DMA_H
#define DMA_H

#include "types.h"

//Starts the DMA transfer
void initiateDMA(word* source, word destination, int size);

//Transfers size amount of memory from source location to destination location
void dmaTransfer(word* source, word destination, int size);
#endif
