#include "b.h"
#include "p.h"

/*
 * General constants.
 */

#define C_SCALEX 0
#define C_SCALEY 5
#define C_SCALEZ 10
#define C_SCALEW 15
#define C_TRANSLATEX 12
#define C_TRANSLATEY 13
#define C_TRANSLATEZ 14

#define C_PALETTE_MAX 1

/* The confines of the assumed screen. */
#define C_WIDTH ((bF32) 400)
#define C_HEIGHT ((bF32) 300)
#define C_ASPECT (C_WIDTH/C_HEIGHT)

/*
 * The maximum frame lag, everything above gets floored to this. This is done to
 * make sure that we never have to execute too many cycles in one frame, as it
 * puts an upper bound on it.
 */
#define C_FRAME_MAX ((bF32)0.25)

/*
 * The time a single cycle simulates.
 */
#define C_FRAME_DELTA (((bF32)1)/((bF32)30))

typedef enum cStage {
    /* Initial stage, all it does is upload basic data and switch to TITLE. */
    C_STAGE_INIT,

    /*
     * The title screen, initially only shows the title, but also includes the
     * main menu. Once the 'play' button is pressed it transitions into the
     * 'PLAY' stage.
     */
    C_STAGE_TITLE,

    /* Plays the game, also includes the in-game pause menu. */
    C_STAGE_PLAY,
} cStage;

#define C_FLAG_CANVAS_CHANGED 0x01

typedef struct cStoreTitle {
    pTransformation titleTransform;
    bF32 accumulator;

    bU8 lastLeft;
    bU8 lastRight;

    bF32 startTime;
} cStoreTitle;

typedef struct cStore {
    bU32 stage;
    bU32 nextStage;
    bF32 frameAccumulator;
    bF32 lastTime;
    bF64 simulatedTime;

    /*
     * This variable is used by the input system to signal that something has
     * happened. The following flags are used:
     *
     * - C_FLAG_CANVAS_CHANGED - If the canvas size has changed or it is
     *                           otherwise dirty.
     */
    bU32 flags;

    bU8 dPadUp;
    bU8 dPadDown;
    bU8 dPadLeft;
    bU8 dPadRight;

    /*
     * The angle, the analog stick is held at, if it is not at the edge this is
     * negative instead.
     */
    bF32 stickAngle;

    bU16 canvasWidth;
    bU16 canvasHeight;

    bU8 currentPalette;

    union cStoreStageStore {
        cStoreTitle title;
    } stageStore;
} cStore;

#define C_PACK_COLOR(r, g, b, a) \
    ( \
        (((bU32)r&0xFF) << 0) | \
        (((bU32)g&0xFF) << 8) | \
        (((bU32)b&0xFF) << 16) | \
        (((bU32)a&0xFF) << 24) \
    )

#define C_PALETTE_BACKGROUND 0x0
#define C_PALETTE_FOREGROUND 0x1
#define C_PALETTE_BLACK      0xF

B_INTERNAL const pPalette C_PALETTES[C_PALETTE_MAX + 1] = {
    {
        C_PACK_COLOR(0x00, 0x00, 0x08, 0xFF), /* 0 - Background */
        C_PACK_COLOR(0xCC, 0xCC, 0xFF, 0xFF), /* 1 - Foreground */
        C_PACK_COLOR(0x00, 0xFF, 0x00, 0xFF), /* 2 - High-Contrast */
        C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 3 - UNUSED */
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
        C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* F - Black */
    },
    {
        C_PACK_COLOR(0xCC, 0xCC, 0xFF, 0xFF), /* 0 - Background */
        C_PACK_COLOR(0x00, 0x00, 0x08, 0xFF), /* 1 - Foreground */
        C_PACK_COLOR(0x00, 0xFF, 0x00, 0xFF), /* 2 - High-Contrast */
        C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 3 - UNUSED */
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
        C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* F - Black */
    }
};

B_INTERNAL bF32 C_TEST_MESH[] = {
    0, 1, 1,
    0, 0, 1,
    1, 0, 1,
    1, 0, 2,
    1, 1, 2,
    0, 1, 2
};
#define C_TEST_MESH_VERTICES (sizeof(C_TEST_MESH) / sizeof(pVertex))
#define C_TEST_MESH_FACES (C_TEST_MESH_VERTICES / 3)
#define C_TEST_MESH_WIDTH (1.0f)
#define C_TEST_MESH_HEIGHT (1.0f)

B_INTERNAL pTransformation C_IDENTITY = P_TRANSFORMATION_IDENTITY;
B_INTERNAL pTransformation C_GLOBAL_TRANS = P_TRANSFORMATION_IDENTITY;

#include "_pp.c"

