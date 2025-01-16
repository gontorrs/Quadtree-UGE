#include "quad.h"
#include "filter.h"

double calculateCompressionRate(int originalSize, int compressedSize, bool verbose)
{
    if (originalSize == 0)
    {
        return 0.0;
    }
    if(verbose){
        printf("Original Size: %d bits\n", originalSize);
        printf("Compressed Size: %d bits\n", compressedSize);
        printf("Compression Rate: %d / %d = %.2f%%\n", compressedSize, originalSize, (double)compressedSize / originalSize * 100);
    }
    return (double)compressedSize / originalSize * 100;
}

// Function to calculate the total number of nodes in the quadtree
int calculateTreeSize(int levels)
{
    if (levels <= 0)
    {
        return 0;
    }
    return (pow(4, levels) - 1) / 3;
}

// Function to read a PGM file and return pixel data
unsigned char *readPGM(const char *filename, int *width, int *height, int *maxGray, bool verbose)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error opening file");
        return NULL;
    }

    char magicNumber[3];
    if (fscanf(file, "%2s", magicNumber) != 1) {
        fprintf(stderr, "Error reading magic number\n");
        fclose(file);
        return NULL;
    }
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
    if(verbose){
        printf("Width: %d, Height: %d, Max Gray: %d\n", *width, *height, *maxGray);
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
    while ((c = fgetc(file)) == ' ' || c == '\t' || c == '\n');
    ungetc(c, file);

    if (fread(data, 1, dataSize, file) != dataSize) {
    fprintf(stderr, "Error reading pixel data: expected %zu bytes, got less\n", dataSize);
    free(data);
    fclose(file);
    return NULL;
    }
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

    // A node is uniform ONLY if all children are uniform AND have the same value
    parent->u = (tl->u && tr->u && br->u && bl->u && 
                 (tl->m == tr->m) && (tr->m == br->m) && (br->m == bl->m)) ? 1 : 0;
}

