#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define BASE_LAYER(tree) (((1 << (2 * ((tree)->levels - 1))) - 1) / 3)

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

int calculateTreeSize(int levels);
unsigned char* readPGM(const char* filename, int* width, int* height, int* maxGray);
Quadtree* initializeQuadtree(int levels);
void encodePixmapToQuadtreeAscending(unsigned char* pixmap, int width, Quadtree* tree);
void writeQuadtreeToQTC(const char* filename, Quadtree* tree, const char* identification_code, int width, int height, int levels);

#endif
