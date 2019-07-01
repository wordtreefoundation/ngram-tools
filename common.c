#include <stdio.h>
#include <stdlib.h>

FILE *open_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "  failed to open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    return file;
}