B_INTERNAL bF32 C_TITLE_MESH[] = PREPROC_OBJ_TITLE;
#define C_TITLE_MESH_VERTICES (sizeof(C_TITLE_MESH) / sizeof(pVertexMono))
#define C_TITLE_MESH_FACES (C_TITLE_MESH_VERTICES / 3)
#define C_TITLE_MESH_WIDTH (PREPROC_OBJ_TITLE_XMAX - PREPROC_OBJ_TITLE_XMIN)
#define C_TITLE_MESH_HEIGHT (PREPROC_OBJ_TITLE_YMAX - PREPROC_OBJ_TITLE_YMIN)

B_INTERNAL const char *C_HEXCHARS = "0123456789ABCDEF";

B_INTERNAL char C_ITERATION_MESSAGE[] = "TTTT iterations simulated.";
B_INTERNAL char C_DEBUG_MESSAGE[] = "T is the value.";

/* Utility */
#define C_PI PREPROC_PI
#define C_TAU (2*C_PI)

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
    bU8 type,
    bU16 index,
    bU32 length,
    bF32 *mesh
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH) |
        (((bU32) type) << 8) |
        (((bU32) index) << 16)
    );
    io->stack[io->top++] = length;
    io->stack[io->top++] = (bPointer) mesh;
}
void cPushOutputUpdateMesh(
    pOutputBuffer *io,
    bU8 type,
    bU16 index,
    bU32 length,
    bF32 *mesh
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_UPDATE_MESH) |
        (((bU32) type) << 8) |
        (((bU32) index) << 16)
    );
    io->stack[io->top++] = length;
    io->stack[io->top++] = (bPointer) mesh;
}
void cPushOutputRenderMesh(
    pOutputBuffer *io,
    bU8 type,
    bU16 index,
    bU32 start,
    bU16 length,
    bU16 alpha,
    pTransformation *localT
) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_RENDER_MESH) |
        (((bU32) type) << 8) |
        (((bU32) index) << 16)
    );
    io->stack[io->top++] = start;
    io->stack[io->top++] = (
        (((bU32) length) << 0) |
        (((bU32) alpha) << 16)
    );
    io->stack[io->top++] = (bPointer) localT;
}
void cPushOutputDeleteMesh(pOutputBuffer *io, bU16 index) {
    io->stack[io->top++] = (
        ((bU32) P_OUTPUT_BUFFER_COMMAND_DELETE_MESH) | (((bU32) index) << 16)
    );
}
void cPushOutputSetGlobalTransform(pOutputBuffer *io, pTransformation *globalT) {
    io->stack[io->top++] = P_OUTPUT_BUFFER_COMMAND_SET_TRANSFORMATION;
    io->stack[io->top++] = (bPointer) globalT;
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

B_INTERNAL void cComputeGlobalTransform(cStore *store, pTransformation *trans) {
    bF32 width = store->canvasWidth;
    bF32 height = store->canvasHeight;
    bF32 aspect = width / height;

    if(C_ASPECT < aspect) {
        (*trans)[C_SCALEX] = ((1.0f / aspect) / C_HEIGHT) * 2;
        (*trans)[C_SCALEY] = (1.0f / C_HEIGHT) * 2;

        (*trans)[C_TRANSLATEX] = -1 + (aspect-C_ASPECT) * (1/aspect);
        (*trans)[C_TRANSLATEY] = -1;
    } else {
        (*trans)[C_SCALEX] = (1.0f / C_WIDTH) * 2;
        (*trans)[C_SCALEY] = (aspect / C_WIDTH) * 2;

        (*trans)[C_TRANSLATEX] = -1;
        (*trans)[C_TRANSLATEY] = -1 + (C_ASPECT-aspect) * (1/C_ASPECT);
    }
}

/*
 * ##### Title Stage ######
 *
 * This is the first proper stage, it has the following logic:
 * 1. Display the title
 * 2. Move it up and reveal the menu.
 * 3. If a button is pressed (not an analog input) jump to the end.
 * 4. Perform the menu.
 */

/*
 * How many seconds it takes the title to move up and the menu to become visible.
 */
#define C_STITLE_TIME_TO_MENU 2.0f
/* The start and end positions for the move. */
#define C_STITLE_TITLE_YSTART 150
#define C_STITLE_TITLE_YEND   250

void cStartTitle(cStore *cstore, pOutputBuffer *io) {
    cStoreTitle *store = &cstore->stageStore.title;

    store->startTime = cstore->simulatedTime;

    store->accumulator = 0;

    store->titleTransform[C_SCALEX] = 1.0f / (bF32)C_TITLE_MESH_WIDTH;
    store->titleTransform[C_SCALEY] = 1.0f / -(bF32)C_TITLE_MESH_WIDTH;
    store->titleTransform[C_SCALEZ] = 1.0f;
    store->titleTransform[C_SCALEW] = 1.0f;

    store->titleTransform[C_SCALEX] *= C_WIDTH * 0.8;
    store->titleTransform[C_SCALEY] *= C_WIDTH * 0.8;

    store->titleTransform[C_TRANSLATEX] = 200;
    store->titleTransform[C_TRANSLATEY] = C_STITLE_TITLE_YSTART;
    store->titleTransform[C_TRANSLATEZ] = -0.9;

    cPushOutputUploadMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4),
        0,
        C_TITLE_MESH_VERTICES,
        C_TITLE_MESH
    );
}
void cSimulateTitle(cStore *cstore, pOutputBuffer *io, bF32 delta) {
    B_UNUSED(delta);
    cStoreTitle *store = &cstore->stageStore.title;

    bBool canChange = B_FALSE;

    if(
        store->lastLeft != cstore->dPadLeft ||
        store->lastRight != cstore->dPadRight
    ) {
        canChange = B_TRUE;
    }

    store->lastLeft = cstore->dPadLeft;
    store->lastRight = cstore->dPadRight;

    bS8 nextPalette = cstore->currentPalette;

    if(canChange && store->lastLeft) nextPalette--;
    if(canChange && store->lastRight) nextPalette++;

    if(nextPalette != cstore->currentPalette) {
        if(nextPalette < 0) nextPalette = C_PALETTE_MAX;
        else if(nextPalette > C_PALETTE_MAX) nextPalette = 0;

        cPushOutputPalette(io, &C_PALETTES[nextPalette]);

        cstore->currentPalette = nextPalette;
    }

    if(cstore->flags & C_FLAG_CANVAS_CHANGED) {
        cstore->flags &= ~C_FLAG_CANVAS_CHANGED;

        cComputeGlobalTransform(cstore, &C_GLOBAL_TRANS);
        cPushOutputSetGlobalTransform(io, &C_GLOBAL_TRANS);
    }
}
void cStopTitle(cStore *store, pOutputBuffer *io) {
    B_UNUSED(store);

    cPushOutputDeleteMesh(io, 1);
    cPushOutputDeleteMesh(io, 0);
}
void cDrawTitle(cStore *cstore, pOutputBuffer *io, bF32 alpha) {
    B_UNUSED(alpha);
    cStoreTitle *store = &cstore->stageStore.title;

    if(cstore->lastTime - store->startTime >= C_STITLE_TIME_TO_MENU) {
        store->titleTransform[C_TRANSLATEY] = C_STITLE_TITLE_YEND;
    } else {
        store->titleTransform[C_TRANSLATEY] =
            C_STITLE_TITLE_YSTART + 
            ((cstore->lastTime - store->startTime) / C_STITLE_TIME_TO_MENU) *
            (C_STITLE_TITLE_YEND - C_STITLE_TITLE_YSTART);
    }

    cPushOutputClearColor(io, C_PALETTE_BACKGROUND);

    cPushOutputRenderMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4),
        0,
        0,
        C_TITLE_MESH_VERTICES,
        0xFFFF,
        &store->titleTransform
    );
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
                bU16 w = (d >> 0) & 0xFFFF;
                bU16 h = (d >> 16) & 0xFFFF;

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

    /*
     * We do initialization on the first possible frame, so this one case is
     * pulled out of the fixed-step update.
     */
    if(store->stage == C_STAGE_INIT) {
        /* First time initialization */

        /* A few things in state are not zero initialized: */
        store->stickAngle = -1;

        cPushOutputPalette(io, &C_PALETTES[0]);

        store->nextStage = C_STAGE_TITLE;
    }

    bU32 simulatedSteps = 0;

    acc += frameTime;
    while(acc > C_FRAME_DELTA) {
        /* Fixed step here. */

        if(store->nextStage != store->stage) {
            switch(store->stage) {
                case C_STAGE_INIT: break;
                case C_STAGE_TITLE:
                    cStopTitle(store, io);
                    break;
                default:
                    C_THROW(io, "Unknown stage to stop.");
                    break;
            }

            store->stage = store->nextStage;

            switch(store->stage) {
                case C_STAGE_TITLE:
                    cStartTitle(store, io);
                    break;
                default:
                    C_THROW(io, "Unknown stage to start.");
                    break;
            }
        }

        switch(store->stage) {
            case C_STAGE_TITLE:
                cSimulateTitle(store, io, C_FRAME_DELTA);
                break;
            default:
                C_THROW(io, "Unknown stage to simulate.");
                break;
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

    switch(store->stage) {
        case C_STAGE_TITLE:
            cDrawTitle(store, io, 0);
            break;
        default:
            C_THROW(io, "Unknown stage to draw.");
            break;
    }
}
