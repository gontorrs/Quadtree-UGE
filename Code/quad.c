#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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

// Function to calculate the total number of nodes in the quadtree
int calculateTreeSize(int levels) {
    if (levels <= 0) {
        return 0; // If no levels, there are no nodes
    }
    
    // Calculate the number of nodes using the formula for the sum of the geometric series
    return (pow(4, levels) - 1) / 3;
}

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

    // Skip whitespace and comments
    int c;
    while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n');
    if (c == '#') {
        // Skip comment line
        while (fgetc(file) != '\n');
        // Skip any additional whitespace
        while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n');
    }
    // Unget the last character read
    ungetc(c, file);

    // Read width, height, and max gray value
    if (fscanf(file, "%d %d %d", width, height, maxGray) != 3) {
        fprintf(stderr, "Error: Failed to read width, height, or max gray value\n");
        fclose(file);
        return NULL;
    }

    // Debug: Print the raw width, height, and max gray values
    printf("Width: %d, Height: %d, Max Gray: %d\n", *width, *height, *maxGray);

    // Read pixel data
    size_t dataSize = (*width) * (*height);
    unsigned char* data = (unsigned char*)malloc(dataSize);
    if (!data) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Skip to the next line and any whitespace before pixel data
    while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n');
    ungetc(c, file);

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

// Function to combine 4 neighboring nodes or pixels into a single quadtree node
void createPixel(Pixnode* parent, Pixnode* tl, Pixnode* tr, Pixnode* br, Pixnode* bl) {
    int sum = tl->m + tr->m + br->m + bl->m;
    parent->m = (unsigned char)(sum / 4); 
    parent->e = sum%4; 
    
    // Check if all child nodes have the same pixel value
    parent->u = ((tl->m == tr->m) && (tr->m == br->m) && (br->m == bl->m)) ? 1 : 0;
}

// Recursive function to build the quadtree in ascending order (suffix traversal)
void pixmapToQuadtree(unsigned char* pixmap, int width, Quadtree* tree, int nodeIndex, int x, int y, int size, int level) {
    // If we reach the smallest level (leaf nodes)
    if (size == 1) {
        tree->Pixels[nodeIndex].m = pixmap[y * width + x];
        tree->Pixels[nodeIndex].u = 1;  
        tree->Pixels[nodeIndex].e = 0;
        return;
    }

    int halfSize = size / 2;

    // Calculate indices for the 4 child blocks (clockwise order)
    int tlIndex = 4 * nodeIndex + 1;  
    int trIndex = tlIndex + 1;        
    int brIndex = tlIndex + 2;        
    int blIndex = tlIndex + 3;        

    // Recursively process the four quadrants (top-left, top-right, bottom-right, bottom-left)
    pixmapToQuadtree(pixmap, width, tree, tlIndex, x, y, halfSize, level + 1);                // Top-left
    pixmapToQuadtree(pixmap, width, tree, trIndex, x + halfSize, y, halfSize, level + 1);      // Top-right
    pixmapToQuadtree(pixmap, width, tree, brIndex, x + halfSize, y + halfSize, halfSize, level + 1);  // Bottom-right
    pixmapToQuadtree(pixmap, width, tree, blIndex, x, y + halfSize, halfSize, level + 1);      // Bottom-left

    // Combine the four quadrants into the parent node
    createPixel(&tree->Pixels[nodeIndex], &tree->Pixels[tlIndex], &tree->Pixels[trIndex], &tree->Pixels[brIndex], &tree->Pixels[blIndex]);
}

// Wrapper function to encode the pixmap into the quadtree
void encodePixmapToQuadtreeAscending(unsigned char* pixmap, int width, Quadtree* tree) {
    if (!pixmap || !tree) {
        printf("Error: Invalid inputs to encodePixmapToQuadtree.\n");
        return;
    }

    int size = width;  // Assuming square images (2^n x 2^n)
    pixmapToQuadtree(pixmap, width, tree, 0, 0, 0, size, 0);  // Start from the root node (level 0)
}

void printQuadtree(Quadtree* tree, int nodeIndex, int level) {
    Pixnode* node = &tree->Pixels[nodeIndex];
    printf("Level %d: Node %d -> m: %d, e: %d, u: %d\n", level, nodeIndex, node->m, node->e, node->u);

    // If the node is non-uniform, recurse into the four child nodes
    if (node->e == 1) {
        int childIndex = 4 * nodeIndex + 1;
        printQuadtree(tree, childIndex, level + 1);
        printQuadtree(tree, childIndex + 1, level + 1);
        printQuadtree(tree, childIndex + 2, level + 1);
        printQuadtree(tree, childIndex + 3, level + 1);
    }
}


// Function to get the current date and time in a string format
void get_current_datetime(char* buffer, size_t buffer_size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, buffer_size, "%d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

// Function to write the quadtree to a .qtc file
void writeQuadtreeToQTC(const char* filename, Quadtree* tree, const char* identification_code, int width, int height, int levels) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return;
    }

    // Step 1: Write the identification code (e.g., Q1)
    fwrite(identification_code, sizeof(char), 2, file);

    // Step 4: Write the metadata as comments
    fprintf(file, "\n# Metadata: Date and Time of Creation\n");
    char datetime[20];
    get_current_datetime(datetime, sizeof(datetime));
    fprintf(file, "# Created on: %s\n", datetime);
    fprintf(file, "# Compression Rate: <compression_rate_here>\n");
    fprintf(file, "# This file contains quadtree data for an image.\n");

    // Step 2: Write the image size (width, height, number of levels)
    fwrite(&width, sizeof(int), 1, file);
    fwrite(&height, sizeof(int), 1, file);
    fwrite(&levels, sizeof(int), 1, file);


    // Step 3: Write the quadtree data (nodes of the quadtree)
    for (int i = 0; i < tree->treesize; i++) {
        Pixnode* node = &tree->Pixels[i];
        unsigned char e = node->e;
        unsigned char u = node->u;

        fwrite(&node->m, sizeof(unsigned char), 1, file);  // Write the average pixel value
        fwrite(&e, sizeof(unsigned char), 1, file);        // Write the error flag
        fwrite(&u, sizeof(unsigned char), 1, file);        // Write the uniformity flag
    }

    fclose(file);
}


int main(int argc, char **argv) {
    // Step 1: Read the PGM image
    int width, height, maxGray;
    unsigned char* pixmap = readPGM("/home/mario/Documents/QuadTree/PGM/TEST4x4.pgm", &width, &height, &maxGray);
    if (!pixmap) {
        return -1; // Handle error if image is not read
    }

    // Step 2: Calculate the number of levels for the quadtree (log2 of the width)
    // We assume the width and height of the image are equal and powers of 2 (e.g., 4x4, 8x8)
    int levels = log2(width) + 1;  // For a 4x4 image, levels would be 3

    // Step 3: Initialize the quadtree
    Quadtree* tree = initializeQuadtree(levels);
    if (!tree) {
        free(pixmap);
        return -1; // Handle error if quadtree cannot be initialized
    }

    // Step 4: Encode the image into the quadtree
    encodePixmapToQuadtreeAscending(pixmap, width, tree);

    // Step 5: Print the quadtree structure to verify it
    printf("Quadtree Structure:\n");
    printQuadtree(tree, 0, 0); // Print the quadtree starting from the root (node index 0)
    writeQuadtreeToQTC("TEST4x4.qtc", tree, "Q1", width, height, levels);

    // Step 6: Cleanup
    free(pixmap);
    free(tree->Pixels);
    free(tree);

    return 0;
}