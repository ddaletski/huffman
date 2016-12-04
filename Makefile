.PHONY: all test clean install uninstall

all: libhuffman.so

test: 
	@cd tests && make && cd -
clean:
	rm *.o *.so
	cd tests && make clean && cd -

install:
	cp libhuffman.so /usr/local/lib/ 
	cp huffman.h /usr/local/include/

uninstall:
	rm /usr/local/lib/libhuffman.so
	rm /usr/local/include/huffman.h

libhuffman.so: huffman.o
	$(CC) -std=c99 -shared -o $@ $<

huffman.o: huffman.c heap.c
	$(CC) -std=c99 -c -o $@ $< -fPIC
