#include "btb_support.c"
#include "p.h"

/*
 * ##### Play Stage #####
 *
 * 1. Fade in on the player.
 * 2. Perfom physics.
 * 3. Generate the world procedually.
 */

#define C_SPLAY_FADEIN_TIME 3.0f

#define C_SPLAY_MESH_BLIND 0
#define C_SPLAY_MESH_BLIND_TYPE \
    (P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_BLACK << 4))
#define C_SPLAY_MESH_SPACEMAN 1
#define C_SPLAY_MESH_SPACEMAN_TYPE \
    (P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4))

void cStartPlay(cStore *cstore, pOutputBuffer *io) {
    cStorePlay *store = &cstore->stageStore.play;
    bZero(*store);

    store->isFadingIn = B_TRUE;
    store->startTime = cstore->lastTime;

    cPushOutputPalette(io, &C_PALETTES[cstore->currentPalette]);

    cPushOutputUploadMesh(
        io,
        C_SPLAY_MESH_BLIND_TYPE,
        C_SPLAY_MESH_BLIND,
        C_BLIND_MESH_VERTICES,
        C_BLIND_MESH
    );

    cPushOutputUploadMesh(
        io,
        C_SPLAY_MESH_SPACEMAN_TYPE,
        C_SPLAY_MESH_SPACEMAN,
        C_SPACEMAN_MESH_VERTICES,
        C_SPACEMAN_MESH
    );
}

void cSimulatePlay(cStore *cstore, pOutputBuffer *io, bF32 delta) {}

void cStopPlay(cStore *cstore, pOutputBuffer *io) {
    cPushOutputDeleteMesh(io, C_SPLAY_MESH_SPACEMAN);
    cPushOutputDeleteMesh(io, C_SPLAY_MESH_BLIND);
}

void cDrawPlay(cStore *cstore, pOutputBuffer *io, bF32 alpha) {
    cStorePlay *store = &cstore->stageStore.play;
    B_UNUSED(store);

    if(cstore->flags & C_FLAG_CANVAS_CHANGED) {
        cstore->flags &= ~C_FLAG_CANVAS_CHANGED;

        cComputeGlobalTransform(cstore, &C_GLOBAL_TRANS);
        cPushOutputSetGlobalTransform(io, &C_GLOBAL_TRANS);
    }

    cPushOutputClearColor(io, 0);

    cPushOutputRenderMesh(
        io,
        C_SPLAY_MESH_BLIND_TYPE,
        C_SPLAY_MESH_BLIND,
        0,
        C_BLIND_MESH_VERTICES,
        0xFFFF,
        &C_BLIND_TRANS
    );
}
