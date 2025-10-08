#include "../include/memory.h"
#include "../include/dma.h"

//Transfers size amount of memory from source location to destination location
void dmaTransfer(word* source, word* destination, int size) {
    for (int i = 0; i < size; i++) {
        *destination = *source;
        destination++;
        source++;
    }
}

//Starts the DMA transfer
void initiateDMA(word* source, word* destination, int size) {
    dmaTransfer(source, destination, size);
}
