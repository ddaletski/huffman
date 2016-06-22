#include <check.h>
#include "../heap.c"
#include "../huffman.c"
#include <stdlib.h>
#include <stdio.h>

#define uchar unsigned char
#define uint unsigned int

int frequency(char chr) {
    switch(chr) {
        case 'a':
            return 4;
        case 'b':
            return 6;
        case 'c':
            return 3;
        case 'd':
            return 2;
        case 'e':
            return 1;
        case 'f':
            return 1;
    }
}

START_TEST(test_frequencies) {
    uchar* input = "abbabbabbacccddef";
    int sym_count;
    symbol_frequency* freqs = calculate_frequencies(input,
            strlen(input), &sym_count);

    for(int i = 0; i < sym_count; ++i) {
        ck_assert_int_eq(freqs[i].frequency, frequency(freqs[i].symbol));
    }
} END_TEST

// safety of original data size
START_TEST(test_original_size) {
    uchar* input = "adasdaskjdasnk;dfjhbaslkerfgbas";
    int comp_size, decomp_size;
    uchar* output = huffman_compress(input, strlen(input), &comp_size);
    decomp_size = ((int*)output)[0];

    ck_assert_msg(strlen(input) == decomp_size,
            "original data size is stored incorrectly:\ngot %d\nshould be %d",
            decomp_size, strlen(input));
    free(output);
} END_TEST


// correctness of writing the symbol code to bit_stream
START_TEST(test_code_write) {
    int offset = 0;
    long int original_number = (long int) rand() % (1 << 26);

    symbol_code* code = (symbol_code*) malloc(sizeof(symbol_code));
    code->code = (uchar*) calloc(16, 1);
    int bufsize = 20;

    uchar* buf = (uchar*) calloc(bufsize, 1);
    bit_stream* stream = (bit_stream*) malloc(sizeof(bit_stream));
    stream->byte = buf;
    stream->bit_pos = offset;

    ((long int*) code->code)[0] = original_number;

    uchar* ptr = stream->byte;
    write_code(stream, code);

    long int written_number = ((long int*) buf)[0] >> offset;

    ck_assert_msg(written_number == original_number,
            "symbol code has been written incorrectly");
    free(code->code);
    free(stream);
    free(buf);
} END_TEST


int trees_equal(encoder_tree_node* t_enc, decoder_tree_node* t_dec) {
    int result = 1;
    if(!t_enc->is_leaf) {
        result &= trees_equal(t_enc->data.childs.zero, t_dec->data.childs.zero);
        result &= trees_equal(t_enc->data.childs.one, t_dec->data.childs.one);
    } else {
        return result = t_enc->data.symbol == t_dec->data.symbol;
    }
    return result;
}


// equivalence of encoder and decoder trees
START_TEST(test_trees_equal) {
    int sym_count, outsize = 0, insize;
    uchar* input = "aaabbccaszxodchnas;oskdhasifgd";
    insize = strlen(input);

    symbol_frequency* freqs = calculate_frequencies(input, insize, &sym_count);
    encoder_tree_node* tree = generate_encoder_tree(freqs, sym_count);
    free(freqs);

    // bit prefix for recursive tree traverse 
    symbol_code prefix;
    prefix.code = (uchar*) malloc(16);
    prefix.bit_length = 0;
    memset(prefix.code, 0, 16);
    free(prefix.code);

    uchar* buf = (uchar*) malloc(512);
    uchar* ptr = buf;
    write_tree(tree, &ptr, &outsize); 

    ptr = buf;
    int dtree_size;
    decoder_tree_node* dtree = read_decoder_tree(&ptr, NULL);

    ck_assert_msg(trees_equal(tree, dtree) == 1, "encoder and decoder trees are not equal");

    destroy_encoder_tree(tree);
    destroy_decoder_tree(dtree);
    free(buf);
} END_TEST


// equivalence of original and decompressed data
START_TEST(test_compress_decompress) {
    uchar* input = "abcdeaaabccsaderasdadzxcvmc";
    int size = strlen(input), comp_size;
    uchar* output = huffman_compress(input, size, &comp_size);
    uchar* decompressed = huffman_decompress(output);

    int result = 1;
    for(int j = 0; j < size; ++j) 
        if(decompressed[j] != input[j])
            result = 0;

    ck_assert_msg(result == 1,
            "original data recovered incorrectly");

    free(output);
    free(decompressed);
} END_TEST


