#define B_IMPLEMENTATION
#include "b.h"
#include "p.h"

#include "btb.c"

#define P_HTML5_PAGE_SIZE ((bPointer) (0xFFFF + 1))

typedef struct pHtml5Page0 {
    bPointer ioBufferPointer;
    bPointer flags;
    pStore store;
} pHtml5Page0;

B_INTERNAL bU8 *heapStart = B_NULL;
B_INTERNAL pHtml5Page0 *page0 = B_NULL;
B_INTERNAL pOutputBuffer *outputBuffer = B_NULL;
B_INTERNAL bU8 *transitiveStore = B_NULL;

B_EXPORT bPointer pHtml5Frame(bF32 nowMillis) {
    /* We need the 'now' in seconds. */
    bF32 now = nowMillis/1000;

    if(heapStart == B_NULL) {
        page0->flags |= 0x01;

        bPointer currentMemory = __builtin_wasm_memory_size(0);

        /*
         * We need to skip over constant data, most of the time it will take up
         * less then a page, so we could start at page 1, but this is future
         * proof.
         */
        heapStart = (bU8 *)(currentMemory * P_HTML5_PAGE_SIZE);
        page0 = (pHtml5Page0 *)(heapStart + 0 * P_HTML5_PAGE_SIZE);
        outputBuffer = (pOutputBuffer *)(heapStart + 1 * P_HTML5_PAGE_SIZE);
        transitiveStore = (bU8 *)(heapStart + 2 * P_HTML5_PAGE_SIZE);

        /* Make sure we actually have memory for everything above. */
        __builtin_wasm_memory_grow(0, 3);

        page0->ioBufferPointer = (bPointer) outputBuffer;
        outputBuffer->top = 0;

        page0->store.transitiveSize = P_HTML5_PAGE_SIZE * 1;
        page0->store.transitiveStore = transitiveStore;
    }

    cFrame(&page0->store, outputBuffer, now);

    return (bPointer)page0;
}
