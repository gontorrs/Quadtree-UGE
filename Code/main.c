#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void parameters(int argc, char *argv[])
{
    bool encode = false;
    bool decode = false;
    bool input = false;
    bool out = false;
    bool grid = false;
    bool help = false;
    bool verbose = false;
    char *outputfile;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != '\0'; j++)
            {
                switch (argv[i][j])
                {
                case 'c':
                    encode = true;
                    break;
                case 'u':
                    decode = true;
                    break;
                case 'i':
                    input = true;
                    break;
                case 'o':
                    out = true;
                    outputfile = malloc(strlen(argv[i + 1]) + 1);
                    strcpy(outputfile, argv[i + 1]);
                    i++;
                    break;

                case 'g':
                    grid = true;
                    break;
                case 'h':
                    help = true;
                    break;
                case 'v':
                    verbose = true;
                    break;
                default:
                    break;
                }
            }
        }
    }
    if (outputfile != NULL) {
        FILE *file = fopen(outputfile, "w");

        if (file == NULL) {
            printf("Error: Could not open the output file for writing\n");
            free(outputfile);
            return;
        }
        fclose(file);
        free(outputfile);
    } else {
        printf("No output file specified.\n");
    }
    
}

int main(int argc, char *argv[])
{
    parameters(argc, argv);

    return 0;
}