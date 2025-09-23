#ifndef DMA_H
#define DMA_H

#include "types.h"

//Starts the DMA transfer
void initiateDMA(word* source, word* destination, int size);

#endif
