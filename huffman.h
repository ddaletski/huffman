#ifndef HUFFMAN_H
#define HUFFMAN_H

typedef unsigned int uint;
typedef unsigned char uchar;

uchar* huffman_compress(uchar* input, int insize, int* outsize);
uchar* huffman_decompress(uchar* input);

#endif
