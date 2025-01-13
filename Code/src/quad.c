#include "quad.h"
#include "compress.h"
#include "decode.h"

double calculateCompressionRate(int originalSize, int compressedSize)
{
    if (originalSize == 0)
    {
        return 0.0; // Avoid division by zero
    }
    printf("Compression Rate %d / %d : %f\n", compressedSize / 8,  originalSize / 8, (double)compressedSize / originalSize * 100);
    return (double)compressedSize / originalSize * 100;
}

// Function to calculate the total number of nodes in the quadtree
int calculateTreeSize(int levels)
{
    if (levels <= 0)
    {
        return 0; // Si no hay niveles, no hay nodos
    }
    return (pow(4, levels) - 1) / 3;
}

// Function to read a PGM file and return pixel data
unsigned char *readPGM(const char *filename, int *width, int *height, int *maxGray)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening file");
        return NULL;
    }

    char magicNumber[3];
    fscanf(file, "%2s", magicNumber);
    if (strcmp(magicNumber, "P5") != 0)
    {
        fprintf(stderr, "Unsupported format: %s (only binary PGM supported)\n", magicNumber);
        fclose(file);
        return NULL;
    }

    // Skip whitespace and comments
    int c;
    while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n')
        ;
    if (c == '#')
    {
        // Skip comment line
        while (fgetc(file) != '\n')
            ;
        // Skip any additional whitespace
        while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n')
            ;
    }
    // Unget the last character read
    ungetc(c, file);

    // Read width, height, and max gray value
    if (fscanf(file, "%d %d %d", width, height, maxGray) != 3)
    {
        fprintf(stderr, "Error: Failed to read width, height, or max gray value\n");
        fclose(file);
        return NULL;
    }

    // Read pixel data
    size_t dataSize = (*width) * (*height);
    unsigned char *data = (unsigned char *)malloc(dataSize);
    if (!data)
    {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Skip to the next line and any whitespace before pixel data
    while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n')
        ;
    ungetc(c, file);

    fread(data, 1, dataSize, file);
    fclose(file);
    return data;
}

// Function to initialize the QuadTree
Quadtree *initializeQuadtree(int levels)
{
    if (levels <= 0)
    {
        printf("Error: Levels must be greater than 0.\n");
        return NULL;
    }

    // Allocate memory for the QuadTree
    Quadtree *tree = (Quadtree *)malloc(sizeof(Quadtree));
    if (!tree)
    {
        printf("Error: Memory allocation failed for QuadTree.\n");
        return NULL;
    }

    // Calculate the tree size
    tree->levels = levels;
    tree->treesize = calculateTreeSize(levels);

    // Allocate memory for the pixels
    tree->Pixels = (Pixnode *)malloc(tree->treesize * sizeof(Pixnode));
    if (!tree->Pixels)
    {
        printf("Error: Memory allocation failed for pixels.\n");
        free(tree);
        return NULL;
    }

    // Initialize pixels to default values
    for (int i = 0; i < tree->treesize; i++)
    {
        tree->Pixels[i].m = 0;
        tree->Pixels[i].u = 1;
        tree->Pixels[i].e = 0;
    }

    return tree;
}

// Function to combine 4 neighboring nodes or pixels into a single quadtree node
void createPixel(Pixnode *parent, Pixnode *tl, Pixnode *tr, Pixnode *br, Pixnode *bl)
{
    int sum = tl->m + tr->m + br->m + bl->m;
    parent->m = (unsigned char)(sum / 4);
    parent->e = sum % 4;

    // Check if all child nodes have the same pixel value
    parent->u = ((tl->m == tr->m) && (tr->m == br->m) && (br->m == bl->m)) ? 1 : 0;
}

