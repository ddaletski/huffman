# Libhuffman - huffman compression/decompression library

## Features
- Fast
- Fixed-size result (5 bytes) for a special case where the input buffer is full of some constant byte

## API

1) `uchar* huffman_compress (uchar *input, uint insize, uint* outsize)` - compress the data, using huffman codes
- `input` - input data
- `insize` - size of input data
- `outsize` - pointer to size of output data
- `returns:` `uchar*` - pointer to memory where the compressed data is stored. Memory is allocated automatically (`*outsize` bytes). The uncompressed (original) data size is stored in first 4 bytes of the output buffer

2) `uchar* huffman_decompress (uchar *input)` - decompress the data, using huffman codes
- `input` - input data (compressed)
- `returns:` *uchar* - pointer to memory where decompressed data is stored. Memory is allocated automatically
