#if !defined(IG_P_H)
#define IG_P_H

#include "b.h"

/* Client interface */

#define P_STATIC_STORE_SIZE (0x100*0x10)
typedef bU8 pStaticStore[P_STATIC_STORE_SIZE];

typedef struct pStore {
    pStaticStore *staticStore;
    bPointer transitiveSize;
    bByte *transitiveStore;
} pStore;

#define P_PALETTE_SIZE 0x10
typedef bU32 pPalette[P_PALETTE_SIZE];

/*
 * These are not in an enum, because their actual values need to be kept in sync
 * with the JS side of things.
 */

/*
 * Sets the color palette. The palette is made up of 16 bytes, each describing
 * one color in RGBA8 format.
 *
 * - 0:0 - Command byte.
 * - 1   - Palette pointer.
 */
#define P_OUTPUT_BUFFER_COMMAND_SET_PALETTE 0x01

/*
 * Clears the color buffer to the given color.
 *
 * - 0:0 - Command byte
 * - 0:1 - Palette index
 */
#define P_OUTPUT_BUFFER_COMMAND_CLEAR_COLOR 0x02

/*
 * Throws a debug error.
 *
 * - 0:0 - Command byte
 * - 1   - Zero terminated string pointer.
 */
#define P_OUTPUT_BUFFER_COMMAND_DEBUG_THROW 0xF0

/*
 * Prints a debug message to console.
 *
 * - 0:0 - Command byte.
 * - 1   - Zero terminated string pointer.
 */
#define P_OUTPUT_BUFFER_COMMAND_DEBUG_PRINT 0xF1

/*
 * Sets the digital input state.
 *
 * - 0:0   - Command byte
 * - 1:0 - Up state
 * - 1:1 - Down state
 * - 1:2 - Left state
 * - 1:3 - Right state
 */
#define P_INPUT_BUFFER_COMMAND_SET_DPAD 0x01

/*
 * Sets the analog input state.
 *
 * - 0:0 - Command byte
 * - 1   - Angle in radians
 */
#define P_INPUT_BUFFER_COMMAND_SET_STICK 0x02

/*
 * Sets the mouse state.
 *
 * - 0:0 - Command byte
 * - 0:1 - Left button state
 * - 0:2 - Right button state
 * - 1   - X Position
 * - 2   - Y Position
 */
#define P_INPUT_BUFFER_COMMAND_SET_MOUSE 0x03

/*
 * Signals a new canvas size.
 *
 * - 0:0   - Command byte
 * - 1:0-1 - Canvas width
 * - 1:2-3 - Canvas height
 */
#define P_INPUT_BUFFER_COMMAND_SET_CANVAS_SIZE 0x04

#define P_OUTPUT_BUFFER_LENGTH 0xFF
typedef struct pOutputBuffer {
    bU32 top;
    bU32 stack[P_OUTPUT_BUFFER_LENGTH];
} pOutputBuffer;

void cFrame(pStore *store, pOutputBuffer *outputBuffer, bF32 now);

#endif