// Recursive function to build the quadtree in ascending order (suffix traversal)
void pixmapToQuadtree(unsigned char* pixmap, int width, Quadtree* tree, int nodeIndex, int x, int y, int size, int level){
    if (size == 1) {
        // Leaf node: store direct pixel
        unsigned char pix = pixmap[y * width + x];
        tree->Pixels[nodeIndex].m = pix;
        tree->Pixels[nodeIndex].u = 1; 
        tree->Pixels[nodeIndex].e = 0;

        // Compute variance from a single pixel 
        // By definition, a single pixel is perfectly uniform => variance = 0
        tree->Pixels[nodeIndex].variance = 0.0;
        return;
    }

    int halfSize = size / 2;

    // Child indices in BFS order
    int tlIndex = 4 * nodeIndex + 1;  
    int trIndex = tlIndex + 1;        
    int brIndex = tlIndex + 2;        
    int blIndex = tlIndex + 3;        

    // Recurse down
    pixmapToQuadtree(pixmap, width, tree, tlIndex, 
                     x, y, halfSize, level + 1);            // Top-left
    pixmapToQuadtree(pixmap, width, tree, trIndex, 
                     x + halfSize, y, halfSize, level + 1);  // Top-right
    pixmapToQuadtree(pixmap, width, tree, brIndex, 
                     x + halfSize, y + halfSize, halfSize, level + 1);  // Bottom-right
    pixmapToQuadtree(pixmap, width, tree, blIndex, 
                     x, y + halfSize, halfSize, level + 1);  // Bottom-left

    // Combine the four children into the parent
    createPixel(&tree->Pixels[nodeIndex], 
                &tree->Pixels[tlIndex], 
                &tree->Pixels[trIndex],
                &tree->Pixels[brIndex], 
                &tree->Pixels[blIndex]);

    // Now compute the parent's variance 
    // using child means (m_k) and child variances (ν_k).
    // We'll demonstrate that next:
    double childMeans[4] = {
        tree->Pixels[tlIndex].m,
        tree->Pixels[trIndex].m,
        tree->Pixels[brIndex].m,
        tree->Pixels[blIndex].m
    };
    double childVars[4] = {
        tree->Pixels[tlIndex].variance,
        tree->Pixels[trIndex].variance,
        tree->Pixels[brIndex].variance,
        tree->Pixels[blIndex].variance
    };

    double parentMean = tree->Pixels[nodeIndex].m; // from createPixel()
    
    // Use the formula:  μ = Σ ( (ν_k)^2 + (m - m_k)^2 ),  then ν = sqrt(μ)/4
    // Let’s call a helper function:
    tree->Pixels[nodeIndex].variance = computeBlockVariance(parentMean, childMeans, childVars);
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
// Wrapper function to encode the pixmap into the quadtree
void encodePixmapToQuadtreeAscending(unsigned char *pixmap, int width, Quadtree *tree) {
    if (!pixmap || !tree) {
        printf("Error: Entradas inválidas a encodePixmapToQuadtreeAscending.\n");
        return;
    }

    int size = width; // Asumimos imágenes cuadradas (2^n x 2^n)
    pixmapToQuadtree(pixmap, width, tree, 0, 0, 0, size, 0);
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

void packNodeData(Quadtree *tree, uchar *uncompressed, WriteLog *log, int *logSize, bool verbose)
{
    int bufferIndex = 0;
    int baseLayerStartIndex = BASE_LAYER(tree);
    int logIndex = 0;

    // Local counters for m, e, and u writes
    int mWrites = 0, eWrites = 0, uWrites = 0;

    // Write the root node
    uncompressed[bufferIndex] = tree->Pixels[0].m;
    log[logIndex++] = (WriteLog){'m', bufferIndex++};
    mWrites++;

    uncompressed[bufferIndex] = tree->Pixels[0].e;
    log[logIndex++] = (WriteLog){'e', bufferIndex++};
    eWrites++;

    if (tree->Pixels[0].e == 0)
    {
        uncompressed[bufferIndex] = tree->Pixels[0].u;
        log[logIndex++] = (WriteLog){'u', bufferIndex++};
        uWrites++;
    }

    // Skip writing children if the root node has uniformity equals to 1
    if (tree->Pixels[0].u == 1)
    {
        *logSize = logIndex;
        return;
    }

    for (int i = 1; i < tree->treesize; i++)
    {
        Pixnode *node = &tree->Pixels[i];

        // IMPORTANT: Check if parent is uniform FIRST to stay in sync with decoder
        int parentIndex = (i - 1) / 4;
        if (tree->Pixels[parentIndex].u == 1)
        {
            continue;
        }

        // Skip the 4th node of each level - don't write m (calculated during decode)
        if (i % 4 == 0)
        {
            if (i < baseLayerStartIndex)
            {
                uncompressed[bufferIndex] = node->e;
                log[logIndex++] = (WriteLog){'e', bufferIndex++};
                eWrites++;

                // Only write u if e == 0
                if (node->e == 0)
                {
                    uncompressed[bufferIndex] = node->u;
                    log[logIndex++] = (WriteLog){'u', bufferIndex++};
                    uWrites++;
                }
            }
            continue;
        }

        // Write the node data
        uncompressed[bufferIndex] = node->m;
        log[logIndex++] = (WriteLog){'m', bufferIndex++};
        mWrites++;

        // At the base layer, skip writing node->e and node->u
        if (i >= baseLayerStartIndex)
        {
            continue;
        }

        uncompressed[bufferIndex] = node->e;
        log[logIndex++] = (WriteLog){'e', bufferIndex++};
        eWrites++;

        if (node->e == 0)
        {
            uncompressed[bufferIndex] = node->u;
            log[logIndex++] = (WriteLog){'u', bufferIndex++};
            uWrites++;
        }
    }

    *logSize = logIndex;

    if(verbose){
        printf("mWrites: %d, eWrites: %d, uWrites: %d\n", mWrites, eWrites, uWrites);
    }
}

// Function to write the quadtree to a .qtc file
void writeQuadtreeToQTC(const char *filename, Quadtree *tree, const char *identification_code, int width, int height, int levels, bool verbose)
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
    WriteLog *log = malloc(tree->treesize * 3 * sizeof(WriteLog)); // m, e, u
    uchar *uncompressed = malloc(tree->treesize * 3);              // 3 bytes per node (m, e, u)
    uchar *compressed = malloc(tree->treesize);                    // Worst case compression buffer

    // Pack node data
    int logSize = 0;
    packNodeData(tree, uncompressed, log, &logSize, verbose);

    // Step 4: Compress data
    int compressedSize = encode(compressed, uncompressed, log, logSize);
    int compressedBytes = (compressedSize + 7) / 8; // Convert bits to bytes (round up)

    // Calculate compression rate
    int originalSize = width * height * 8;
    int paddedCompressedSize = (compressedSize + 7) & ~7;
    double compressionRate = calculateCompressionRate(originalSize, paddedCompressedSize, verbose);

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
        fprintf(stderr, "Error: Number of levels could not be found int the .qtc file.\n");
        return -1;
    }

    tree->levels = byte + 1; // I am not counting the root node as a level when reading from the .qtc file so I add 1.
    if (tree->levels <= 0) {
        fprintf(stderr, "Error: Number of levels not valid (%d).\n", tree->levels);
        return -1;
    }

    return 0;
}

