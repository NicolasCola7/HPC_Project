/****************************************************************************
 *
 * correlogram.c - Compute the correlogram of a time series
 *
 * Copyright (C) 2026 Moreno Marzolla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------------
 *
 * Compile with:
 *
 * gcc -std=c99 -Wall -Wpedantic correlogram.c -o correlogram
 *
 * Execute with:
 *
 * ./correlogram input [output [maxshifts]]
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

float *X = NULL;   /* (array of `nvalues` elements). */
int nvalues = 0;
float *coef = NULL;     /* (array of `maxfhits` elements); `coef[j]`
                           is the lag-h autocorrelation
                           coefficient. */
int maxshifts = 2048;

/**
 * Read X data from file `f`.
 */
void read_signal(FILE *f)
{
    float tmp;
    int nread;

    assert(f != NULL);

    /* Count the number of records. */
    nvalues = 0;
    while (fscanf(f, "%f", &tmp) == 1) {
        nvalues++;
    }
    rewind(f);
    X = (float*)malloc(nvalues * sizeof(*X));
    assert(X != NULL);

    for (int i=0; i < nvalues; i++) {
        nread = fscanf(f, "%f", &X[i]);
        assert(nread == 1);
    }
}

/**
 * Compute the lag-s autocorrelation.
 */
void autocorrelate( void )
{
    coef = (float*)malloc(maxshifts * sizeof(*coef));
    assert(coef != NULL);

    /* Compute the mean. */
    float mean = 0.0f;
    for (int i=0; i<nvalues; i++) {
        mean += X[i];
    }
    mean /= nvalues;

    /* Compute the variance. */
    float var = 0.0f;
    for (int i=0; i<nvalues; i++) {
        var += (X[i] - mean)*(X[i] - mean) / nvalues;
    }

    /* Compute the lag-h autocorrelation. */
    for (int h=0; h<maxshifts; h++) {
        float ac = 0.0f; /* autocovariance */
        for (int i=0; i < nvalues; i++) {
            ac += (X[i] - mean)*(X[(i+h)%nvalues] - mean) / nvalues;
        }
        coef[h] = ac / var;
    }
}

/**
 * Store the lag-h autocorrelation coefficients into file `f`.
 */
void store( FILE* f )
{
    for (int h=0; h<maxshifts; h++) {
        fprintf(f, "%f\n", coef[h]);
    }
}

int main( int argc, char *argv[] )
{
    FILE *inputf;
    FILE *outputf;
    const char* outputf_name = "out";

    if (argc < 2 || argc > 4) {
        fprintf(stderr, "Usage: %s input_file [output_file [max_shifts]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 2) {
        outputf_name = argv[2];
    }

    if (argc > 3) {
        maxshifts = atoi(argv[3]);
    }

    if ((inputf = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "FATAL: can not open \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }

    if ((outputf = fopen(outputf_name, "w")) == NULL) {
        fprintf(stderr, "FATAL: can not create \"%s\"", outputf_name);
        return EXIT_FAILURE;
    }

    read_signal(inputf);
    autocorrelate();
    store(outputf);

    fclose(inputf);
    fclose(outputf);
    free(X);
    free(coef);
    return EXIT_SUCCESS;
}