// Recursive function to build the quadtree in ascending order (suffix traversal)
void pixmapToQuadtree(unsigned char *pixmap, int width, Quadtree *tree, int nodeIndex, int x, int y, int size, int level)
{
    // If we reach the smallest level (leaf nodes)
    if (size == 1)
    {
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
    pixmapToQuadtree(pixmap, width, tree, tlIndex, x, y, halfSize, level + 1);                       // Top-left
    pixmapToQuadtree(pixmap, width, tree, trIndex, x + halfSize, y, halfSize, level + 1);            // Top-right
    pixmapToQuadtree(pixmap, width, tree, brIndex, x + halfSize, y + halfSize, halfSize, level + 1); // Bottom-right
    pixmapToQuadtree(pixmap, width, tree, blIndex, x, y + halfSize, halfSize, level + 1);            // Bottom-left

    // Combine the four quadrants into the parent node
    createPixel(&tree->Pixels[nodeIndex], &tree->Pixels[tlIndex], &tree->Pixels[trIndex], &tree->Pixels[brIndex], &tree->Pixels[blIndex]);
}

// Wrapper function to encode the pixmap into the quadtree
void encodePixmapToQuadtreeAscending(unsigned char *pixmap, int width, Quadtree *tree)
{
    if (!pixmap || !tree)
    {
        printf("Error: Invalid inputs to encodePixmapToQuadtree.\n");
        return;
    }

    int size = width;                                        // Assuming square images (2^n x 2^n)
    pixmapToQuadtree(pixmap, width, tree, 0, 0, 0, size, 0); // Start from the root node (level 0)
}

void printQuadtree(Quadtree *tree, int nodeIndex, int level)
{
    Pixnode *node = &tree->Pixels[nodeIndex];

    // Print tabulation based on the level
    for (int i = 0; i < level; i++)
    {
        printf("   ");
    }

    printf("Level %d: Node %d -> m: %d, e: %d, u: %d\n", level, nodeIndex, node->m, node->e, node->u);

    // If the node is non-uniform, recurse into the four child nodes
    if (node->u == 0)
    {
        int childIndex = 4 * nodeIndex + 1;
        printQuadtree(tree, childIndex, level + 1);
        printQuadtree(tree, childIndex + 1, level + 1);
        printQuadtree(tree, childIndex + 2, level + 1);
        printQuadtree(tree, childIndex + 3, level + 1);
    }
}

// Function to get the current date and time in a string format
void get_current_datetime(char *buffer, size_t buffer_size)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, buffer_size, "%d-%02d-%02d %02d:%02d:%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void packNodeData(Quadtree *tree, uchar *uncompressed, WriteLog *log, int *logSize)
{
    int bufferIndex = 0;
    int baseLayerStartIndex = BASE_LAYER(tree);
    int logIndex = 0;

    // Write the root node
    uncompressed[bufferIndex] = tree->Pixels[0].m;
    log[logIndex++] = (WriteLog){'m', bufferIndex++};

    uncompressed[bufferIndex] = tree->Pixels[0].e;
    log[logIndex++] = (WriteLog){'e', bufferIndex++};

    if (tree->Pixels[0].e == 0)
    {
        uncompressed[bufferIndex] = tree->Pixels[0].u;
        log[logIndex++] = (WriteLog){'u', bufferIndex++};
    }

    // Skip writing child nodes if the root node has uniformity equals to 1
    if (tree->Pixels[0].u == 1) {
        *logSize = logIndex;
        return;
    }

    for (int i = 1; i < tree->treesize; i++)
    {
        Pixnode *node = &tree->Pixels[i];

        // Determine the parent index
        int parentIndex = (i - 1) / 4;

        // Skip if parent node has uniformity equals to 1
        if (tree->Pixels[parentIndex].u == 1) {
            continue;
        }

        // Handle the base layer case
        if (i >= baseLayerStartIndex) {
            // Skip writing anything if it's the 4th child in the base layer
            if (i % 4 == 0) {
                continue;
            }

            // Otherwise, only write m
            uncompressed[bufferIndex] = node->m;
            log[logIndex++] = (WriteLog){'m', bufferIndex++};
            continue;
        }

        // Handle the 4th child node case (non-base layer)
        if (i % 4 == 0) {
            uncompressed[bufferIndex] = node->e;
            log[logIndex++] = (WriteLog){'e', bufferIndex++};

            if (node->e == 0) {
                uncompressed[bufferIndex] = node->u;
                log[logIndex++] = (WriteLog){'u', bufferIndex++};
            }
            continue;
        }

        // Write the node's m value
        uncompressed[bufferIndex] = node->m;
        log[logIndex++] = (WriteLog){'m', bufferIndex++};

        // Write the e value
        uncompressed[bufferIndex] = node->e;
        log[logIndex++] = (WriteLog){'e', bufferIndex++};

        // Only write u if e == 0
        if (node->e == 0) {
            uncompressed[bufferIndex] = node->u;
            log[logIndex++] = (WriteLog){'u', bufferIndex++};
        }
    }

    *logSize = logIndex;
}







// Function to write the quadtree to a .qtc file
void writeQuadtreeToQTC(const char *filename, Quadtree *tree, const char *identification_code, int width, int height, int levels)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Error opening file for writing");
        return;
    }

    // Step 1: Write header
    fwrite(identification_code, sizeof(char), strlen(identification_code), file);
    fwrite("\n", sizeof(char), 1, file);

    // Step 2: Write metadata
    time_t now = time(NULL);
    char datetime[30];
    strftime(datetime, sizeof(datetime), "# %a %b %d %H:%M:%S %Y\n", localtime(&now));
    fwrite(datetime, sizeof(char), strlen(datetime), file);

    // Step 3: Prepare data for compression
    WriteLog* log = malloc(tree->treesize * 3 * sizeof(WriteLog)); // m, e, u
    uchar* uncompressed = malloc(tree->treesize * 3); // 3 bytes per node (m, e, u)
    uchar* compressed = malloc(tree->treesize * 3);    // Worst case compression buffer
    
    // Pack node data
    int logSize = 0;
    packNodeData(tree, uncompressed, log, &logSize);

    // Step 4: Compress data
    int compressedSize = encode(compressed, uncompressed, log, logSize);
    int compressedBytes = (compressedSize + 7) / 8; // Convert bits to bytes (round up)

    // Calculate compression rate
    int originalSize = width * height * 8;
    int paddedCompressedSize = (compressedSize + 7) & ~7;
    double compressionRate = calculateCompressionRate(originalSize, paddedCompressedSize);

    char compressionRateStr[50];
    snprintf(compressionRateStr, sizeof(compressionRateStr), "# compression rate %.2f%%\n", compressionRate);
    fwrite(compressionRateStr, sizeof(char), strlen(compressionRateStr), file);

    // Step 5: Write compressed data size and data
    levels = levels - 1;
    fwrite(&levels, sizeof(uchar), 1, file);
    fwrite(compressed, sizeof(uchar), compressedBytes, file);

    // Cleanup
    free(uncompressed);
    free(compressed);
    fclose(file);
}
// Function to open the file and find the position after the third newline
long findLastPositionAfterNewline(FILE *file) {
    unsigned char byte;
    int newlineCount = 0;
    long lastPosition = -1;

    while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
        if (byte == 0x0A) {
            newlineCount++;
            if (newlineCount == 3) {
                lastPosition = ftell(file);
                break;
            }
        }
    }

    return lastPosition;
}

