#include "btb_support.c"

#include "btb_title.c"
#include "btb_play.c"

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

    bF32 *floatStack = (bF32*) &io->stack;

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
            case P_INPUT_BUFFER_COMMAND_SET_MOUSE: {
                bF32 x = floatStack[currentTop++];
                bF32 y = floatStack[currentTop++];

                bBool left = (c >> 8) & 0x01;
                bBool right = (c >> 16) & 0x01;

                store->flags |= C_FLAG_MOUSE_CHANGED;
                store->flags &= ~(C_FLAG_MOUSE_LEFT_PRESSED | C_FLAG_MOUSE_RIGHT_PRESSED);
                store->flags |= C_FLAG_MOUSE_LEFT_PRESSED * left;
                store->flags |= C_FLAG_MOUSE_RIGHT_PRESSED * right;
                store->mouseX = (x-0.5)*2;
                store->mouseY = ((y-0.5)*2)*-1;
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

        /* For blind debugging */
        //store->currentPalette = 1;

        cPushOutputPalette(io, &C_PALETTES[store->currentPalette]);

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
                case C_STAGE_PLAY:
                    cStopPlay(store, io);
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
                case C_STAGE_PLAY:
                    cStartPlay(store, io);
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
            case C_STAGE_PLAY:
                cSimulatePlay(store, io, C_FRAME_DELTA);
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
        case C_STAGE_PLAY:
            cDrawPlay(store, io, 0);
            break;
        default:
            C_THROW(io, "Unknown stage to draw.");
            break;
    }
}
