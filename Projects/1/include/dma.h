#ifndef DMA_H
#define DMA_H

//Transfers size amount of memory from source to destination
void dmaTransfer(int source, int destination, int size);

//Starts the DMA transfer
void initiateDMA(int* source, int* destination, int size);

#endif