// Function to read the number of levels from the file
int readLevels(FILE *file, Quadtree *tree) {
    unsigned char byte;
    if (fread(&byte, sizeof(unsigned char), 1, file) != 1) {
        fprintf(stderr, "Error: No se encontró el número de niveles en el archivo.\n");
        return -1;
    }

    tree->levels = byte;
    if (tree->levels <= 0) {
        fprintf(stderr, "Error: Número de niveles inválido (%d).\n", tree->levels);
        return -1;
    }

    return 0;
}

// Function to allocate memory for the quadtree pixels and compressed data
int allocateMemoryForQuadtree(Quadtree *tree) {
    tree->treesize = calculateTreeSize(tree->levels);

    tree->Pixels = (Pixnode *)malloc(tree->treesize * sizeof(Pixnode));
    if (!tree->Pixels) {
        perror("Error al asignar memoria para el Quadtree");
        return -1;
    }

    size_t dataSize = tree->treesize * 3;
    unsigned char *compressedData = (unsigned char *)malloc(dataSize);
    if (!compressedData) {
        perror("Error al asignar memoria para los datos comprimidos");
        free(tree->Pixels);
        return -1;
    }

    return 0;
}

// Function to read the compressed data from the file
size_t readCompressedData(FILE *file, unsigned char *compressedData, size_t dataSize) {
    size_t bytesRead = fread(compressedData, sizeof(unsigned char), dataSize, file);
    if (bytesRead == 0) {
        fprintf(stderr, "Error: No se pudieron leer los datos comprimidos.\n");
    }
    return bytesRead;
}

