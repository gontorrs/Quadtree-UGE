#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "quad.h"



typedef struct { 
	uchar* ptr; 
	size_t capa; 
} BitStream;

uchar getbit(uchar,size_t);
void setbit(unsigned char*, size_t, int);
size_t pushbits(BitStream*, uchar, size_t);
size_t pullbits(BitStream*, uchar*, size_t);
int encode(uchar*, uchar*, WriteLog*, int);
#endif