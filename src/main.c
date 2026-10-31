#include "lzma.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s compress   <input> <output>\n", prog);
    printf("  %s decompress <input> <output>\n", prog);
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        print_usage(argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *in_path = argv[2];
    const char *out_path = argv[3];

    FILE *in = fopen(in_path, "rb");
    if (!in)
    {
        printf("Error: cannot open input file '%s'\n", in_path);
        return 1;
    }

    if (strcmp(mode, "compress") == 0)
    {
        fseek(in, 0, SEEK_END);
        uint64_t original_size = ftell(in);
        fseek(in, 0, SEEK_SET);

        FILE *out = fopen(out_path, "wb");
        if (!out)
        {
            printf("Error: cannot open output file '%s'\n", out_path);
            fclose(in);
            return 1;
        }

        fwrite(&original_size, sizeof(original_size), 1, out);

        int res = lzma_encode_stream(in, out);
        fclose(in);
        fclose(out);

        if (res == LZMA_OK)
        {
            printf("Compressed: %s -> %s\n", in_path, out_path);
            return 0;
        }
        else
        {
            printf("Compression failed (code %d)\n", res);
            return 1;
        }
    }
    else if (strcmp(mode, "decompress") == 0)
    {
        uint64_t original_size = 0;
        if (fread(&original_size, sizeof(original_size), 1, in) != 1)
        {
            printf("Error: cannot read header from '%s'\n", in_path);
            fclose(in);
            return 1;
        }

        FILE *out = fopen(out_path, "wb");
        if (!out)
        {
            printf("Error: cannot open output file '%s'\n", out_path);
            fclose(in);
            return 1;
        }

        int res = lzma_decode_stream(in, out, original_size);
        fclose(in);
        fclose(out);

        if (res == LZMA_OK)
        {
            printf("Decompressed: %s -> %s\n", in_path, out_path);
            return 0;
        }
        else
        {
            printf("Decompression failed (code %d)\n", res);
            return 1;
        }
    }
    else
    {
        printf("Error: unknown mode '%s'\n", mode);
        print_usage(argv[0]);
        fclose(in);
        return 1;
    }
}
