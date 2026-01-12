#ifndef BLOCKS_H
#define BLOCKS_H
#include "defrag.h"

int fix_single_indirect(unsigned char *new_disk, int og_block,int *next, int blocks,int bytes);
int fix_double_indirect(unsigned char *new_disk,int og_block,int *next, int bytes);
int fix_triple_indirect(unsigned char *new_disk,int og_block,int *next,int bytes);

#endif