// correctnes of big data compression/decompression
START_TEST(test_big_data) {
    printf("\nbig sample test\n");
    int size = 100000, j = 0, power = 3, comp_size, decomp_size;
    uchar* input = (uchar*) malloc(size);
    srand(time(0));
    int subsize = size/2, subsubsize = size / 4;
    char value = 0;
    for(int k = 0; k < power; ++k) {
        for(; j < subsize; ++j)
            input[j] = value;
        value++;
        subsize += subsubsize;
        subsubsize /= 2;
    }
    for(; j < size; ++j) 
        input[j] = rand() % 256;
    uchar* output = huffman_compress(input, size, &comp_size);
    printf("compressed size = %d\n", comp_size);
    decomp_size = ((int*) output)[0];
    uchar* decompressed = huffman_decompress(output);
    printf("decompressed size = %d\n", decomp_size);

    int error = 0;
    for(int j = 0; j < size; ++j) {
        if(decompressed[j] != input[j]) {
            error = 1;
            break;
        }
    }

    ck_assert_msg(error == 0,
            "original data recovered incorrectly");

    free(output);
    free(decompressed);
} END_TEST

// correctnes of one symbol (many times) compression
START_TEST(test_one_symbol) {
    int size = 1000000;
    uchar* input = (uchar*) malloc(size);
    for(int j = 0; j < size; ++j) 
        input[j] = 12;

    int comp_size, decomp_size;

    printf("\none symbol case\n");
    uchar* output = huffman_compress(input, size, &comp_size);
    printf("compressed size = %d\n", comp_size);
    decomp_size = ((int*) output)[0];
    uchar* decompressed = huffman_decompress(output);
    printf("decompressed size = %d\n", decomp_size);

    int error = 0;
    for(int j = 0; j < size; ++j) {
        if(decompressed[j] != input[j]) {
            error = 1;
            break;
        }
    }

    ck_assert_msg(error == 0,
            "original data recovered incorrectly");

    free(output);
    free(decompressed);
} END_TEST


// NULL data compression test
START_TEST(test_compress_null) {
    uchar* input = NULL;
    int size = 100, comp_size;
    uchar* output = huffman_compress(input, size, &comp_size);
    ck_assert_int_eq(output, NULL);
} END_TEST


// null data decompression test
START_TEST(test_decompress_null) {
    uchar* input = "asdhakdha;skdh23048903kopjm54638746";
    int size = strlen(input), comp_size, decomp_size;
    uchar* output = huffman_compress(input, size, &comp_size);

    uchar* decompressed = huffman_decompress(NULL);
    ck_assert_int_eq(decompressed, NULL);

    free(output);
} END_TEST


// zero size compression test
START_TEST(test_zero_size) {
    uchar* input = "asdhakdha;skdh23048903kopjm54638746";
    int size = 0, comp_size;
    uchar* output = huffman_compress(input, size, &comp_size);
    ck_assert_int_eq(output, NULL);
} END_TEST


// bad pointer to input size test
START_TEST(test_bad_insize_pointer) {
    uchar* input = "asdhakdha;skdh23048903kopjm54638746";
    int size = strlen(input), *comp_size = NULL;
    uchar* output = huffman_compress(input, size, comp_size);
    ck_assert_int_eq(output, NULL);

    free(output);
} END_TEST




Suite * huffman_suite(void)
{
    Suite *s;
    TCase *tc_core;
s = suite_create("Huffman");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_set_timeout(tc_core, 0);
    tcase_add_test(tc_core, test_original_size);
    tcase_add_test(tc_core, test_code_write);
    tcase_add_test(tc_core, test_trees_equal);
    tcase_add_test(tc_core, test_compress_decompress);
    tcase_add_test(tc_core, test_frequencies);
    tcase_add_test(tc_core, test_big_data);
    tcase_add_test(tc_core, test_one_symbol);
    tcase_add_test(tc_core, test_compress_null);
    tcase_add_test(tc_core, test_decompress_null);
    tcase_add_test(tc_core, test_zero_size);
    tcase_add_test(tc_core, test_bad_insize_pointer);

    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = huffman_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
