#include "huffman.h"
#include "heap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define uchar unsigned char
#define MAX(x, y) (x) < (y) ? (y) : (x)

typedef enum {false, true} bool;

typedef struct encoder_tree_node_t {
    int frequency;
    uchar is_leaf;
    union {
        struct {
            struct encoder_tree_node_t* zero; // left child
            struct encoder_tree_node_t* one; // right child
        } childs;
        uchar symbol; // or symbol if it's not leaf
    } data;
} encoder_tree_node;


typedef struct decoder_tree_node_t {
    uchar is_leaf;
    union {
        struct {
            struct decoder_tree_node_t* zero; // left child
            struct decoder_tree_node_t* one; // right child
        } childs;
        uchar symbol; // or symbol if it's not leaf
    } data;
} decoder_tree_node;


typedef struct {
    uchar is_leaf;  
    uchar symbol;
} stored_tree_node;  // node of the tree that's stored in compressed data


typedef struct {
    uchar* byte;
    uchar bit_pos;
} bit_stream;


typedef struct {
    uchar* code;
    uchar bit_length;
} symbol_code;


typedef struct {
    uchar symbol;
    int frequency;
} symbol_frequency;


static encoder_tree_node*
new_encoder_tree_leaf(int frequency, char symbol) {

    encoder_tree_node* leaf = (encoder_tree_node*)
        malloc(sizeof(encoder_tree_node));
    leaf->frequency = frequency;
    leaf->is_leaf = true;
    leaf->data.symbol = symbol; 

    return leaf;
}


static encoder_tree_node*
new_encoder_tree_nonleaf(int frequency, encoder_tree_node* zero,
        encoder_tree_node* one) {

    encoder_tree_node* nonleaf = (encoder_tree_node*)
        malloc(sizeof(encoder_tree_node));
    nonleaf->frequency = frequency;
    nonleaf->is_leaf = false;
    nonleaf->data.childs.zero = zero;
    nonleaf->data.childs.one = one;

    return nonleaf;
}


static int compare_node(const void* a, const void* b) {
    // ascending order (such as we need for heap)
    return ((encoder_tree_node*) a)->frequency -
        ((encoder_tree_node*) b)->frequency;
}


static symbol_code append_bit(symbol_code code, char bit) {
    symbol_code new_code;

    new_code.code = (uchar*) malloc(16);
    memcpy(new_code.code, code.code, 16);
    new_code.bit_length = code.bit_length + 1;

    char offset = code.bit_length % 8;
    char byte_pos = code.bit_length / 8;

    if(bit) {
        new_code.code[byte_pos] |= (1 << offset);
    } else {
        new_code.code[byte_pos] &= ~(1 << offset);
    }

    return new_code;
}


static void copy_code(symbol_code* dest, symbol_code* src) {
    memcpy(dest->code, src->code, 16);
    dest->bit_length = src->bit_length;
}


static int sf_compare_up(const void* a, const void* b) {
    return ((symbol_frequency*) a)->frequency -
        ((symbol_frequency*) b)->frequency;
}

static int sf_compare_down(const void* a, const void* b) {
    return ((symbol_frequency*) b)->frequency -
        ((symbol_frequency*) a)->frequency;
}


static symbol_frequency*
calculate_frequencies(uchar* input, int size, int* outsize) {
    uchar* ptr = input;
    symbol_frequency* frequencies =
        (symbol_frequency*) malloc(256 * sizeof(symbol_frequency));

    *outsize = 0;
    for(int i = 0; i < 256; ++i) {
        frequencies[i].frequency = 0;
        frequencies[i].symbol = (uchar) i;
    }

    for(int i = 0; i < size; ++i) {
        if(frequencies[*ptr].frequency == 0) {
            (*outsize)++;
        }
        frequencies[*ptr++].frequency++;
    }

    // sort by freq down
    qsort(frequencies, 256, sizeof(symbol_frequency), sf_compare_down);
    frequencies = realloc(frequencies, (*outsize) * sizeof(symbol_frequency));
    return frequencies;
}


