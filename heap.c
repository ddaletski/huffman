#include "heap.h"
#include <stdlib.h>

/* ============= helpers =============== */

static void swap(void** a, void** b) {
    void* c = *a;
    *a = *b;
    *b = c;
}

static void heapify_up(binary_heap* heap, int n) {
    // moves n-th element up (if it's needed)
    if(n == 0) return;
    if(heap->cmp(heap->buffer[n], heap->buffer[(n-1)/2]) < 0) {
        swap(&heap->buffer[n], &heap->buffer[(n-1)/2]);
        heapify_up(heap, (n-1)/2);
    }
}

static void heapify_down(binary_heap* heap, int n) {
    // moves n-th element down (if it's needed)
    if(heap->size > 2*n + 2) { // if it has two childs
        void* left = heap->buffer[2*n + 1];
        void* right = heap->buffer[2*n + 2];
        void* current = heap->buffer[n];

        if(heap->cmp(current, left) > 0) {  // swap with left child
                                            // and heapify it
            swap(&heap->buffer[n], &heap->buffer[2*n + 1]);
            current = heap->buffer[n];
            heapify_down(heap, 2*n + 1);
        }
        if(heap->cmp(current, right) > 0) { // swap with right child
                                            // and heapify it
            swap(&heap->buffer[n], &heap->buffer[2*n + 2]);
            heapify_down(heap, 2*n + 2);
        }
    } else if (heap->size < 2*n + 2) { // if it has no childs
        return;                        // do nothing
    } else { // if it has only left child 
        if(heap->cmp(heap->buffer[n], heap->buffer[2*n + 1]) > 0)
            swap(&heap->buffer[n], &heap->buffer[2*n + 1]);
    }
}

/* =========== public functions ============ */

binary_heap* heap_create(int elements_count,
        int (*cmp_f)(const void* arg1, const void* arg2)) {

    binary_heap* heap = malloc(sizeof(binary_heap));

    heap->buffer = malloc(sizeof(void*) * elements_count);
    heap->capacity = elements_count;
    heap->size = 0;
    heap->cmp = cmp_f;

    return heap;
}


void heap_destroy(binary_heap* heap, void (*destroyer) (void* data)) {
    if(destroyer)
        for(int i = 0; i < heap->size; ++i)
            destroyer(heap->buffer[i]);

    free(heap->buffer);
    free(heap);
}


char heap_insert(binary_heap* heap, void* data) {
    if(heap->size == heap->capacity)
        return 0;

    heap->buffer[heap->size] = data; // insert element to the rear
    heapify_up(heap, heap->size++);  // restore the heap property
    return 1;
}


void* heap_pop(binary_heap* heap) {
    if(heap->size == 0)
        return NULL;
    else {
        void* data = heap->buffer[0];
        swap(&heap->buffer[0], &heap->buffer[--heap->size]);
        heapify_down(heap, 0);
        return data;
    }
}


char heap_empty(binary_heap* heap) {
    return heap->size == 0;
}


char heap_full(binary_heap* heap) {
    return heap->size == heap->capacity;
}
