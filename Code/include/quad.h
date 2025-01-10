#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define BASE_LAYER(tree) (((1 << (2 * ((tree)->levels - 1))) - 1) / 3)

typedef struct WriteLog {
    char type;
    int index;
} WriteLog;

typedef struct Pixnode {
    unsigned char m;
    unsigned char e : 2;
    unsigned char u : 1;
} Pixnode;

typedef struct Quadtree {
    Pixnode* Pixels;
    long long int treesize;
    int levels;
} Quadtree;

typedef unsigned char uchar;

int calculateTreeSize(int);
unsigned char* readPGM(const char*, int*, int*, int*);
Quadtree* initializeQuadtree(int levels);
void encodePixmapToQuadtreeAscending(unsigned char*, int, Quadtree*);
void packNodeData(Quadtree*, uchar*, WriteLog*, int*);
void writeQuadtreeToQTC(const char*, Quadtree*, const char*, int, int, int);

#endif
