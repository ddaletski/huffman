#include "huffman.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define uchar unsigned char

char compress_file(const char* infile_name, const char* outfile_name);
char decompress_file(const char* infile_name, const char* outfile_name);

int main(int argc, char* argv[]) {
    char operation = 0, success = 0;
    char* infile_name, *outfile_name;
    if(argc < 2) {
        printf("usage: ./huff [-c|-d] infile_name [outfile_name]\n");
        return 1;
    }
    switch(argc) {
        case 2:
            printf("no infile_name specified\n");
        case 3:
            printf("no outfile_name specified\n");
            return 1;
        default:
            if(strcmp(argv[1], "-c") == 0)
                operation = 1;
            else if(strcmp(argv[1], "-d") == 0)
                operation = 2;
            else {
                printf("bad option: %s\n", argv[1]);
                printf("usage: ./huff [-c|-d] infile_name [outfile_name]\n");
                return 1;
            }
    }

    infile_name = (char*) malloc(strlen(argv[2]));
    strcpy(infile_name, argv[2]);
    outfile_name = (char*) malloc(strlen(argv[3]));
    strcpy(outfile_name, argv[3]);

    switch(operation) {
    case 1:
        success = compress_file(infile_name, outfile_name);
        break;
    case 2:
        success = decompress_file(infile_name, outfile_name);
        break;
    }
    return 0;
}

char compress_file(const char* infile_name, const char* outfile_name) {
    FILE* infile = fopen(infile_name, "r");
    if(!infile) {
        printf("infile not found\n");
        return 0;
    }
    fseek(infile, 0, SEEK_END);
    int insize = ftell(infile), outsize = 0;

    if(insize == 0) {
        printf("input file is empty\n");
        return 0;
    }

    fclose(infile);
    infile = fopen(infile_name, "r");

    uchar* input = (uchar*) malloc(insize);
    if(fread(input, 1, insize, infile) != insize) {
        printf("input file reading failure\n");
        return 0;
    }

    uchar* output = huffman_compress(input, insize, &outsize);

    if(!output) {
        printf("compression error\n");
        return 0;
    }

    free(input);
    fclose(infile);

    FILE* outfile = fopen(outfile_name, "w");

    if(fwrite(output, 1, outsize, outfile) != outsize) {
        printf("output file writting failure\n");
        return 0;
    }

    free(output);
    fclose(outfile);
    return 1;
}

char decompress_file(const char* infile_name, const char* outfile_name) {
    FILE* infile = fopen(infile_name, "r");
    if(!infile) {
        printf("infile not found\n");
        return 0;
    }
    fseek(infile, 0, SEEK_END);
    int insize = ftell(infile), outsize = 0;

    if(insize == 0) {
        printf("input file is empty\n");
        return 0;
    }

    fclose(infile);
    infile = fopen(infile_name, "r");

    if(insize < 6) {
        printf("wrong compressed data format\n");
        return 0;
    }

    uchar* input = (uchar*) malloc(insize);
    if(fread(input, 1, insize, infile) != insize) {
        printf("input file reading failure\n");
        return 0;
    }

    outsize = *((int*) input);

    uchar* output = huffman_decompress(input);

    if(!output) {
        printf("decompression error\n");
        return 0;
    }

    free(input);
    fclose(infile);

    FILE* outfile = fopen(outfile_name, "w");

    if(fwrite(output, 1, outsize, outfile) != outsize) {
        printf("output file writting failure\n");
        return 0;
    }

    free(output);
    fclose(outfile);
    return 1;

    return 0;
}

