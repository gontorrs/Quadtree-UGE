#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifndef _UCHAR_
	#define _UCHAR_
	#include <limits.h>
	typedef unsigned char uchar;
#endif

typedef struct { uchar* ptr; size_t capa; } BitStream;

uchar getbit(uchar,size_t);
void setbit(unsigned char*, size_t, int);
void case1(BitStream*);
void case2(BitStream*);
void case3(BitStream*);
void case4(BitStream*);
size_t pushbits(BitStream*, uchar, size_t);
size_t pullbits(BitStream*, uchar*, size_t);
int encode(uchar*, uchar*, int);