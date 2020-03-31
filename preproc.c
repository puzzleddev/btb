#define _CRT_SECURE_NO_WARNINGS 1

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899f
#define TAU (PI*2)
#define SINE_STEPS 256

typedef float objVertex[2]; 

static size_t objVertexStorageSize = 256;
static objVertex *objVertexStorage = NULL;
static size_t objVertexStorageTop = 0;

size_t objPushVertex(float x, float y) {
    if(objVertexStorageTop >= objVertexStorageSize) {
        objVertexStorageSize += 256;
        objVertexStorage = (objVertex *) realloc(objVertexStorage, sizeof(objVertex) * objVertexStorageSize);
    }

    objVertexStorage[objVertexStorageTop][0] = x;
    objVertexStorage[objVertexStorageTop++][1] = y;

    return objVertexStorageTop - 1;
}

#define PRINT_OBJ_LINE_SIZE 1024
char printObjLine[PRINT_OBJ_LINE_SIZE];
void printObj(FILE *out, const char *baseName, const char *inName) {
    FILE *in = fopen(inName, "r");

    objVertexStorageTop = 0;

    fprintf(out, "#define PREPROC_%s {", baseName);

    bool isFirst = true;
    (void)(isFirst);

    float xMin = 0;
    float xMax = 0;
    float yMin = 0;
    float yMax = 0;

    while(fgets(printObjLine, PRINT_OBJ_LINE_SIZE, in)) {
        if(printObjLine[0] == 'v') {
            float x = 0;
            float y = 0;

            char *start = printObjLine + 2;
            char *nextStart;
            x = strtof(start, &nextStart);
            strtof(nextStart, &start);
            y = strtof(start, NULL);

            if(x < xMin) {
                xMin = x;
            } else if(x > xMax) {
                xMax = x;
            }

            if(y < yMin) {
                yMin = y;
            } else if(y > yMax) {
                yMax = y;
            }

            objPushVertex(x, y);
        } else if(printObjLine[0] == 'f') {
            int a = 0;
            int b = 0;
            int c = 0;

            char *start = printObjLine + 2;
            char *nextStart;
            a = strtod(start, &nextStart) - 1;
            b = strtod(nextStart, &start) - 1;
            c = strtod(start, NULL) - 1;

            if(!isFirst) {
                fputc(',', out);
                fputc(' ', out);
            }
            isFirst = false;

            fprintf(
                out,
                "%f, %f, %f, %f, %f, %f",
                objVertexStorage[a][0], objVertexStorage[a][1],
                objVertexStorage[b][0], objVertexStorage[b][1],
                objVertexStorage[c][0], objVertexStorage[c][1]
            );
        }
    }

    fputc('}', out);
    fputc('\n', out);

    fprintf(
        out,
        "#define PREPROC_%s_XMIN %f\n"
        "#define PREPROC_%s_XMAX %f\n"
        "#define PREPROC_%s_YMIN %f\n"
        "#define PREPROC_%s_YMAX %f\n",

        baseName, xMin,
        baseName, xMax,
        baseName, yMin,
        baseName, yMax
    );

    fclose(in);
}

int main(void) {
    /* Setup */
    objVertexStorage = (objVertex *) malloc(sizeof(objVertex) * objVertexStorageSize);

    FILE *includeFile = fopen("_pp.c", "wb");

    /* Calculate sine table. */
    fprintf(includeFile, "#define PREPROC_PI %f\n", PI);
    fprintf(includeFile, "#define PREPROC_SINE_STEPS %d\n", SINE_STEPS);
    fprintf(includeFile, "#define PREPROC_SINE {");

    double sineStep = TAU / SINE_STEPS;

    for(int i = 0; i < SINE_STEPS; i++) {
        if(i != 0) {
            fputc(',', includeFile);
            fputc(' ', includeFile);
        }
        fprintf(includeFile, "%f", sin(sineStep * i));
    }

    fprintf(includeFile, "}\n");

    /* Load OBJ's */

    printObj(includeFile, "OBJ_TITLE", "../../data/title.obj");
    printObj(includeFile, "OBJ_TEXT_PLAY", "../../data/play.obj");
    printObj(includeFile, "OBJ_TEXT_PALETTE", "../../data/palette.obj");
    printObj(includeFile, "OBJ_SELECTOR", "../../data/selector.obj");
    printObj(includeFile, "OBJ_SPACEMAN", "../../data/proto_spaceman.obj");

    printObj(includeFile, "OBJ_HEX_0", "../../data/hex_0.obj");
    printObj(includeFile, "OBJ_HEX_1", "../../data/hex_1.obj");
    printObj(includeFile, "OBJ_HEX_2", "../../data/hex_2.obj");
    printObj(includeFile, "OBJ_HEX_3", "../../data/hex_3.obj");
    printObj(includeFile, "OBJ_HEX_4", "../../data/hex_4.obj");
    printObj(includeFile, "OBJ_HEX_5", "../../data/hex_5.obj");
    printObj(includeFile, "OBJ_HEX_6", "../../data/hex_6.obj");
    printObj(includeFile, "OBJ_HEX_7", "../../data/hex_7.obj");
    printObj(includeFile, "OBJ_HEX_8", "../../data/hex_8.obj");
    printObj(includeFile, "OBJ_HEX_9", "../../data/hex_9.obj");
    printObj(includeFile, "OBJ_HEX_A", "../../data/hex_A.obj");
    printObj(includeFile, "OBJ_HEX_B", "../../data/hex_B.obj");
    printObj(includeFile, "OBJ_HEX_C", "../../data/hex_C.obj");
    printObj(includeFile, "OBJ_HEX_D", "../../data/hex_D.obj");
    printObj(includeFile, "OBJ_HEX_E", "../../data/hex_E.obj");
    printObj(includeFile, "OBJ_HEX_F", "../../data/hex_F.obj");

    fclose(includeFile);

    return EXIT_SUCCESS;
}
