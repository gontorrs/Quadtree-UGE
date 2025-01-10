#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quad.h"
#include "compress.h"

unsigned char* readQTC(const char* filename, int* width, int* height, int* levels);
Quadtree* decodeQTC(unsigned char* data, int width, int height, int levels);
void quadtreeToPGM(Quadtree* tree, int width, int height, const char* filename);
void decodeQTCToPGM(const char* qtcFilename, const char* pgmFilename);

#endif