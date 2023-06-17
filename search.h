#ifndef SEARCH_H
#define SEARCH_H

#include <inttypes.h>
#include "level.h"
#include <stdio.h>
#include <stdlib.h>

#define PATH_NOT_FOUND (((uint32_t)-1) >> 1)
#define PATH_FOUND 123456789 //be careful when this is 0 (idk what happens then but sth weird)

#define PATH_SIZE (1<<18) //path repetition table size (262144 int64s => 2MB)
#define TT_SIZE ((1<<24) / sizeof(TTEntry)) //TT size quite small for now (16MB = 1M entries)
extern uint64_t path_nodes[PATH_SIZE];

typedef struct {
    uint64_t key; //8 byte key
    uint32_t cost; //4 byte cost
    uint32_t depth; //4 byte depth (unused right now)
} TTEntry;
extern TTEntry tt[TT_SIZE]; //transposition table

uint32_t idastar_heuristic();
uint32_t idastar_search(uint32_t bound);

#endif
