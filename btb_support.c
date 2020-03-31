#if !defined(IG_BTB_SUPPORT_C)
#define IG_BTB_SUPPORT_C

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

typedef struct cStoreTitle {
    pTransformation titleTransform;
    pTransformation menuTransform;

    bF32 accumulator;
    bF32 pulse;
    bBool isPulseExpanding;

    bBool isFadingIn;
    bBool isFadingOut;

    bU8 lastLeft;
    bU8 lastRight;

    bU8 lastMouseLeft;
    bU8 lastMouseRight;

    bF32 startTime;
} cStoreTitle;

typedef struct cStorePlay {
    bBool isFadingIn;

    bF32 locationX;
    bF32 locationY;
    bF32 velocityA;
    bF32 velocityX;
    bF32 vecolityY;

    bF32 startTime;
} cStorePlay;

#define C_FLAG_CANVAS_CHANGED 0x01
#define C_FLAG_MOUSE_CHANGED 0x02
#define C_FLAG_MOUSE_LEFT_PRESSED 0x04
#define C_FLAG_MOUSE_RIGHT_PRESSED 0x08

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
     * - C_FLAG_MOUSE_CHANGED - If the mouse moved or clicked.
     */
    bU32 flags;

    bU8 dPadUp;
    bU8 dPadDown;
    bU8 dPadLeft;
    bU8 dPadRight;

    bF32 mouseX;
    bF32 mouseY;

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
        cStorePlay play;
    } stageStore;
} cStore;

/*
 * ##### Palettes ######
 *
 * A palette is made of 16 colors, these are the only colors (plus alpha changes)
 * that can be on the screen at the same time.
 */

#define C_PACK_COLOR(r, g, b, a) \
    ( \
        (((bU32)r&0xFF) << 0) | \
        (((bU32)g&0xFF) << 8) | \
        (((bU32)b&0xFF) << 16) | \
        (((bU32)a&0xFF) << 24) \
    )

#define C_PALETTE_MAX 2

#define C_PALETTE_BACKGROUND 0x0
#define C_PALETTE_FOREGROUND 0x1
/* A high-contrast color for debug purposes. In doubt FF00FF is viable. */
#define C_PALETTE_HIGHCONTRAST 0x02
/* This last color always has to be total black. */
#define C_PALETTE_BLACK      0xF

