test_build_opts=-std=c99 -lcheck_pic -pthread -lrt -lm -lsubunit


test_all: heap_tests.t huffman_tests.t
	./heap_tests.t
	./huffman_tests.t

heap_tests.t: heap_tests.c
	${CC} $< -o $@ ${test_build_opts}

huffman_tests.t: huffman_tests.c
	${CC} $< -o $@ ${test_build_opts}

clean:
	rm -f *.t
