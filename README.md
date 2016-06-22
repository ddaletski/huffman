# Libhuffman - huffman compression/decompression library
## Functions

1) `uchar* huffman_compress (uchar *input, uint insize, uint* outsize)` - compress the data, using huffman codes
- **input** - input data
- **insize** - size of input data
- **outsize** - pointer to size of output data
- **returns:** *uchar* - pointer to memory where the compressed data is stored. Memory is allocated automatically, size = **insize**. First 4 bytes store size of original data

2) `uchar* huffman_decompress (uchar *input)` - decompress the data, using huffman codes
- **input** - input data (compressed)
- **returns:** *uchar* - pointer to memory where decompressed data is stored. Memory is allocated automatically
