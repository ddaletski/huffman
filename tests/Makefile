test_build_opts=-pthread -lcheck_pic -pthread -lrt -lm

test_all: heap_tests.t huffman_tests.t
	./heap_tests.t
	./huffman_tests.t

heap_tests.t: heap_tests.c
	${CC} $< -o $@ ${test_build_opts} `pkg-config --libs check`

huffman_tests.t: huffman_tests.c
	${CC} $< -o $@ ${test_build_opts} `pkg-config --libs check`

clean:
	rm -f *.t
