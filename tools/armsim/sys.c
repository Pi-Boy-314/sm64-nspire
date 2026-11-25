// ARMSIM build only: the parts of the C runtime that Ndless supplies on a calculator.
// The Ndless toolchain's newlib is built without malloc, so this is a small allocator:
// power-of-two size classes, one free list each, memory from sbrk and never given back.
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <reent.h>

#define MIN_SHIFT 4 // smallest block is 16 bytes, header included
#define NUM_CLASSES 27

struct block { // 8 bytes on ARM, so the payload after it stays 8-byte aligned
    uint32_t cls;
    struct block *next; // only meaningful while the block is free
};

static struct block *free_lists[NUM_CLASSES];

static size_t class_bytes(uint32_t cls) {
    return (size_t) 1 << (cls + MIN_SHIFT);
}

// calloc and realloc call this rather than malloc: GCC turns "malloc, then memset to zero"
// into a call to calloc, which inside calloc itself would loop forever.
static void *alloc(size_t n) {
    uint32_t cls = 0;
    while (class_bytes(cls) < n + sizeof(struct block)) {
        if (++cls == NUM_CLASSES) {
            return NULL;
        }
    }
    struct block *b = free_lists[cls];
    if (b != NULL) {
        free_lists[cls] = b->next;
    } else {
        uintptr_t brk = (uintptr_t) sbrk(0);
        if (brk & 7) {
            sbrk(8 - (brk & 7));
        }
        b = sbrk(class_bytes(cls));
        if (b == (void *) -1) {
            return NULL;
        }
        b->cls = cls;
    }
    return b + 1;
}

void *malloc(size_t n) {
    return alloc(n);
}

void free(void *p) {
    if (p != NULL) {
        struct block *b = (struct block *) p - 1;
        b->next = free_lists[b->cls];
        free_lists[b->cls] = b;
    }
}

void *calloc(size_t n, size_t size) {
    if (size != 0 && n > SIZE_MAX / size) {
        return NULL;
    }
    void *p = alloc(n * size);
    if (p != NULL) {
        memset(p, 0, n * size);
    }
    return p;
}

void *realloc(void *p, size_t n) {
    if (p == NULL) {
        return alloc(n);
    }
    if (n == 0) {
        free(p);
        return NULL;
    }
    const size_t cap = class_bytes(((struct block *) p - 1)->cls) - sizeof(struct block);
    if (n <= cap) {
        return p;
    }
    void *q = alloc(n);
    if (q != NULL) {
        memcpy(q, p, cap);
        free(p);
    }
    return q;
}

// newlib's own code (stdio) calls the reentrant versions
void *_malloc_r(struct _reent *r, size_t n) { (void) r; return malloc(n); }
void _free_r(struct _reent *r, void *p) { (void) r; free(p); }
void *_calloc_r(struct _reent *r, size_t n, size_t size) { (void) r; return calloc(n, size); }
void *_realloc_r(struct _reent *r, void *p, size_t n) { (void) r; return realloc(p, n); }

void abort(void) {
    _exit(134);
}