B_INTERNAL const pPalette C_PALETTES[C_PALETTE_MAX + 1] = {
    /* White on black */
    {
        C_PACK_COLOR(0x00, 0x00, 0x08, 0xFF), /* 0 - Background */
        C_PACK_COLOR(0xCC, 0xCC, 0xFF, 0xFF), /* 1 - Foreground */
        C_PACK_COLOR(0xFF, 0x00, 0xFF, 0xFF), /* 2 - High-Contrast */
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
    /* Black on white */
    {
        C_PACK_COLOR(0xCC, 0xCC, 0xFF, 0xFF), /* 0 - Background */
        C_PACK_COLOR(0x00, 0x00, 0x08, 0xFF), /* 1 - Foreground */
        C_PACK_COLOR(0xFF, 0x00, 0xFF, 0xFF), /* 2 - High-Contrast */
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
    /* MAINFRAME */
    {
        C_PACK_COLOR(0x00, 0x00, 0x00, 0xFF), /* 0 - Background */
        C_PACK_COLOR(0x00, 0xAA, 0x00, 0xFF), /* 1 - Foreground */
        C_PACK_COLOR(0xFF, 0x00, 0xFF, 0xFF), /* 2 - High-Contrast */
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
};

/*
 * ##### Meshes #####
 */

B_INTERNAL bF32 C_SQUARE_MESH[] = {
    0, 1, 1,
    0, 0, 1,
    1, 0, 1,
    1, 0, 2,
    1, 1, 2,
    0, 1, 2
};
#define C_SQUARE_MESH_VERTICES (sizeof(C_SQUARE_MESH) / sizeof(pVertex))

#define C_BLIND_DISTANCE 5000

B_INTERNAL bF32 C_BLIND_MESH[] = {
     C_BLIND_DISTANCE*0,            C_BLIND_DISTANCE*0,
    -C_BLIND_DISTANCE*1,           -C_BLIND_DISTANCE*1,
     C_BLIND_DISTANCE*1 + C_WIDTH, -C_BLIND_DISTANCE*1,
     C_BLIND_DISTANCE*1 + C_WIDTH, -C_BLIND_DISTANCE*1,
     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0,
     C_BLIND_DISTANCE*0,            C_BLIND_DISTANCE*0,

     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0,
     C_BLIND_DISTANCE*1 + C_WIDTH, -C_BLIND_DISTANCE*1,
     C_BLIND_DISTANCE*1 + C_WIDTH,  C_BLIND_DISTANCE*1 + C_HEIGHT,
     C_BLIND_DISTANCE*1 + C_WIDTH,  C_BLIND_DISTANCE*1 + C_HEIGHT,
     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0 + C_HEIGHT,
     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0,

     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0 + C_HEIGHT,
     C_BLIND_DISTANCE*1 + C_WIDTH,  C_BLIND_DISTANCE*1 + C_HEIGHT,
    -C_BLIND_DISTANCE*1,            C_BLIND_DISTANCE*1 + C_HEIGHT,
    -C_BLIND_DISTANCE*1,            C_BLIND_DISTANCE*1 + C_HEIGHT,
     C_BLIND_DISTANCE*0,            C_BLIND_DISTANCE*0 + C_HEIGHT,
     C_BLIND_DISTANCE*0 + C_WIDTH,  C_BLIND_DISTANCE*0 + C_HEIGHT,

     C_BLIND_DISTANCE*0          ,  C_BLIND_DISTANCE*0 + C_HEIGHT,
    -C_BLIND_DISTANCE*1          ,  C_BLIND_DISTANCE*1 + C_HEIGHT,
    -C_BLIND_DISTANCE*1          , -C_BLIND_DISTANCE*1,
    -C_BLIND_DISTANCE*1          , -C_BLIND_DISTANCE*1,
     C_BLIND_DISTANCE*0          ,  C_BLIND_DISTANCE*0,
     C_BLIND_DISTANCE*0          ,  C_BLIND_DISTANCE*0 + C_HEIGHT,
};
#define C_BLIND_MESH_VERTICES (sizeof(C_BLIND_MESH) / sizeof(pVertexMono))

B_INTERNAL pTransformation C_IDENTITY = P_TRANSFORMATION_IDENTITY;
B_INTERNAL pTransformation C_BLIND_TRANS = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, -1, 1
};
B_INTERNAL pTransformation C_GLOBAL_TRANS = P_TRANSFORMATION_IDENTITY;

#include "_pp.c"

B_INTERNAL bF32 C_TITLE_MESH[] = PREPROC_OBJ_TITLE;
#define C_TITLE_MESH_VERTICES (sizeof(C_TITLE_MESH) / sizeof(pVertexMono))
#define C_TITLE_MESH_WIDTH (PREPROC_OBJ_TITLE_XMAX - PREPROC_OBJ_TITLE_XMIN)
#define C_TITLE_MESH_HEIGHT (PREPROC_OBJ_TITLE_YMAX - PREPROC_OBJ_TITLE_YMIN)

B_INTERNAL bF32 C_PLAY_MESH[] = PREPROC_OBJ_TEXT_PLAY;
#define C_PLAY_MESH_VERTICES (sizeof(C_PLAY_MESH) / sizeof(pVertexMono))
#define C_PLAY_MESH_WIDTH (PREPROC_OBJ_TEXT_PLAY_XMAX - PREPROC_OBJ_TEXT_PLAY_XMIN)
#define C_PLAY_MESH_HEIGHT (PREPROC_OBJ_TEXT_PLAY_YMAX - PREPROC_OBJ_TEXT_PLAY_YMIN)

B_INTERNAL bF32 C_PALETTE_MESH[] = PREPROC_OBJ_TEXT_PALETTE;
#define C_PALETTE_MESH_VERTICES (sizeof(C_PALETTE_MESH) / sizeof(pVertexMono))
#define C_PALETTE_MESH_WIDTH (PREPROC_OBJ_TEXT_PALETTE_XMAX - PREPROC_OBJ_TEXT_PALETTE_XMIN)
#define C_PALETTE_MESH_HEIGHT (PREPROC_OBJ_TEXT_PALETTE_YMAX - PREPROC_OBJ_TEXT_PALETTE_YMIN)

B_INTERNAL bF32 C_SPACEMAN_MESH[] = PREPROC_OBJ_SPACEMAN;
#define C_SPACEMAN_MESH_VERTICES (sizeof(C_SPACEMAN_MESH) / sizeof(pVertexMono))
#define C_SPACEMAN_MESH_WIDTH (PREPROC_OBJ_SPACEMAN_XMAX - PREPROC_OBJ_SPACEMAN_XMIN)
#define C_SPACEMAN_MESH_HEIGHT (PREPROC_OBJ_SPACEMAN_YMAX - PREPROC_OBJ_SPACEMAN_YMIN)

/* Constant strings. */

B_INTERNAL const char *C_HEXCHARS = "0123456789ABCDEF";

B_INTERNAL char C_ITERATION_MESSAGE[] = "TTTT iterations simulated.";
B_INTERNAL char C_DEBUG_MESSAGE[] = "T is the value.";

/* Utility */
#define C_PI PREPROC_PI
#define C_TAU (2*C_PI)

B_INTERNAL bF32 cLerp(bF32 start, bF32 end, bF32 alpha) {
    return start + alpha * (end - start);
}
B_INTERNAL bF32 cQLerp(bF32 start, bF32 end, bF32 alpha) {
    return cLerp(start, end, alpha*alpha);
}
B_INTERNAL bF32 cQEaseInOut(bF32 start, bF32 end, bF32 alpha) {
    alpha *= 2;
    if(alpha < 1) return (end - start) / 2 * alpha*alpha + start;
    alpha -= 1;
    return -(end - start)/2 * (alpha * (alpha - 2) - 1) + start;
}
B_INTERNAL bF32 cQEaseOut(bF32 start, bF32 end, bF32 alpha) {
    return -(end - start) * alpha * (alpha - 2) + start;
}
B_INTERNAL bBool cAABB(
    bF32 x, bF32 y,
    bF32 xmin, bF32 width,
    bF32 ymin, bF32 height
) {
    return x > xmin && x < xmin + width && y > ymin && y < ymin + height;
}

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

    bF32 *floatStack = (bF32*) &io->stack;

    for(bUWord i = 0; i < 16; i++) {
        floatStack[io->top++] = (*localT)[i];
    }
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
void cPushOutputDebugFloat(pOutputBuffer *io, bF32 f) {
    bF32 *floatStack = (bF32*) &io->stack;
    io->stack[io->top++] = P_OUTPUT_BUFFER_COMMAND_DEBUG_FLOAT;
    floatStack[io->top++] = f;
}
#define C_PRINT_FLOAT(_io, _msg) \
    do { \
        cPushOutputDebugFloat((_io), (_msg)); \
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
B_INTERNAL void cTransformTranslate(pTransformation *trans, bF32 x, bF32 y, bF32 z) {
    (*trans)[C_TRANSLATEX] = x;
    (*trans)[C_TRANSLATEY] = y;
    (*trans)[C_TRANSLATEZ] = z;
}
B_INTERNAL void cTransformScale(pTransformation *trans, bF32 x, bF32 y, bF32 z) {
    (*trans)[C_SCALEX] = x;
    (*trans)[C_SCALEY] = y;
    (*trans)[C_SCALEZ] = z;
    (*trans)[C_SCALEW] = 1.0f;
}
/*
 * @NOTE: This only deals with translation and scaling, everything else is
 * ignored intentionally.
 */
B_INTERNAL void cPerform2DInverseTransform(pTransformation *trans, bF32 *px, bF32 *py) {
    bF32 x = *px;
    bF32 y = *py;

    bF32 a = 1.0f / (*trans)[0];
    bF32 d = -1 * (*trans)[12];
    bF32 f = 1.0f / (*trans)[5];
    bF32 h = -1 * (*trans)[13];

    /* @TODO: This isn't correct*/
    *px = a*x + d*1 + (C_WIDTH / 2);
    *py = f*y + h*1 + (C_HEIGHT / 2);
}

#endif
