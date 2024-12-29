#include "decode.h"

long long calculateTreeSizeforDecod(int levels)
{
    long long treesize = 0;
    for (int i = 0; i <= levels; i++)
    {
        treesize += (long long)pow(4, i);
        if (treesize < 0)
        { // Manejo de desbordamiento
            fprintf(stderr, "Error: Desbordamiento en el cálculo del tamaño del Quadtree\n");
            return -1;
        }
    }
    return treesize;
}

int readIntPortable(FILE *file)
{
    unsigned char bytes[4];
    if (fread(bytes, sizeof(unsigned char), 4, file) != 4)
    {
        fprintf(stderr, "Error leyendo entero\n");
        return -1;
    }
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

Quadtree *readQuadtreeFromQTC(const char *filename, int *width, int *height, int *levels)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Error al abrir el archivo .qtc");
        return NULL;
    }

    // Leer identificador
    char id[3];
    if (fread(id, sizeof(char), 2, file) != 2)
    {
        fprintf(stderr, "Error leyendo el identificador del archivo .qtc\n");
        fclose(file);
        return NULL;
    }
    id[2] = '\0';
    printf("Identificador leído: %s\n", id);
    if (strcmp(id, "Q1") != 0)
    {
        fprintf(stderr, "Identificador inválido: %s\n", id);
        fclose(file);
        return NULL;
    }

    // Saltar líneas de comentarios
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] != '#')
        {
            break;
        }
    }

    // Leer ancho, alto y niveles
    *width = readIntPortable(file);
    *height = readIntPortable(file);
    *levels = readIntPortable(file);

    printf("Dimensiones leídas: ancho=%d, alto=%d, niveles=%d\n", *width, *height, *levels);

    if (*width <= 0 || *height <= 0 || *levels <= 0)
    {
        fprintf(stderr, "Dimensiones o niveles inválidos en el archivo .qtc\n");
        fclose(file);
        return NULL;
    }

    // Calcular tamaño del Quadtree
    long long treesize = calculateTreeSizeforDecod(*levels);
    if (treesize <= 0)
    {
        fclose(file);
        return NULL;
    }
    printf("Tamaño del árbol: %lld\n", treesize);

    // Inicializar Quadtree
    Quadtree *tree = (Quadtree *)malloc(sizeof(Quadtree));
    if (!tree)
    {
        fprintf(stderr, "Error al asignar memoria para el Quadtree\n");
        fclose(file);
        return NULL;
    }

    tree->treesize = treesize;
    tree->levels = *levels;
    tree->Pixels = (Pixnode *)malloc(treesize * sizeof(Pixnode));
    if (!tree->Pixels)
    {
        fprintf(stderr, "Error al asignar memoria para los nodos del Quadtree\n");
        free(tree);
        fclose(file);
        return NULL;
    }

    // Leer nodos del Quadtree
    for (int i = 0; i < treesize; i++)
    {
        unsigned char temp;
        if (fread(&tree->Pixels[i].m, sizeof(unsigned char), 1, file) != 1 ||
            fread(&temp, sizeof(unsigned char), 1, file) != 1)
        {
            fprintf(stderr, "Error leyendo nodos del archivo en posición %d\n", i);
            free(tree->Pixels);
            free(tree);
            fclose(file);
            return NULL;
        }
        tree->Pixels[i].e = temp & 0b11;
        tree->Pixels[i].u = (temp >> 2) & 0b1;
    }

    fclose(file);
    return tree;
}

void quadtreeToPixmap(Quadtree *tree, unsigned char *pixmap, int width, int nodeIndex, int x, int y, int size)
{
    if (nodeIndex < 0 || nodeIndex >= tree->treesize)
    {
        fprintf(stderr, "Error: Índice de nodo fuera de rango (nodeIndex: %d, tree->treesize: %lld)\n", nodeIndex, tree->treesize);
        return;
    }

    if (size <= 0)
    {
        fprintf(stderr, "Error: Tamaño inválido en quadtreeToPixmap (size: %d)\n", size);
        return;
    }

    if (size == 1)
    {
        pixmap[y * width + x] = tree->Pixels[nodeIndex].m;
        return;
    }

    int halfSize = size / 2;

    // Índices de los hijos
    int tlIndex = 4 * nodeIndex + 1;
    int trIndex = tlIndex + 1;
    int brIndex = tlIndex + 2;
    int blIndex = tlIndex + 3;

    // Asegurarse de que los índices están dentro de los límites antes de llamar recursivamente
    if (tlIndex < tree->treesize)
        quadtreeToPixmap(tree, pixmap, width, tlIndex, x, y, halfSize);
    if (trIndex < tree->treesize)
        quadtreeToPixmap(tree, pixmap, width, trIndex, x + halfSize, y, halfSize);
    if (brIndex < tree->treesize)
        quadtreeToPixmap(tree, pixmap, width, brIndex, x + halfSize, y + halfSize, halfSize);
    if (blIndex < tree->treesize)
        quadtreeToPixmap(tree, pixmap, width, blIndex, x, y + halfSize, halfSize);
}

void writePGM(const char *filename, unsigned char *pixmap, int width, int height)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Error opening .pgm file for writing");
        return;
    }

    // Escribir encabezado
    fprintf(file, "P5\n%d %d\n255\n", width, height);

    // Escribir datos de pixeles
    fwrite(pixmap, sizeof(unsigned char), width * height, file);

    fclose(file);
}

void decode(const char *inputFile, const char *outputFile)
{
    int width, height, levels;

    // Leer el archivo .qtc
    Quadtree *tree = readQuadtreeFromQTC(inputFile, &width, &height, &levels);
    if (!tree)
    {
        fprintf(stderr, "Error: Failed to read the .qtc file\n");
        return;
    }

    // Reconstruir pixmap
    unsigned char *pixmap = (unsigned char *)malloc(width * height);
    if (!pixmap)
    {
        fprintf(stderr, "Error al asignar memoria para el pixmap\n");
        free(tree->Pixels);
        free(tree);
        return;
    }
    quadtreeToPixmap(tree, pixmap, width, 0, 0, 0, width);

    // Escribir pixmap a archivo PGM
    writePGM(outputFile, pixmap, width, height);

    // Liberar memoria
    free(tree->Pixels);
    free(tree);
    free(pixmap);
}
