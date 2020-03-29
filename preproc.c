#define _CRT_SECURE_NO_WARNINGS 1

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define PI 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899f
#define TAU (PI*2)
#define SINE_STEPS 256

int main(void) {
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

    fclose(includeFile);

    return EXIT_SUCCESS;
}
