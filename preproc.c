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
void printObj(FILE *out, const char *inName) {
    FILE *in = fopen(inName, "r");

    objVertexStorageTop = 0;

    fputc('{', out);

    bool isFirst = true;
    (void)(isFirst);

    while(fgets(printObjLine, PRINT_OBJ_LINE_SIZE, in)) {
        if(printObjLine[0] == 'v') {
            float x = 0;
            float y = 0;
            float z = 0;

            char *start = printObjLine + 2;
            char *nextStart;
            x = strtof(start, &nextStart);
            y = strtof(nextStart, &start);
            z = strtof(start, NULL);

            (void)(x);
            (void)(y);
            (void)(z);
            objPushVertex(x, z);
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

    fprintf(includeFile, "#define PREPROC_OBJ_TITLE ");
    printObj(includeFile, "../../data/title.obj");
    fputc('\n', includeFile);

    fclose(includeFile);

    return EXIT_SUCCESS;
}