void decodePixels(BitStream *stream, Quadtree *tree) {
    for (long long i = 0; i < tree->treesize; i++) {
        Pixnode *current = &tree->Pixels[i];
        Pixnode *parent = NULL;
        if (i > 0) {
            parent = &tree->Pixels[(i - 1) / 4];
        }

        if (i == 0) { // Root node
            pullbits(stream, &current->m, 8);
            unsigned char eRaw = 0;
            pullbits(stream, &eRaw, 2);

            int e_signed = (int)eRaw;
            if (e_signed > 1) {
                e_signed -= 4;
            }

            current->e = (unsigned char)e_signed;

            if (e_signed == 0) {
                unsigned char uBit = 0;
                pullbits(stream, &uBit, 1);
                current->u = uBit;
            } else {
                current->u = 0;
            }
            continue;
        }

        if (parent->u == 1) { // Inherited state
            current->m = parent->m;
            current->e = 0;
            current->u = 1;
            continue;
        }

        if (i % 4 == 0) { // First child of a node
            if (i < BASE_LAYER(tree)) {
                unsigned char eRaw = 0;
                pullbits(stream, &eRaw, 2);

                int e_signed = (int)eRaw;
                if (e_signed > 1) {
                    e_signed -= 4;
                }

                current->e = (unsigned char)e_signed;

                if (e_signed == 0) {
                    unsigned char uBit = 0;
                    pullbits(stream, &uBit, 1);
                    current->u = uBit;
                } else {
                    current->u = 0;
                }
            } else {
                current->e = 0;
                current->u = 1;
            }

            // Interpolate `m` for the first child
            int mParent = (int)parent->m;
            int eParent = (int)parent->e;
            int m1 = (int)tree->Pixels[i - 1].m;
            int m2 = (int)tree->Pixels[i - 2].m;
            int m3 = (int)tree->Pixels[i - 3].m;
            int mCalc = 4 * (mParent + eParent) - (m1 + m2 + m3);

            if (mCalc < 0) {
                mCalc = 0;
            }
            if (mCalc > 255) {
                mCalc = 255;
            }

            current->m = (unsigned char)mCalc;
        } else { // Siblings
            pullbits(stream, &current->m, 8);

            if (i < BASE_LAYER(tree)) {
                unsigned char eRaw = 0;
                pullbits(stream, &eRaw, 2);

                int e_signed = (int)eRaw;
                if (e_signed > 1) {
                    e_signed -= 4;
                }

                current->e = (unsigned char)e_signed;

                if (e_signed == 0) {
                    unsigned char uBit = 0;
                    pullbits(stream, &uBit, 1);
                    current->u = uBit;
                } else {
                    current->u = 0;
                }
            } else {
                current->e = 0;
                current->u = 1;
            }
        }
    }
}


// Main function refactored to use smaller functions
void decodeQTCtoQuadtree(const char *filename, Quadtree *tree) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening .qtc file");
        return;
    }

    long lastPosition = findLastPositionAfterNewline(file);
    if (lastPosition == -1 || fseek(file, lastPosition, SEEK_SET) != 0) {
        fprintf(stderr, "Error: Could not find the position after the newlines.\n");
        fclose(file);
        return;
    }

    if (readLevels(file, tree) != 0) {
        fclose(file);
        return;
    }

    if (allocateMemoryForQuadtree(tree) != 0) {
        fclose(file);
        return;
    }

    size_t dataSize = tree->treesize * 3;
    unsigned char *compressedData = (unsigned char *)malloc(dataSize);
    size_t bytesRead = readCompressedData(file, compressedData, dataSize);
    fclose(file);

    if (bytesRead == 0) {
        free(tree->Pixels);
        free(compressedData);
        return;
    }
    BitStream stream = {compressedData, 0};
    decodePixels(&stream, tree);

    free(compressedData);
}




