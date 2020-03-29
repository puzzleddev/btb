#include "b.h"
#include "p.h"

#define C_FRAME_MAX ((bF32)0.25)
#define C_FRAME_DELTA (((bF32)1)/((bF32)30))

typedef enum cStage {
    C_STAGE_INIT,
    C_STAGE_TITLE
} cStage;

#define C_FLAG_CANVAS_CHANGED 0x01

typedef struct cStore {
    bU32 state;
    bF32 frameAccumulator;
    bF32 lastTime;
    bF64 simulatedTime;

    bU32 flags;

    bU8 dPadUp;
    bU8 dPadDown;
    bU8 dPadLeft;
    bU8 dPadRight;

    bF32 stickAngle;

    bU16 canvasWidth;
    bU16 canvasHeight;
} cStore;

#define C_PACK_COLOR(r, g, b, a) \
    ( \
        (((bU32)r&0xFF) << 0) | \
        (((bU32)g&0xFF) << 8) | \
        (((bU32)b&0xFF) << 16) | \
        (((bU32)a&0xFF) << 24) \
    )

B_INTERNAL const pPalette C_PALETTE0 = {
    C_PACK_COLOR(0x00, 0x00, 0x08, 0xFF), /* 0 - Background */
    C_PACK_COLOR(0xFF, 0x00, 0x00, 0xFF), /* 1 - Red */
    C_PACK_COLOR(0x00, 0xFF, 0x00, 0xFF), /* 2 - Green */
    C_PACK_COLOR(0x00, 0x00, 0xFF, 0xFF), /* 3 - Blue */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 4 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 5 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 6 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 7 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 8 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 9 - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* A - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* B - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* C - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* D - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* E - UNUSED */
    C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* F - UNUSED */
};

B_INTERNAL pVertex C_TEST_VERTICES[] = {
    {
        .x = -0.5,
        .y =  0.5,
        .p = 1,
    },
    {
        .x = -0.5,
        .y = -0.5,
        .p = 1,
    },
    {
        .x =  0.5,
        .y = -0.5,
        .p = 1,
    },
    {
        .x =  0.5,
        .y = -0.5,
        .p = 2,
    },
    {
        .x = 0.5,
        .y = 0.5,
        .p = 2,
    },
    {
        .x = -0.5,
        .y =  0.5,
        .p = 2,
    },
};

B_INTERNAL const char *C_HEXCHARS = "0123456789ABCDEF";

B_INTERNAL char C_ITERATION_MESSAGE[] = "TTTT iterations simulated.";
B_INTERNAL char C_DEBUG_MESSAGE[] = "T is the value.";

/* Utility */
#define C_PI PREPROC_PI
#define C_TAU (2*C_PI)

#include "_pp.c"

#if 0
B_INTERNAL const bF32 C_SINE_TABLE[] = PREPROC_SINE;

B_INTERNAL bS32 cFloor(bF32 x) {
    return __builtin_floorf(x);
}
B_INTERNAL bS32 cCeil(bF32 x) {
    return __builtin_ceilf(x);
}
B_INTERNAL bF32 cFMod(bF32 x, bF32 y) {
    const bF32 a = x/y;
    return a - cFloor(a);
}
B_INTERNAL bF32 cLerp(bF32 x, bF32 y, bF32 a) {
    return x * (1 - a) + y * a;
}
#endif

/* Output interaction */
void cPushOutputPalette(pOutputBuffer *io, const pPalette *palette) {
    io->stack[io->top++] = P_OUTPUT_BUFFER_COMMAND_SET_PALETTE;
    io->stack[io->top++] = (bPointer) palette;
}
void cPushOutputClearColor(pOutputBuffer *io, bU8 index) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_CLEAR_COLOR) | (((bU32) index) << 8)
    );
}
void cPushOutputUploadMesh(
    pOutputBuffer *io,
    bU16 index,
    bU32 length,
    pVertex *mesh
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH) | (((bU32) index) << 16)
    );
    io->stack[io->top++] = length;
    io->stack[io->top++] = (bPointer) mesh;
}
void cPushOutputUpdateMesh(
    pOutputBuffer *io,
    bU16 index,
    bU32 length,
    pVertex *mesh
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_UPDATE_MESH) | (((bU32) index) << 16)
    );
    io->stack[io->top++] = length;
    io->stack[io->top++] = (bPointer) mesh;
}
void cPushOutputRenderMesh(
    pOutputBuffer *io,
    bU16 index,
    bU32 start,
    bU32 length
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_RENDER_MESH) | (((bU32) index) << 16)
    );
    io->stack[io->top++] = start;
    io->stack[io->top++] = length;
}
void cPushOutputDebugError(pOutputBuffer *io, char *message) {
    io->stack[io->top++] = P_OUTPUT_BUFFER_COMMAND_DEBUG_THROW;
    io->stack[io->top++] = (bPointer) message;
}
#define C_THROW(_io, _msg) \
    do { \
        (_io)->top = 0; cPushOutputDebugError((_io), (_msg)); return; \
    } while(B_FALSE)

