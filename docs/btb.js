"use strict";

var WASM_PATH = "btb.wasm";

var ERROR = document.getElementById("ee");
var SUPPORT = document.getElementById("es");
var CANVAS = document.getElementById("a");

function e(str) {
    CANVAS.style.display = "none";
    ERROR.style.display = "";
    throw(str);
}

var P_PALETTE_SIZE = 0x10;
var P_VERTEX_SIZE = 0x0C;

var P_OUTPUT_BUFFER_COMMAND_SET_PALETTE = 0x01;
var P_OUTPUT_BUFFER_COMMAND_CLEAR_COLOR = 0x02;
var P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH = 0x03;
var P_OUTPUT_BUFFER_COMMAND_RENDER_MESH = 0x04;
var P_OUTPUT_BUFFER_COMMAND_UPDATE_MESH = 0x05;
var P_OUTPUT_BUFFER_COMMAND_DEBUG_THROW = 0xF0;
var P_OUTPUT_BUFFER_COMMAND_DEBUG_PRINT = 0xF1;
var P_INPUT_BUFFER_COMMAND_SET_DPAD = 0x01;
var P_INPUT_BUFFER_COMMAND_SET_CANVAS_SIZE = 0x04;

var VERT_SHADER = [
    "uniform vec4 palette[0x10];",
    "",
    "attribute vec2 vPosition;",
    "attribute float vPaletteIndex;",
    "",
    "varying vec4 fColor;",
    "",
    "void main() {",
    "  gl_Position = vec4(vPosition, 0, 1);",
    //"  fColor = vec4(vPosition, 0, 0);",
    "  fColor = palette[int(vPaletteIndex)];",
    "}"
].join("\n");
var FRAG_SHADER = [
    "precision mediump float;",
    "",
    "varying vec4 fColor;",
    "",
    "void main() {",
    "  gl_FragColor = fColor;",
    "}"
].join("\n");

function createShader(gl, vertSource, fragSource) {
    var prog = gl.createProgram();

    var vert = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vert, vertSource);
    gl.compileShader(vert);
    if(!gl.getShaderParameter(vert, gl.COMPILE_STATUS)) {
        e(gl.getShaderInfoLog(vert));
    }

    var frag = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(frag, fragSource);
    gl.compileShader(frag);
    if(!gl.getShaderParameter(frag, gl.COMPILE_STATUS)) {
        e(gl.getShaderInfoLog(frag));
    }

    gl.attachShader(prog, vert);
    gl.attachShader(prog, frag);
    gl.linkProgram(prog);
    gl.deleteShader(vert);
    gl.deleteShader(frag);

    if(!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
        e(gl.getProgramInfoLog(prog));
    }

    return prog;
}

function main() {
    var supported = true;

    if(!("WebAssembly" in window)) {
        supported = false;
    } else if(!("fetch" in window)) {
        supported = false;
    }

    window["gl"] =
        CANVAS.getContext("webgl") ||
        CANVAS.getContext("webgl-experimental");

    if(!gl) {
        support = false;
    }

    if(!supported) {
        CANVAS.style.display = "none";
        SUPPORT.style.display = "";
        throw "Unsupported Device.";
    }

    window["wasm"] = null;

    fetch(WASM_PATH)
        .then(function(res) {
            if(!res.ok) {
                e("Could not fetch '" + WASM_PATH + "'.");
            }

            return res.arrayBuffer();
        })
        .then(function(buffer) {
            return WebAssembly.instantiate(buffer, {});
        })
        .then(function(module) {
            wasm = module;
            requestAnimationFrame(frame);
        });


    /* WebGL initialization to overlap with the fetch. */

    /* Create shaders: */
    var paletteShader = createShader(gl, VERT_SHADER, FRAG_SHADER);

    window["glShaders"] = {
        palette: {
            id: paletteShader,
            aPosition: gl.getAttribLocation(paletteShader, "vPosition"),
            aPaletteIndex: gl.getAttribLocation(paletteShader, "vPaletteIndex"),
            uPalette: gl.getUniformLocation(paletteShader, "palette"),
        },
    };

    window.addEventListener("keydown", function(e) {
        if(e.key == "w" || e.key == "ArrowUp") {
            keys.up = true;
        } else if(e.key == "s" || e.key == "ArrowDown") {
            keys.down = true;
        } else if(e.key == "a" || e.key == "ArrowLeft") {
            keys.left = true;
        } else if(e.key == "d" || e.key == "ArrowRight") {
            keys.right = true;
        }
    });
    window.addEventListener("keyup", function(e) {
        if(e.key == "w" || e.key == "ArrowUp") {
            keys.up = false;
        } else if(e.key == "s" || e.key == "ArrowDown") {
            keys.down = false;
        } else if(e.key == "a" || e.key == "ArrowLeft") {
            keys.left = false;
        } else if(e.key == "d" || e.key == "ArrowRight") {
            keys.right = false;
        }
    });
}

