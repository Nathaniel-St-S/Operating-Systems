#include "../include/interrupts.h"
#include "../include/memory.h"
#include "../include/cpu.h"
#include "../include/dma.h"

//Transfers size amount of memory from source to destination
void dmaTransfer(word source, word destination, int size) {
    for (int i = 0; i < size; i++) {
        write_mem(destination, read_mem(source));
        destination++;
        source++;
    }
}

//Starts the DMA transfer
void initiateDMA(word* source, word* destination, int size) {
    dmaTransfer(source, destination, size);
}
