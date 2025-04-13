#include "asm.h"
int main(int argc, char **argv)
{
    if(argc < 2) {
        fprintf(stderr, "File expected.\nUsage: [8086asm <file.asm>]");
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
        AssembleFile(argv[i]);
        printf("\n");
    }

    AFree();
    return EXIT_SUCCESS;
}