static encoder_tree_node* 
generate_encoder_tree(symbol_frequency* frequencies, int size) {
    if(size == 1) { // special case where there's only one symbol
        return new_encoder_tree_leaf(frequencies[0].frequency,
                frequencies[0].symbol);
    }

    binary_heap* heap = heap_create(size, compare_node);
    symbol_frequency* ptr = frequencies;

    // for every symbol make node and push into heap
    for(int i = 0; i < size; ++i) {
        encoder_tree_node* node =
            new_encoder_tree_leaf(ptr->frequency, ptr->symbol);
        heap_insert(heap, node);
        ptr++;
    }
    // get 2 nodes from heap and create its parent node 
    // until there's only one element
    while(heap->size > 1) {
        encoder_tree_node* node0 = heap_pop(heap);
        encoder_tree_node* node1 = heap_pop(heap);
        encoder_tree_node* node_parent = new_encoder_tree_nonleaf(
                node1->frequency + node0->frequency, node0, node1);

        heap_insert(heap, node_parent);
    }
    encoder_tree_node* root = heap_pop(heap);
    heap_destroy(heap, NULL);
    return root;
}


static void destroy_encoder_tree(encoder_tree_node* root) {
    if(!root) {
        return;
    } else if(!root->is_leaf) {
        destroy_encoder_tree(root->data.childs.zero);
        destroy_encoder_tree(root->data.childs.one);
    }
    free(root);
}


static symbol_code* build_encoder() {
    symbol_code* encoder = (symbol_code*) malloc(256 * sizeof(symbol_code));
    for(int i = 0; i < 256; ++i) {
        encoder[i].code = (uchar*) calloc(16, 1);
        encoder[i].bit_length = 0;
    }

    return encoder;
}


static void destroy_encoder(symbol_code* encoder) {
    for(int i = 0; i < 256; ++i) {
        free(encoder[i].code);
    }
    free(encoder);
}


static void fill_encoder(encoder_tree_node* root, symbol_code* encoder,
        symbol_code prefix) {
    if(root->is_leaf) {
        copy_code(encoder + root->data.symbol, &prefix);
    } else {
        fill_encoder(root->data.childs.zero, encoder, append_bit(prefix, 0));
        fill_encoder(root->data.childs.one, encoder, append_bit(prefix, 1));
    }
    free(prefix.code);
}


static void write_tree(encoder_tree_node* root, uchar** output, int* size){
    stored_tree_node node;
    node.is_leaf = root->is_leaf;
    node.symbol = root->data.symbol;

    memcpy(*output, &node, sizeof(node));
    (*output) += sizeof(node);
    (*size) += sizeof(node);

    if(!root->is_leaf) {
        write_tree(root->data.childs.zero, output, size);
        write_tree(root->data.childs.one, output, size);
    }
}


static decoder_tree_node* read_decoder_tree(uchar** input, int* leaf_count) {
    stored_tree_node node;
    memcpy(&node, *input, sizeof(node));
    (*input) += sizeof(node);

    decoder_tree_node* root =
        (decoder_tree_node*) malloc(sizeof(decoder_tree_node));

    if((root->is_leaf = node.is_leaf)) {
        root->data.symbol = node.symbol;
        if(leaf_count) {
            (*leaf_count)++;
        }
    } else {
        root->data.childs.zero = read_decoder_tree(input, leaf_count);
        root->data.childs.one = read_decoder_tree(input, leaf_count);
    }
    return root;
}


static void destroy_decoder_tree(decoder_tree_node* root) {
    if(!root->is_leaf) {
        destroy_decoder_tree(root->data.childs.zero);
        destroy_decoder_tree(root->data.childs.one);
    }
    free(root);
}


