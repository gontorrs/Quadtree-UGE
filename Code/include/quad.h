#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "compress.h" 
#define BASE_LAYER(tree) (((1 << (2 * ((tree)->levels - 1))) - 1) / 3)



typedef struct Pixnode {
    unsigned char m;
    unsigned char e : 2;
    unsigned char u : 1;
    double variance;
} Pixnode;

typedef struct Quadtree {
    Pixnode* Pixels;
    long long int treesize;
    int levels;
} Quadtree;

// Declarations of Quadtree-related functions
int calculateTreeSize(int);
unsigned char* readPGM(const char*, int*, int*, int*, bool);
Quadtree* initializeQuadtree(int levels);
void encodePixmapToQuadtreeAscending(unsigned char* let, int num, Quadtree* tree);
void packNodeData(Quadtree*, uchar*, WriteLog*, int*, bool verbose);
void writeQuadtreeToQTC(const char*, Quadtree*, const char*, int, int, int, bool verbose);
double calculateCompressionRate(int originalSize, int compressedSize, bool verbose);
void createPixel(Pixnode *parent, Pixnode *tl, Pixnode *tr, Pixnode *br, Pixnode *bl);
void pixmapToQuadtree(unsigned char *pixmap, int width, Quadtree *tree, int nodeIndex, int x, int y, int size, int level);
void printQuadtree(Quadtree *tree, int nodeIndex, int level);
void get_current_datetime(char *buffer, size_t buffer_size);
long findLastPositionAfterNewline(FILE *file);
int readLevels(FILE *file, Quadtree *tree);
int allocateMemoryForQuadtree(Quadtree *tree);
size_t readCompressedData(FILE *file, unsigned char *compressedData, size_t dataSize);
void decodePixels(BitStream *stream, Quadtree *tree);
void decodeQTCtoQuadtree(const char *filename, Quadtree *tree);
void fillPixelMatrixFromQuadtree(Quadtree *tree, int **pixelMatrix, int size, int x, int y, long long int nodeIndex, int level);
void writePGMFile(const char *filename, int **pixelMatrix, int size);

#endif