void cPushOutputDebugPrint(pOutputBuffer *io, char *message) {
    io->stack[io->top++] = P_OUTPUT_BUFFER_COMMAND_DEBUG_PRINT;
    io->stack[io->top++] = (bPointer) message;
}
#define C_PRINT(_io, _msg) \
    do { \
        cPushOutputDebugPrint((_io), (_msg)); \
    } while(B_FALSE)

void cSimulateTitle(cStore *store, pOutputBuffer *io, bF32 delta) {
    B_UNUSED(store);
    B_UNUSED(io);
    B_UNUSED(delta);
}
void cDrawTitle(cStore *store, pOutputBuffer *io, bF32 alpha) {
    B_UNUSED(store);
    B_UNUSED(alpha);

    cPushOutputClearColor(io, 0x0);
    cPushOutputRenderMesh(io, 0, 0, B_ARRAYLENGTH(C_TEST_VERTICES));
}

void cFrame(pStore *pstore, pOutputBuffer *io, bF32 now) {

    cStore *store = (cStore *) pstore->staticStore;
    bF32 acc = store->frameAccumulator;

    bF32 frameTime = now - store->lastTime;
    if(frameTime > C_FRAME_MAX) {
        frameTime = C_FRAME_MAX;
    }
    store->lastTime = now;

    /* Handle input. */
    bU32 currentTop = 0;

    while(currentTop < io->top) {
        bU32 c = io->stack[currentTop++];

        switch(c & 0xFF) {
            case P_INPUT_BUFFER_COMMAND_SET_DPAD: {
                bU32 d = io->stack[currentTop++];
                store->dPadUp = (d >> 0) & 0xFF;
                store->dPadDown = (d >> 8) & 0xFF;
                store->dPadLeft = (d >> 16) & 0xFF;
                store->dPadRight = (d >> 24) & 0xFF;
                break;
            };
            case P_INPUT_BUFFER_COMMAND_SET_CANVAS_SIZE: {
                bU32 d = io->stack[currentTop++];
                bU16 w = (d >> 0) | 0xFFFF;
                bU16 h = (d >> 16) | 0xFFFF;

                store->canvasWidth = w;
                store->canvasHeight = h;

                store->flags |= C_FLAG_CANVAS_CHANGED;
                break;
            };
            default: {
                C_THROW(io, "Received invalid input command.");
                break;
            };
        }
    }

    if(currentTop != io->top) {
        C_THROW(io, "Input buffer desynchronization.");
    }

    /* Reset the IO buffer for writing. */
    io->top = 0;

    bU32 simulatedSteps = 0;

    acc += frameTime;
    while(acc > C_FRAME_DELTA) {
        /* Fixed step here. */

        if(store->state == C_STAGE_TITLE) {
            cSimulateTitle(store, io, C_FRAME_DELTA);
        }

        acc -= C_FRAME_DELTA;
        store->simulatedTime += C_FRAME_DELTA;
        simulatedSteps++;
    }
    store->frameAccumulator = acc;

    /* Writes out how many steps we simulate each frame. */
    #if 0
    simulatedSteps &= 0xFFFF;

    C_ITERATION_MESSAGE[0] = C_HEXCHARS[(simulatedSteps % 0x10000) / 0x1000];
    C_ITERATION_MESSAGE[1] = C_HEXCHARS[(simulatedSteps % 0x1000) / 0x100];
    C_ITERATION_MESSAGE[2] = C_HEXCHARS[(simulatedSteps % 0x100) / 0x10];
    C_ITERATION_MESSAGE[3] = C_HEXCHARS[(simulatedSteps % 0x10) / 0x1];

    if(simulatedSteps > 0) {
        C_PRINT(io, C_ITERATION_MESSAGE);
    }
    #endif

    /* Variable step here. */

    /*
     * We do initialization on the first possible frame, so this one case is
     * pulled out of the fixed-step update.
     */
    if(store->state == C_STAGE_INIT) {
        /* First time initialization */

        /* A few things in state are not zero initialized: */
        store->stickAngle = -1;

        cPushOutputPalette(io, &C_PALETTE0);

        cPushOutputUploadMesh(io, 0, B_ARRAYLENGTH(C_TEST_VERTICES), C_TEST_VERTICES);

        store->state = C_STAGE_TITLE;
    }

    if(store->state == C_STAGE_TITLE) {
        cDrawTitle(store, io, 0);
    }
}
