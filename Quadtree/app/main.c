#include "quad.h"
#include "filter.h"
typedef struct
{
    bool encode;
    bool decode;
    bool grid;
    bool help;
    bool verbose;
    double alpha;
    const char *inputFile;
    const char *outputFile;
} CodecOptions;

void initCodecOptions(CodecOptions *opts)
{
    opts->encode = false;
    opts->decode = false;
    opts->inputFile = NULL;
    opts->outputFile = NULL;
    opts->alpha = 0.0;
    opts->grid = false;
    opts->verbose = false;
    opts->help = false;
}

void printHelp()
{
    printf("Usage:\n");
    printf("  codec -c -i input.pgm [-o output.qtc] [-a alpha] [-g]\n");
    printf("  codec -u -i input.qtc [-o output.pgm] [-g]\n");
    printf("Options:\n");
    printf("  -c            Run as encoder\n");
    printf("  -u            Run as decoder\n");
    printf("  -i <file>     Specify input file\n");
    printf("  -o <file>     Specify output file\n");
    printf("  -a <alpha>    Set compression loss factor (only for encoder)\n");
    printf("  -g            Output segmentation grid\n");
    printf("  -v            Verbose output\n");
    printf("  -h            Show help\n");
}

void parseArguments(int argc, char *argv[], CodecOptions *opts)
{
    initCodecOptions(opts);
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            opts->encode = true;
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
            opts->decode = true;
        }
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
        {
            opts->inputFile = argv[++i];
        }
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
        {
            opts->outputFile = argv[++i];
        }
        else if (strcmp(argv[i], "-a") == 0 && i + 1 < argc)
        {
            opts->alpha = atof(argv[++i]);
        }
        else if (strcmp(argv[i], "-g") == 0)
        {
            opts->grid = true;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            opts->verbose = true;
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            opts->help = true;
        }
        else
        {
            fprintf(stderr, "Error: Unknown or improperly used option '%s'\n", argv[i]);
            printHelp();
            exit(EXIT_FAILURE);
        }
    }

    if (!opts->inputFile)
    {
        fprintf(stderr, "Error: Input file not specified\n");
        printHelp();
        exit(EXIT_FAILURE);
    }

    if (!opts->outputFile)
    {
        if (opts->encode)
        {
            opts->outputFile = "QTC/out.qtc";
        }
        else if (opts->decode)
        {
            opts->outputFile = "PGM/out.pgm";
        }
    }

    if (opts->encode && opts->decode)
    {
        fprintf(stderr, "Error: Cannot specify both encode and decode\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    CodecOptions options = {0};
    parseArguments(argc, argv, &options);

    if (options.help)
    {
        printHelp();
        return 0;
    }
    if (options.encode)
    {
    // Encode process.
    int width, height, maxGray;
    unsigned char *pixmap = readPGM(options.inputFile, &width, &height, &maxGray, options.verbose);
        // Lossless encoding.
        if (!pixmap)
        {
            return -1;
        }

        int levels = log2(width) + 1;
        Quadtree *encodeTree = initializeQuadtree(levels);
        if (!encodeTree)
        {
            free(pixmap);
            return -1;
        }

        encodePixmapToQuadtreeAscending(pixmap, width, encodeTree);
        if (options.verbose)
        {
            printf("Tree size: %lld\n", encodeTree->treesize);
            printf("Tree levels: %d\n", encodeTree->levels);
            printQuadtree(encodeTree, 0, 0);
        }
        if (options.alpha != 0.0)
        {
            filtrage(encodeTree,0,0, calculateSigmaStart(encodeTree), options.alpha);
            printf("Encoding with the lossy version: alpha = %.1f\n", options.alpha);

        }
        writeQuadtreeToQTC(options.outputFile, encodeTree, "Q1", width, height, levels, options.verbose);

        free(pixmap);
        free(encodeTree->Pixels);
        free(encodeTree);
    
    }

    else if (options.decode)
    {
        // Decode process.
        Quadtree *decodeTree = malloc(sizeof(Quadtree));
        if (!decodeTree)
        {
            fprintf(stderr, "Error: Memory could not be assignied to quadtree.\n");
            return -1;
        }

        decodeQTCtoQuadtree(options.inputFile, decodeTree);
        if (options.verbose)
        {
            printf("Tree size: %lld\n", decodeTree->treesize);
            printf("Tree levels: %d\n", decodeTree->levels);
            printQuadtree(decodeTree, 0, 0);
        }

        int size = 1 << (decodeTree->levels - 1);
        int **pixelMatrix = (int **)malloc(size * sizeof(int *));
        for (int i = 0; i < size; i++)
        {
            pixelMatrix[i] = (int *)malloc(size * sizeof(int));
            memset(pixelMatrix[i], 0, size * sizeof(int));
        }

        fillPixelMatrixFromQuadtree(decodeTree, pixelMatrix, size, 0, 0, 0, 0);
        writePGMFile(options.outputFile, pixelMatrix, size);

        for (int i = 0; i < size; i++)
        {
            free(pixelMatrix[i]);
        }
        free(pixelMatrix);
        free(decodeTree->Pixels);
        free(decodeTree);
    }

    else
    {
        printf("No option was selected to decode or encode\n");
    }

    return 0;
}
