#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Pixnode{
    unsigned char m;
    unsigned char e : 2;
    unsigned char u : 1;
}Pixnode;

typedef struct Quadtree{
    Pixnode* Pixels;
    int treesize;
    int levels;
}Quadtree;


// Function to read a PGM file and return pixel data
unsigned char* readPGM(const char* filename, int* width, int* height, int* maxGray) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    char magicNumber[3];
    fscanf(file, "%2s", magicNumber);
    if (strcmp(magicNumber, "P5") != 0) {
        fprintf(stderr, "Unsupported format: %s (only binary PGM supported)\n", magicNumber);
        fclose(file);
        return NULL;
    }

    // Skip comments
    int c;
    while ((c = fgetc(file)) == '#') {
        while (fgetc(file) != '\n');
    }
    ungetc(c, file);

    // Read image dimensions and max gray level
    fscanf(file, "%d %d %d", width, height, maxGray);
    if (*width != *height || (*width & (*width - 1)) != 0) {
        fprintf(stderr, "Error: Only square images of size 2^n x 2^n are supported.\n");
        fclose(file);
        return NULL;
    }

    // Read pixel data
    fgetc(file); 
    size_t dataSize = (*width) * (*height);
    unsigned char* data = (unsigned char*)malloc(dataSize);
    if (!data) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    fread(data, 1, dataSize, file);
    fclose(file);

    return data;
}

// Function to initialize the QuadTree
Quadtree* initializeQuadtree(int levels) {
    if (levels <= 0) {
        printf("Error: Levels must be greater than 0.\n");
        return NULL;
    }

    // Allocate memory for the QuadTree
    Quadtree* tree = (Quadtree*)malloc(sizeof(Quadtree));
    if (!tree) {
        printf("Error: Memory allocation failed for QuadTree.\n");
        return NULL;
    }

    // Calculate the tree size
    tree->levels = levels;
    tree->treesize = calculateTreeSize(levels);

    // Allocate memory for the pixels
    tree->Pixels = (Pixnode*)malloc(tree->treesize * sizeof(Pixnode));
    if (!tree->Pixels) {
        printf("Error: Memory allocation failed for pixels.\n");
        free(tree);
        return NULL;
    }

    // Initialize pixels to default values
    for (int i = 0; i < tree->treesize; i++) {
        tree->Pixels[i].m = 0; 
        tree->Pixels[i].u = 1;
        tree->Pixels[i].e = 0; 
    }

    return tree;
}


int main(int argc, char **argv){
    if (argc!= 3) {
        fprintf(stderr, "Usage: %s <input_pgm_file> <output_quadtree_file>\n", argv[0]);
        return 1;
    }

    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];

    int width, height, maxGray;
    unsigned char* pixmap = readPGM(inputFilename, &width, &height, &maxGray);
    if (!pixmap) {
        return 1;
    }

    int levels = log2(width) + 1;

    // TODO: Implement Quadtree compression and write to output file

    free(pixmap);
    return 0;
}