static void write_code(bit_stream* stream, symbol_code* code) {
    int bits_left = code->bit_length, offset = 0;
    uchar* code_ptr = code->code;
    while(bits_left) {
        short code_bytes = (*((short*) code_ptr) << stream->bit_pos) >> offset;
        *((short*) stream->byte) |= code_bytes; // write 2 bytes of code

        if(bits_left < 16 - stream->bit_pos) { // if rest of code was written
            stream->byte += ((stream->bit_pos + bits_left) > 7);
            stream->bit_pos = (stream->bit_pos + bits_left) % 8;
            bits_left = 0;
        } else {
            // how much bits were written
            // to the first byte of stream
            offset = (8 - stream->bit_pos) * (stream->bit_pos != 0);
            // move 1 or 2 bytes forward
            code_ptr += 1 + (stream->bit_pos == 0); 
            stream->byte += 2;
            bits_left -= (16 - stream->bit_pos);
            stream->bit_pos = 0;
        }
    }
}


static int
encode_data(uchar* input, uchar* output, int insize, symbol_code* encoder) {
    bit_stream* stream = (bit_stream*) malloc(sizeof(bit_stream));
    stream->byte = output;
    stream->bit_pos = 0;

    for(int i = 0; i < insize; ++i, ++input) {
        write_code(stream, encoder + (*input));
    }

    int outsize = stream->byte - output + (stream->bit_pos > 0);

    free(stream);
    return outsize;
}


uchar* huffman_compress(uchar* input, int insize, int* outsize) {
    if(!input) {
        return NULL;
    }
    if(!outsize) {
        return NULL;
    }
    if(insize <= 0) {
        return NULL;
    }

    uchar* output = (uchar*) calloc(insize + 1024, 1);
    uchar* ptr = output;
    int sym_count;
    *outsize = 0;

    // calculate symbol frequencies
    symbol_frequency* freqs = calculate_frequencies(input, insize, &sym_count);
    // encoder tree
    encoder_tree_node* tree = generate_encoder_tree(freqs, sym_count);
    free(freqs);

    memcpy(ptr, &insize, 4); // store input data size 
    ptr += 4;
    *outsize += 4;

    write_tree(tree, &ptr, outsize); // store tree and add its size to outsize

    if(sym_count > 1) { // if it's == 0 -> do nothing
        // bit prefix for recursive tree traverse 
        symbol_code prefix;
        prefix.bit_length = 0;
        prefix.code = (uchar*) calloc(16, 1);

        // encoder is the indexed dictionary of symbol codes
        symbol_code* encoder = build_encoder();
        fill_encoder(tree, encoder, prefix);

        *outsize += encode_data(input, ptr, insize, encoder); // encode data
        destroy_encoder(encoder);
    }
    destroy_encoder_tree(tree);
    output = realloc(output, *outsize);
    return output;
}

static void
decode_data(uchar* input, uchar* output, int outsize, decoder_tree_node* dtree){
    decoder_tree_node* root = dtree; // we will go down the tree until a leaf
    uchar bit_mask = 1;
    int data_read = 0;

    while(data_read != outsize) {
        if(root->is_leaf) { // if leaf is reached
            *(output) = root->data.symbol; // write symbol
            ++output;
            data_read++;
            root = dtree;
        } else {
            if(bit_mask & (*input)) {
                root = root->data.childs.one; // right child
            } else {
                root = root->data.childs.zero; // left child
            }

            if(!(bit_mask <<= 1)) { // bit shift in loop
                ++input;
                bit_mask = 1;
            }
        }
    }
}

uchar* huffman_decompress(uchar* input) {
    if(input == NULL) {
        return NULL;
    }

    uchar* ptr = input;
    int outsize = ((int*) ptr)[0]; // extract size of decompressed data
    if(outsize <= 0) {
        return NULL;
    }
    ptr += 4;

    uchar* output = (uchar*) malloc(outsize);
    int leaf_count = 0;
    decoder_tree_node* dtree = read_decoder_tree(&ptr, &leaf_count); // decoder

    if(leaf_count > 1) {
        decode_data(ptr, output, outsize, dtree);
    } else { // special case when there's only one symbol appears in data
        memset(output, dtree->data.symbol, outsize);
    }

    destroy_decoder_tree(dtree);
    return output;
}
