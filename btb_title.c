#include "btb_support.c"

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
/* Start and end alpha of the menu. */
#define C_STITLE_MENU_ASTART 0x0000
#define C_STITLE_MENU_AEND 0xFFFF

#define C_STITLE_MENU_YEND (C_HEIGHT * 0.4f)

#define C_STITLE_PULSE_FREQ 0.5f
#define C_STITLE_MENU_PULSE_MIN 1.0f
#define C_STITLE_MENU_PULSE_MAX 1.1f

#define C_STITLE_MENU_PALETTE_YOFF (C_HEIGHT * -0.2f)

#define C_STITLE_FADEOUT_TIME 0.5f
#define C_STITLE_FADEOUT_ASTART 0xFFFF
#define C_STITLE_FADEOUT_AEND 0x0000

#define C_STITLE_MESH_TITLE 0
#define C_STITLE_MESH_BLIND 1
#define C_STITLE_MESH_PLAY 2
#define C_STITLE_MESH_PALETTE 3

#define C_STITLE_MENU_SCALE ((1.0f / (bF32)C_PLAY_MESH_WIDTH) * C_WIDTH * 0.2)

void cStartTitle(cStore *cstore, pOutputBuffer *io) {
    cStoreTitle *store = &cstore->stageStore.title;

    bFillMemory(store, sizeof(*store), 0);

    store->startTime = cstore->simulatedTime;
    store->isFadingIn = B_TRUE;

    cTransformScale(
        &store->titleTransform,
        (1.0f / (bF32)C_TITLE_MESH_WIDTH) * C_WIDTH * 0.9,
        (1.0f / -(bF32)C_TITLE_MESH_WIDTH) * C_WIDTH * 0.9,
        1.0f
    );
    cTransformTranslate(
        &store->titleTransform,
        C_WIDTH * 0.5,
        C_STITLE_TITLE_YSTART,
        0.0f
    );

    cTransformScale(
        &store->menuTransform,
        C_STITLE_MENU_SCALE,
        -C_STITLE_MENU_SCALE,
        1.0f
    );
    cTransformTranslate(
        &store->menuTransform,
        C_WIDTH * 0.5,
        C_STITLE_MENU_YEND,
        0.5f
    );

    cPushOutputUploadMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4),
        C_STITLE_MESH_TITLE,
        C_TITLE_MESH_VERTICES,
        C_TITLE_MESH
    );

    cPushOutputUploadMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_BLACK << 4),
        C_STITLE_MESH_BLIND,
        C_BLIND_MESH_VERTICES,
        C_BLIND_MESH
    );

    cPushOutputUploadMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4),
        C_STITLE_MESH_PLAY,
        C_PLAY_MESH_VERTICES,
        C_PLAY_MESH
    );

    cPushOutputUploadMesh(
        io,
        P_OUTPUT_BUFFER_TYPE_VERTEX_MONO | (C_PALETTE_FOREGROUND << 4),
        C_STITLE_MESH_PALETTE,
        C_PALETTE_MESH_VERTICES,
        C_PALETTE_MESH
    );
}
void cSimulateTitle(cStore *cstore, pOutputBuffer *io, bF32 delta) {
    B_UNUSED(delta);
    cStoreTitle *store = &cstore->stageStore.title;

    bS8 nextPalette = cstore->currentPalette;

    /* Keyboard calculations */
    {
        bBool canChangePalette = B_FALSE;

        if(
            store->lastLeft != cstore->dPadLeft ||
            store->lastRight != cstore->dPadRight
        ) {
            canChangePalette = B_TRUE;
        }

        store->lastLeft = cstore->dPadLeft;
        store->lastRight = cstore->dPadRight;

        if(canChangePalette && store->lastLeft) nextPalette--;
        if(canChangePalette && store->lastRight) nextPalette++;
    }

    /* Mouse calculations */
    {
        bF32 mx = cstore->mouseX;
        bF32 my = cstore->mouseY;

        cPerform2DInverseTransform(&C_GLOBAL_TRANS, &mx, &my);

        bBool canLClick = B_FALSE;
        bBool canRClick = B_FALSE;

        if(store->lastMouseLeft != (cstore->flags & C_FLAG_MOUSE_LEFT_PRESSED)) {
            canLClick = B_TRUE;
        }
        if(store->lastMouseRight != (cstore->flags & C_FLAG_MOUSE_RIGHT_PRESSED)) {
            canRClick = B_TRUE;
        }
        store->lastMouseLeft = cstore->flags & C_FLAG_MOUSE_LEFT_PRESSED;
        store->lastMouseRight = cstore->flags & C_FLAG_MOUSE_RIGHT_PRESSED;

        bF32 menuX = C_WIDTH * 0.5;
        bF32 menuY = C_STITLE_MENU_YEND;

        bBool inPalette = cAABB(mx, my,
            menuX - (C_STITLE_MENU_SCALE * C_PALETTE_MESH_WIDTH * 0.5),
            C_STITLE_MENU_SCALE * C_PALETTE_MESH_WIDTH,
            menuY - (C_STITLE_MENU_SCALE * C_PALETTE_MESH_HEIGHT * 0.5) + C_STITLE_MENU_PALETTE_YOFF,
            C_STITLE_MENU_SCALE * C_PALETTE_MESH_HEIGHT
        );
        bBool inPlay = cAABB(mx, my,
            menuX - (C_STITLE_MENU_SCALE * C_PLAY_MESH_WIDTH * 0.5),
            C_STITLE_MENU_SCALE * C_PLAY_MESH_WIDTH,
            menuY - (C_STITLE_MENU_SCALE * C_PLAY_MESH_HEIGHT * 0.5),
            C_STITLE_MENU_SCALE * C_PLAY_MESH_HEIGHT
        );

        if(canLClick && (cstore->flags & C_FLAG_MOUSE_LEFT_PRESSED)) {
            if(inPalette) {
                nextPalette++;
            }
            if(inPlay) {
                /* TODO: Start play stage. */
                store->isFadingOut = B_TRUE;
                store->startTime = cstore->lastTime;
            }
        }

        if(canRClick && (cstore->flags & C_FLAG_MOUSE_RIGHT_PRESSED)) {
            if(inPalette) {
                nextPalette--;
            }
        }
    }
    
    if(nextPalette != cstore->currentPalette) {
        if(nextPalette < 0) nextPalette = C_PALETTE_MAX;
        else if(nextPalette > C_PALETTE_MAX) nextPalette = 0;

        cPushOutputPalette(io, &C_PALETTES[nextPalette]);

        cstore->currentPalette = nextPalette;
    }

    
}
void cStopTitle(cStore *store, pOutputBuffer *io) {
    B_UNUSED(store);

    cPushOutputDeleteMesh(io, C_STITLE_MESH_TITLE);
    cPushOutputDeleteMesh(io, C_STITLE_MESH_BLIND);
    cPushOutputDeleteMesh(io, C_STITLE_MESH_PLAY);
    cPushOutputDeleteMesh(io, C_STITLE_MESH_PALETTE);
}
void cDrawTitle(cStore *cstore, pOutputBuffer *io, bF32 alpha) {
    cStoreTitle *store = &cstore->stageStore.title;

    if(cstore->flags & C_FLAG_CANVAS_CHANGED) {
        cstore->flags &= ~C_FLAG_CANVAS_CHANGED;

        cComputeGlobalTransform(cstore, &C_GLOBAL_TRANS);
        cPushOutputSetGlobalTransform(io, &C_GLOBAL_TRANS);
    }

    /* If any button is pressed, skip the animation. */
    if(
        cstore->dPadUp || cstore->dPadDown || cstore->dPadLeft || cstore->dPadRight ||
        cstore->flags & (C_FLAG_MOUSE_LEFT_PRESSED | C_FLAG_MOUSE_RIGHT_PRESSED)
    ) {
        store->isFadingIn = B_FALSE;
    }

    bF32 titleY = C_STITLE_TITLE_YEND;
    bF32 menuAlpha = C_STITLE_MENU_AEND;

    if(store->isFadingIn) {
        if(cstore->lastTime - store->startTime >= C_STITLE_TIME_TO_MENU) {
            store->isFadingIn = B_FALSE;
        } else {
            titleY = cQEaseOut(
                C_STITLE_TITLE_YSTART,
                C_STITLE_TITLE_YEND,
                ((cstore->lastTime - store->startTime) / C_STITLE_TIME_TO_MENU)
            );

            menuAlpha = cQLerp(
                C_STITLE_MENU_ASTART,
                C_STITLE_MENU_AEND,
                ((cstore->lastTime - store->startTime) / C_STITLE_TIME_TO_MENU)
            );
        }
    }

    bF32 globalAlpha = C_STITLE_FADEOUT_ASTART;

    if(store->isFadingOut) {
        if(cstore->lastTime - store->startTime >= C_STITLE_FADEOUT_TIME) {
            globalAlpha = C_STITLE_FADEOUT_AEND;
            cstore->nextStage = C_STAGE_PLAY;
        } else {
            globalAlpha = cQEaseOut(
                C_STITLE_FADEOUT_ASTART,
                C_STITLE_FADEOUT_AEND,
                ((cstore->lastTime - store->startTime) / C_STITLE_FADEOUT_TIME)
            );
        }
    }

#if 0
    /* Example of reverse transforming. */
    if(cstore->flags & C_FLAG_MOUSE_LEFT_PRESSED) {
        bF32 mx = cstore->mouseX;
        bF32 my = cstore->mouseY;

        cPerform2DInverseTransform(&C_GLOBAL_TRANS, &mx, &my);

        store->titleTransform[C_TRANSLATEX] = mx;
        titleY = my;
    }
#endif

    store->titleTransform[C_TRANSLATEY] = titleY;

    cPushOutputClearColor(io, C_PALETTE_BACKGROUND);

    cPushOutputRenderMesh(
        io,
        ((bU8) P_OUTPUT_BUFFER_TYPE_VERTEX_MONO) | (((bU8) C_PALETTE_FOREGROUND) << 4),
        C_STITLE_MESH_TITLE,
        0,
        C_TITLE_MESH_VERTICES,
        store->isFadingOut ? globalAlpha : 0xFFFF,
        &store->titleTransform
    );

    cPushOutputRenderMesh(
        io,
        ((bU8) P_OUTPUT_BUFFER_TYPE_VERTEX_MONO) | (((bU8) C_PALETTE_BLACK) << 4),
        C_STITLE_MESH_BLIND,
        0,
        C_BLIND_MESH_VERTICES,
        0xFFFF,
        &C_BLIND_TRANS
    );

    bF32 pulseMultiplier = C_STITLE_MENU_PULSE_MIN;

    if(cstore->lastTime - store->pulse >= C_STITLE_PULSE_FREQ) {
        store->pulse = cstore->lastTime;
        store->isPulseExpanding = !store->isPulseExpanding;
    }

    if(store->isPulseExpanding) {
        pulseMultiplier = cQEaseInOut(
            C_STITLE_MENU_PULSE_MIN,
            C_STITLE_MENU_PULSE_MAX,
            ((cstore->lastTime - store->pulse) / C_STITLE_PULSE_FREQ)
        );
    } else {
        pulseMultiplier = cQEaseInOut(
            C_STITLE_MENU_PULSE_MAX,
            C_STITLE_MENU_PULSE_MIN,
            ((cstore->lastTime - store->pulse) / C_STITLE_PULSE_FREQ)
        );
    }

    cTransformScale(
        &store->menuTransform,
        C_STITLE_MENU_SCALE * pulseMultiplier,
        -C_STITLE_MENU_SCALE * pulseMultiplier,
        1.0f
    );

    cPushOutputRenderMesh(
        io,
        ((bU8) P_OUTPUT_BUFFER_TYPE_VERTEX_MONO) | (((bU8) C_PALETTE_FOREGROUND) << 4),
        C_STITLE_MESH_PLAY,
        0,
        C_PLAY_MESH_VERTICES,
        store->isFadingOut ? globalAlpha : menuAlpha,
        &store->menuTransform
    );

    store->menuTransform[C_TRANSLATEY] += C_STITLE_MENU_PALETTE_YOFF;
    cPushOutputRenderMesh(
        io,
        ((bU8) P_OUTPUT_BUFFER_TYPE_VERTEX_MONO) | (((bU8) C_PALETTE_FOREGROUND) << 4),
        C_STITLE_MESH_PALETTE,
        0,
        C_PALETTE_MESH_VERTICES,
        store->isFadingOut ? globalAlpha : menuAlpha,
        &store->menuTransform
    );
    store->menuTransform[C_TRANSLATEY] -= C_STITLE_MENU_PALETTE_YOFF;
}
