#include "quad.h"
#include "decode.h"
#include <stdbool.h>

void parameters(int argc, char *argv[], bool *encode, bool *decode, char **inputFile, char **outputFile, bool *grid, bool *help, bool *verbose) {
    *encode = false;
    *decode = false;
    *grid = false;
    *help = false;
    *verbose = false;
    *inputFile = NULL;
    *outputFile = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                    *encode = true;
                    break;
                case 'u':
                    *decode = true;
                    break;
                case 'i':
                    if (i + 1 < argc) {
                        *inputFile = argv[++i];
                    } else {
                        fprintf(stderr, "Error: Missing input file after -i\n");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'o':
                    if (i + 1 < argc) {
                        *outputFile = argv[++i];
                    } else {
                        fprintf(stderr, "Error: Missing output file after -o\n");
                        exit(EXIT_FAILURE);
                    }
                    break;
                case 'g':
                    *grid = true;
                    break;
                case 'h':
                    *help = true;
                    break;
                case 'v':
                    *verbose = true;
                    break;
                default:
                    fprintf(stderr, "Error: Unknown option -%c\n", argv[i][1]);
                    exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Error: Invalid argument format: %s\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }

    if (!*inputFile) {
        fprintf(stderr, "Error: Input file not specified\n");
        exit(EXIT_FAILURE);
    }

    if (!*outputFile) {
        *outputFile = *encode ? "out.qtc" : "out.pgm";
    }
}

int main(int argc, char *argv[]) {
    bool encode, decodeVar, grid, help, verbose;
    char *inputFile, *outputFile;

    // Analizar parámetros
    parameters(argc, argv, &encode, &decodeVar, &inputFile, &outputFile, &grid, &help, &verbose);

    if (help) {
        printf("Usage: codec [options]\n");
        printf("  -c          Encode an input PGM file to QTC\n");
        printf("  -u          Decode an input QTC file to PGM\n");
        printf("  -i <file>   Specify input file\n");
        printf("  -o <file>   Specify output file\n");
        printf("  -g          Generate grid visualization (optional)\n");
        printf("  -h          Show this help message\n");
        printf("  -v          Verbose mode\n");
        return 0;
    }

    if (encode && decodeVar) {
        fprintf(stderr, "Error: Cannot encode and decode simultaneously\n");
        return EXIT_FAILURE;
    }

    if (encode) {
        if (verbose) printf("Encoding %s to %s\n", inputFile, outputFile);
        // Lógica del codificador
        int width, height, maxGray;
        unsigned char *pixmap = readPGM(inputFile, &width, &height, &maxGray);
        if (!pixmap) {
            fprintf(stderr, "Error reading PGM file\n");
            return EXIT_FAILURE;
        }

        int levels = log2(width) + 1;
        Quadtree *tree = initializeQuadtree(levels);
        if (!tree) {
            free(pixmap);
            fprintf(stderr, "Error initializing Quadtree\n");
            return EXIT_FAILURE;
        }

        encodePixmapToQuadtreeAscending(pixmap, width, tree);
        writeQuadtreeToQTC(outputFile, tree, "Q1", width, height, levels);

        free(tree->Pixels);
        free(tree);
        free(pixmap);
    } else if (decodeVar) {
        if (verbose) printf("Decoding %s to %s\n", inputFile, outputFile);
        // Lógica del decodificador
        decode(inputFile, outputFile);
    } else {
        fprintf(stderr, "Error: No operation specified. Use -c to encode or -u to decode\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