// Función para llenar la matriz de píxeles desde el quadtree
void fillPixelMatrixFromQuadtree(Quadtree *tree, int **pixelMatrix, int size, int x, int y, long long int nodeIndex, int level) {
    if (nodeIndex < 0 || nodeIndex >= tree->treesize) {
        fprintf(stderr, "Error: Índice del nodo (%lld) fuera de límites. Tamaño del árbol: %lld\n", nodeIndex, tree->treesize);
        return;
    }

    Pixnode *node = &tree->Pixels[nodeIndex];

    if (node->u == 1) { // Nodo uniforme
        int regionSize = size >> level;
        for (int i = 0; i < regionSize; i++) {
            for (int j = 0; j < regionSize; j++) {
                pixelMatrix[x + i][y + j] = node->m;
            }
        }
    } else { // Nodo interno
        int halfSize = size >> (level + 1);

        int tlIndex = 4 * nodeIndex + 1;
        int trIndex = 4 * nodeIndex + 2;
        int brIndex = 4 * nodeIndex + 3;
        int blIndex = 4 * nodeIndex + 4;

        if (tlIndex >= tree->treesize || trIndex >= tree->treesize ||
            brIndex >= tree->treesize || blIndex >= tree->treesize) {
            fprintf(stderr, "Error: Índice de hijo fuera de límites en fillPixelMatrixFromQuadtree.\n");
            return;
        }

        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x, y, tlIndex, level + 1);
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x, y + halfSize, trIndex, level + 1);
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x + halfSize, y + halfSize, brIndex, level + 1);
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x + halfSize, y, blIndex, level + 1);
    }
}

// Función para escribir el archivo .pgm
void writePGMFile(const char *filename, int **pixelMatrix, int size)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Error when writing on .pgm file.");
        return;
    }

    // Escribir encabezado del archivo PGM
    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", size, size);
    fprintf(file, "255\n");

    // Escribir la matriz de píxeles
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            fprintf(file, "%d ", pixelMatrix[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}
int main(int argc, char **argv)
{
    int width, height, maxGray;
<<<<<<< HEAD
    unsigned char* pixmap = readPGM("../PGM/passiflora.2048.pgm", &width, &height, &maxGray);
    if (!pixmap) {
=======
    unsigned char *pixmap = readPGM("../PGM/TEST4x4.pgm", &width, &height, &maxGray);
    if (!pixmap)
    {
>>>>>>> 17af677 (decode works, missing translation and organise code)
        return -1;
    }

    int levels = log2(width) + 1;

    Quadtree *tree = initializeQuadtree(levels);
    if (!tree)
    {
        free(pixmap);
        return -1;
    }

    encodePixmapToQuadtreeAscending(pixmap, width, tree);
<<<<<<< HEAD
    writeQuadtreeToQTC("passiflora.2048.qtc", tree, "Q1", width, height, levels);


=======
    writeQuadtreeToQTC("TEST4x4.qtc", tree, "Q1", width, height, levels);
>>>>>>> 17af677 (decode works, missing translation and organise code)
    free(pixmap);
    free(tree->Pixels);
    free(tree);

    // Decode the .qtc file back to a PGM file.

    const char *inputFilename = "../QTC.lossy/boat.512.a.qtc";
    const char *outputFilename = "output.pgm";

    Quadtree tree2;
    decodeQTCtoQuadtree(inputFilename, &tree2);
    int size = 1 << tree2.levels;
    int **pixelMatrix = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
    {
        pixelMatrix[i] = (int *)malloc(size * sizeof(int));
        memset(pixelMatrix[i], 0, size * sizeof(int));
    }
    fillPixelMatrixFromQuadtree(&tree2, pixelMatrix, size, 0, 0, 0, 0);
    writePGMFile(outputFilename, pixelMatrix, size);
    for (int i = 0; i < size; i++)
    {
        free(pixelMatrix[i]);
    }
    free(pixelMatrix);
    free(tree2.Pixels);

    return 0;
}