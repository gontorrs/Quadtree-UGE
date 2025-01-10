#include "decode.h"

unsigned char* readQTC(const char* filename, int* width, int* height, int* levels) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char id[3];
    fread(id, sizeof(char), 2, file);
    id[2] = '\0';

    if (strcmp(id, "Q1") != 0) {
        fclose(file);
        fprintf(stderr, "Invalid file format\n");
        return NULL;
    }

    fread(width, sizeof(int), 1, file);
    fread(height, sizeof(int), 1, file);
    fread(levels, sizeof(int), 1, file);

    int dataSize;
    fread(&dataSize, sizeof(int), 1, file);

    unsigned char* data = (unsigned char*)malloc(dataSize);
    fread(data, sizeof(unsigned char), dataSize, file);

    fclose(file);
    return data;
}
Quadtree* decodeQTC(unsigned char* data, int width, int height, int levels) {
    Quadtree* tree = initializeQuadtree(levels);
    if (!tree) {
        return NULL;
    }

    BitStream stream = {data, 0};
    int totalNodes = calculateTreeSize(levels);

    for (int i = 0; i < totalNodes; i++) {
        unsigned char value;
        pullbits(&stream, &value, 8); // Assuming each node is encoded in 8 bits
        tree->Pixels[i].m = value;
    }

    return tree;
}
void quadtreeToPGM(Quadtree* tree, int width, int height, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "P5\n%d %d\n255\n", width, height);

    int baseLayer = BASE_LAYER(tree);
    for (int i = 0; i < baseLayer; i++) {
        fputc(tree->Pixels[i].m, file);
    }

    fclose(file);
}
void decodeQTCToPGM(const char* qtcFilename, const char* pgmFilename) {
    int width, height, levels;
    unsigned char* data = readQTC(qtcFilename, &width, &height, &levels);
    if (!data) {
        return;
    }

    Quadtree* tree = decodeQTC(data, width, height, levels);
    if (!tree) {
        free(data);
        return;
    }

    quadtreeToPGM(tree, width, height, pgmFilename);

    free(data);
    free(tree->Pixels);
    free(tree);
}

