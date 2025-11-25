#include "../include/process.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static List *Ready_Queue = NULL;

static PCB P1 = {0, 1, 10, "Ready", 0, 8, 3.2};
/*
List cons(PCB elem, List L) {
    return List(elem, L);
}

int isEmpty() {
    List->rest == NULL;
}
*/
//printf(Ready_Queue == NULL);
//READYQ;