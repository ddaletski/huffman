#ifndef HEAP_H
#define HEAP_H

typedef struct {
    void** buffer;
    int size;
    int capacity;
    int (*cmp)(const void* arg1, const void* arg2);
} binary_heap;

binary_heap* heap_create(int elements_count,
        int (*cmp_f)(const void* arg1, const void* arg2));
 
void heap_destroy(binary_heap* heap, void(*destroyer)(void* data));

char heap_insert(binary_heap* heap, void* data);

void* heap_pop(binary_heap* heap);

char heap_empty(binary_heap* heap);

char heap_full(binary_heap* heap);

#endif
