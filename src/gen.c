/****************************************************************************
 *
 * gen.c - Generate input for the correlogram program.
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
 * Generate a periodic signal with added random noise.
 *
 * Compile with:
 *
 *      gcc -std=c99 -Wall -Wpedantic gen.c -o gen -lm
 *
 * Execute with:
 *
 *      ./gen [nw [n [base_name]]]
 *
 * where nw = number of sine waves, n = number of data points,
 * base_name is the base name of the output files.
 *
 * The program generates two files containing n floating-point
 * numbers. The first is called <basename>_clean.txt and contains the
 * signal obtained by superimposing <nw> sinusoidal signals. The
 * second is called <basename>_noisy.txt, and contains the same signal
 * with added random noise.
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifndef PI
#define PI 3.14
#endif

double rand01( void )
{
    return rand() / (double)RAND_MAX;
}

double randab(double a, double b)
{
    return a + rand01() * (b-a);
}

int main( int argc, char *argv[] )
{
    int n = 1024*1024; // number of data points
    float freq[] = {213.0f, 87.0f, 2087.0f, 179.0f}; // frequencies
    float phi[]  = { 30.0f, 60.0f,  138.0f,  33.0f}; // phases
    const int NFREQ = sizeof(freq) / sizeof(freq[0]);
    int nw = 1; // number of sine waves to use
    const char *base_name = "signal";
    char fname_noisy[1024], fname_clean[1024];

    if (argc > 4) {
        fprintf(stderr, "Usage: %s [nw [n [base_name]]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc > 1) {
        nw = atoi(argv[1]);
    }

    if (argc > 2) {
        n = atoi(argv[2]);
    }

    if (argc > 3) {
        base_name = argv[3];
    }

    assert(nw > 0);
    assert(nw <= NFREQ);

    snprintf(fname_clean, sizeof(fname_clean), "%s_clean.txt", base_name);
    FILE *fclean = fopen(fname_clean, "w");
    if (fclean == NULL) {
        fprintf(stderr, "FATAL: can not create file %s\n", fname_clean);
        return EXIT_FAILURE;
    }

    snprintf(fname_noisy, sizeof(fname_noisy), "%s_noisy.txt", base_name);
    FILE *fnoisy = fopen(fname_noisy, "w");
    if (fnoisy == NULL) {
        fprintf(stderr, "FATAL: can not create file %s\n", fname_noisy);
        return EXIT_FAILURE;
    }

    for (int i=0; i<n; i++) {
        float signal = 0;
        for (int j=0; j<nw; j++) {
            signal += sin(2*PI*i / freq[j] + phi[j]);
        }
        fprintf(fclean, "%f\n", signal);
        signal += randab(-2*nw, 2*nw); // add some random noise
        fprintf(fnoisy, "%f\n", signal);
    }

    fclose(fclean);
    fclose(fnoisy);

    return EXIT_SUCCESS;
}