var lastFrameTime = 0;
var firstFrame = true;
var pointerTable = null;
var palette = new Float32Array(P_PALETTE_SIZE * 4);
var glSlots = Array(0x10000);

var keys = {
    up: false,
    down: false,
    left: false,
    right: false,
};
var lastKeysWord = 0;

var lastCanvasWidth = 0;
var lastCanvasHeight = 0;

function frame() {
    var now = performance.now();

    var m32 = null;
    var ioBufferPointer = null;
    var ioBufferBase = null;
    var ioBufferTop = null;
    var currentTop = null;

    /*
     * Don't generate input events on the first frame, since we don't know
     * where the IO buffer is yet.
     */
    if(!firstFrame) {
        m32 = new Uint32Array(wasm.instance.exports.memory.buffer);
        ioBufferPointer = m32[pointerTable]/4;
        ioBufferBase = ioBufferPointer + 1;
        ioBufferTop = 0;

        var keysWord = 
            ((keys.up ? 0xFF: 0x00) << 0) |
            ((keys.down ? 0xFF: 0x00) << 8) |
            ((keys.left ? 0xFF: 0x00) << 16) |
            ((keys.right ? 0xFF: 0x00) << 24);

        if(keysWord !== lastKeysWord) {
            m32[ioBufferBase + ioBufferTop++] = P_INPUT_BUFFER_COMMAND_SET_DPAD;
            m32[ioBufferBase + ioBufferTop++] = keysWord;
            lastKeysWord = keysWord;
        }

        var newCanvasWidth = CANVAS.clientWidth;
        var newCanvasHeight = CANVAS.clientHeight;

        if(
            newCanvasWidth != lastCanvasWidth ||
            newCanvasHeight != lastCanvasHeight
        ) {
            CANVAS.width = newCanvasWidth;
            CANVAS.height = newCanvasHeight;

            if(newCanvasWidth > 0xFFFF || newCanvasHeight > 0xFFFF) {
                e("Canvas width or height exceeded FFFF.");
            }

            m32[ioBufferBase + ioBufferTop++] = P_INPUT_BUFFER_COMMAND_SET_CANVAS_SIZE;
            m32[ioBufferBase + ioBufferTop++] = 
                ((newCanvasWidth & 0xFFFF) >> 0) |
                ((newCanvasHeight & 0xFFFF) >> 16);

            lastCanvasWidth = newCanvasWidth;
            lastCanvasHeight = newCanvasHeight;
        }

        m32[ioBufferPointer] = ioBufferTop;
    }

    pointerTable = wasm.instance.exports.pHtml5Frame(now) / 4;

    if(pointerTable % 1 != 0) {
        e("Unaligned pointer table: 0x" + (pointerTable * 4).toString(16).toUpperCase());
    }

    gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);

    gl.useProgram(glShaders.palette.id);
    gl.enableVertexAttribArray(glShaders.palette.aPosition);
    gl.enableVertexAttribArray(glShaders.palette.aPaletteIndex);

    m32 = new Uint32Array(wasm.instance.exports.memory.buffer);
    ioBufferPointer = m32[pointerTable]/4;
    ioBufferBase = ioBufferPointer + 1;
    ioBufferTop = m32[ioBufferPointer];
    currentTop = 0;

    while(ioBufferTop > currentTop) {
        var c = m32[ioBufferBase + currentTop++];

        switch(c & 0xFF) {
            case P_OUTPUT_BUFFER_COMMAND_SET_PALETTE: {
                var pointer = m32[ioBufferBase + currentTop++] / 4;

                if(pointer % 1 != 0) {
                    e("Unaligned palette pointer.");
                }

                for(var i = 0; i < P_PALETTE_SIZE; i++) {
                    var color = m32[pointer + i];

                    palette[i*4 + 0] = ((color >> 0) & 0xFF) / 0xFF;
                    palette[i*4 + 1] = ((color >> 8) & 0xFF) / 0xFF;
                    palette[i*4 + 2] = ((color >> 16) & 0xFF) / 0xFF;
                    palette[i*4 + 3] = ((color >> 24) & 0xFF) / 0xFF;
                }

                break;
            };
            case P_OUTPUT_BUFFER_COMMAND_CLEAR_COLOR: {
                var index = (c >> 8) & 0x0F;

                gl.clearColor(
                    palette[index*4 + 0],
                    palette[index*4 + 1],
                    palette[index*4 + 2],
                    palette[index*4 + 3]
                );
                gl.clear(gl.COLOR_BUFFER_BIT);
                break;
            };
            case P_OUTPUT_BUFFER_COMMAND_UPDATE_MESH:
            case P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH: {
                var index = (c >> 16) & 0xFFFF;
                var vertexNumber = m32[ioBufferBase + currentTop++];
                var meshPointer = m32[ioBufferBase + currentTop++];

                var buffer = null;

                if((c & 0xFF) == P_OUTPUT_BUFFER_COMMAND_UPLOAD_MESH) {
                    if(glSlots[index]) {
                        e("Attempting to upload buffer into filled slot.");
                    }
                    buffer = gl.createBuffer();
                } else {
                    if(!glSlots[index]) {
                        e("Attempting to update into unfilled slot.");
                    }
                    buffer = glSlots[index]
                }
                
                gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
                gl.bufferData(
                    gl.ARRAY_BUFFER,
                    new Float32Array(
                        wasm.instance.exports.memory.buffer,
                        meshPointer,
                        (vertexNumber * P_VERTEX_SIZE) / 4
                    ),
                    gl.DYNAMIC_DRAW
                );
                glSlots[index] = buffer;

                break;
            };
            case P_OUTPUT_BUFFER_COMMAND_RENDER_MESH: {
                var index = (c >> 16) & 0xFFFF;
                var startIndex = m32[ioBufferBase + currentTop++];
                var length = m32[ioBufferBase + currentTop++];

                if(!glSlots[index]) {
                    e("Attempting to render an unfilled slot.");
                }

                var buffer = glSlots[index];

                gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
                gl.vertexAttribPointer(
                    glShaders.palette.aPosition,
                    2,
                    gl.FLOAT,
                    false,
                    0xC,
                    0
                );
                gl.vertexAttribPointer(
                    glShaders.palette.aPaletteIndex,
                    1,
                    gl.FLOAT,
                    false,
                    0xC,
                    0x8
                );
                gl.uniform4fv(glShaders.palette.uPalette, palette);

                gl.drawArrays(gl.TRIANGLES, startIndex, length);

                break;
            };
            case P_OUTPUT_BUFFER_COMMAND_DEBUG_THROW:
            case P_OUTPUT_BUFFER_COMMAND_DEBUG_PRINT: {
                var messagePointer = m32[ioBufferBase + currentTop++];

                var m8 = new Uint8Array(wasm.instance.exports.memory.buffer);
                var message = "";

                for(var p = messagePointer; m8[p] != 0; p++) {
                    message += String.fromCharCode(m8[p]);
                }

                if((c & 0xFF) == P_OUTPUT_BUFFER_COMMAND_DEBUG_THROW) {
                    e(message);
                } else {
                    console.log(message);
                }

                break;
            };
            default:
                e("Unknown command 0x" + (c & 0xFF).toString(16));
        }
    }

    if(ioBufferTop !== currentTop) {
        e("IO buffer desynchronization.");
    }

    requestAnimationFrame(frame);

    firstFrame = false;
}

window.onload = main;
