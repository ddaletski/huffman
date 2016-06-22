.PHONY: all tests clean install uninstall

all: libhuffman.so

tests: 
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
	$(CC) -shared -o $@ $<

huffman.o: huffman.c heap.c
	$(CC) -c -o $@ $< -fPIC
