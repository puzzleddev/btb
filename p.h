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

/* @TODO: Push packed attribute into b.h */
typedef struct __attribute__((packed)) pVertex {
    bF32 x;
    bF32 y;
    bF32 p;
} pVertex;

/* @TODO: Push packed attribute into b.h */
typedef struct __attribute__((packed)) pVertexMono {
    bF32 x;
    bF32 y;
} pVertexMono;

/* @TODO: Push packed attribute into b.h */
typedef bF32 pTransformation[16];
#define P_TRANSFORMATION_IDENTITY (pTransformation) { \
    1, 0, 0, 0, \
    0, 1, 0, 0, \
    0, 0, 1, 0, \
    0, 0, 0, 1  \
}

#define P_OUTPUT_BUFFER_TYPE_VERTEX_PALETTE 0x01
#define P_OUTPUT_BUFFER_TYPE_VERTEX_MONO 0x02

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
 * Uploads a mesh and binds it to an index.
 *
 * - 0:0   - Command byte
 * - 0:1   - Type
 * - 0:2-3 - Index
 * - 1     - Number of vertices.
 * - 2     - Vertex buffer pointer
 */
#define P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH 0x03

/*
 * Renders a mesh with the current uniforms.
 *
 * - 0:0   - Command byte
 * - 0:1   - Type
 * - 0:2-3 - Index
 * - 1     - Start index
 * - 2:0-1 - Vertices to draw
 * - 2:2-3 - Alpha
 * - 3     - Transformation pointer
 */
#define P_OUTPUT_BUFFER_COMMAND_RENDER_MESH 0x04

/*
 * Updates a mesh.
 *
 * - 0:0   - Command byte
 * - 0:1   - Type
 * - 0:2-3 - Index
 * - 1     - Number of vertices
 * - 2     - Vertex buffer pointer
 */
#define P_OUTPUT_BUFFER_COMMAND_UPDATE_MESH 0x05

/*
 * Deletes a mesh.
 *
 * - 0:0   - Command byte
 * - 0:1   - Type
 * - 0:2-3 - Index
 */
#define P_OUTPUT_BUFFER_COMMAND_DELETE_MESH 0x06

/*
 * Sets the global transformation.
 *
 * - 0:0 - Command byte
 * - 1   - Transformation pointer.
 */
#define P_OUTPUT_BUFFER_COMMAND_SET_TRANSFORMATION 0x07

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
 * Prints a debug float.
 *
 * - 0:0 - Command byte.
 * - 1   - A 32 bit float.
 */
#define P_OUTPUT_BUFFER_COMMAND_DEBUG_FLOAT 0xF2

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

#define P_INPUT_BUFFER_COMMAND_SET_TOUCH 0x04

/*
 * Signals a new canvas size.
 *
 * - 0:0   - Command byte
 * - 1:0-1 - Canvas width
 * - 1:2-3 - Canvas height
 */
#define P_INPUT_BUFFER_COMMAND_SET_CANVAS_SIZE 0x05

#define P_OUTPUT_BUFFER_LENGTH 0xFF
typedef struct pOutputBuffer {
    bU32 top;
    bU32 stack[P_OUTPUT_BUFFER_LENGTH];
} pOutputBuffer;

void cFrame(pStore *store, pOutputBuffer *outputBuffer, bF32 now);

#endif