// Function to allocate memory for the quadtree pixels and compressed data
int allocateMemoryForQuadtree(Quadtree *tree) {
    tree->treesize = calculateTreeSize(tree->levels);

    tree->Pixels = (Pixnode *)malloc(tree->treesize * sizeof(Pixnode));
    if (!tree->Pixels) {
        perror("Error when assigning memory for the quadtree pixels.");
        return -1;
    }

    size_t dataSize = tree->treesize * 3;
    unsigned char *compressedData = (unsigned char *)malloc(dataSize);
    if (!compressedData) {
        perror("Error when assigning memory for compressed data.");
        free(tree->Pixels);
        return -1;
    }

    return 0;
}

// Function to read the compressed data from the file
size_t readCompressedData(FILE *file, unsigned char *compressedData, size_t dataSize) {
    size_t bytesRead = fread(compressedData, sizeof(unsigned char), dataSize, file);
    if (bytesRead == 0) {
        fprintf(stderr, "Compressed data could not be read.\n");
    }
    return bytesRead;
}

void decodePixels(BitStream *stream, Quadtree *tree) {
    for (long long i = 0; i < tree->treesize; i++) { // Loop through all the nodes in the quadtree.
        Pixnode *current = &tree->Pixels[i];
        Pixnode *parent = NULL;
        if (i > 0) {
            parent = &tree->Pixels[(i - 1) / 4];
        }

        if (i == 0) { // Root node
            pullbits(stream, &current->m, 8);
            unsigned char eRaw = 0;
            pullbits(stream, &eRaw, 2);
            current->e = eRaw;

            if (eRaw == 0) {
                unsigned char uBit = 0;
                pullbits(stream, &uBit, 1);
                current->u = uBit;
            } else {
                current->u = 0;
            }
            continue;
        }

        if (parent->u == 1) { // If the parent is flagged as uniform, then the child inherits the parent's state (m).
            current->m = parent->m; //If so, we don't read anything else.
            current->e = 0;
            current->u = 1;
            continue;
        }

        if (i % 4 == 0) { // 4th child of a node (i = 4, 8, 12, ...)
            if (i < BASE_LAYER(tree)) {
                unsigned char eRaw = 0;
                pullbits(stream, &eRaw, 2);
                current->e = eRaw;

                if (eRaw == 0) {
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

            // Interpolate m for the 4th child with the math formula.
            // m4 = 4*mParent + e - (m1 + m2 + m3)
            // where e is the raw remainder (0-3), NOT the signed conversion
            // Interpolate m for the 4th child: m4 = 4*mP + eP - (m1+m2+m3)
            int mParent = (int)parent->m;
            int eParent = (int)parent->e; 
            
            int m1 = (int)tree->Pixels[i - 1].m;
            int m2 = (int)tree->Pixels[i - 2].m;
            int m3 = (int)tree->Pixels[i - 3].m;
            int mCalc = 4 * mParent + eParent - (m1 + m2 + m3);

            if (mCalc < 0) {
                mCalc = 0;
            }
            if (mCalc > 255) {
                mCalc = 255;
            }

            current->m = (unsigned char)mCalc;
        } else { // For the rest of the child nodes (siblings) we just read the the data.
            pullbits(stream, &current->m, 8);

            if (i < BASE_LAYER(tree)) {
                unsigned char eRaw = 0;
                pullbits(stream, &eRaw, 2);
                current->e = eRaw;

                if (eRaw == 0) {
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


// Main decode function.
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

// Function to fill the pixel matrix from the quadtree.
void fillPixelMatrixFromQuadtree(Quadtree *tree, int **pixelMatrix, int size, int x, int y, long long int nodeIndex, int level) {
    if (nodeIndex < 0 || nodeIndex >= tree->treesize) {
        fprintf(stderr, "Error: Node index (%lld) out of bounds. Tree size: %lld\n", nodeIndex, tree->treesize);
        return;
    }

    Pixnode *node = &tree->Pixels[nodeIndex];

    if (node->u == 1) { // Uniform node, just copy the data.
        int regionSize = size >> level;
        for (int i = 0; i < regionSize; i++) {
            for (int j = 0; j < regionSize; j++) {
                pixelMatrix[y + i][x + j] = node->m;  // y is row, x is column
            }
        }
    } else { // Not a uniform node, recurse into the children, and apply the same logic.
        int halfSize = size >> (level + 1);

        int tlIndex = 4 * nodeIndex + 1;
        int trIndex = 4 * nodeIndex + 2;
        int brIndex = 4 * nodeIndex + 3;
        int blIndex = 4 * nodeIndex + 4;

        if (tlIndex >= tree->treesize || trIndex >= tree->treesize ||
            brIndex >= tree->treesize || blIndex >= tree->treesize) {
            fprintf(stderr, "Error: Child index out of bounds in fillPixelMatrixFromQuadtree.\n");
            return;
        }

        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x, y, tlIndex, level + 1);                          // Top-left: (x, y)
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x + halfSize, y, trIndex, level + 1);              // Top-right: (x + halfSize, y)
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x + halfSize, y + halfSize, brIndex, level + 1);  // Bottom-right: (x + halfSize, y + halfSize)
        fillPixelMatrixFromQuadtree(tree, pixelMatrix, size, x, y + halfSize, blIndex, level + 1);              // Bottom-left: (x, y + halfSize)
    }
}

// Function to write in the PGM file.
void writePGMFile(const char *filename, int **pixelMatrix, int size)
{
    FILE *file = fopen(filename, "wb");  // Binary mode
    if (!file)
    {
        perror("Error when writing on the pgm file.");
        return;
    }

    // Write the header of the pgm file (ASCII format for header)
    fprintf(file, "P5\n");
    fprintf(file, "%d %d\n", size, size);
    fprintf(file, "255\n");

    // Write the pixel matrix to the pgm file in binary format
    // Convert int to unsigned char and write row by row
    unsigned char *row = (unsigned char *)malloc(size * sizeof(unsigned char));
    if (!row)
    {
        perror("Error allocating memory for row buffer");
        fclose(file);
        return;
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            // Clamp values to [0, 255]
            int val = pixelMatrix[i][j];
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            row[j] = (unsigned char)val;
        }
        fwrite(row, sizeof(unsigned char), size, file);
    }

    free(row);
    fclose(file);
}