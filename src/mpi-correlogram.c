// Nicolas Cola
// 0001235951

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <mpi.h>

float *X = NULL;   
int nvalues = 0;
float *coef = NULL;    
int maxshifts = 2048;

int my_rank, comm_sz;

void read_signal(FILE *f)
{
    float tmp;
    int nread;

    assert(f != NULL);

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

void autocorrelate( void )
{
    /* 
    Mean and variance are computed redundantly by all processes.
    This is because they are O(n) computations and the overhead of
    distributing the data and collecting the results would be
    higher than the cost of computing them redundantly.
    */
    float mean = 0.0f;
    for (int i=0; i<nvalues; i++) {
        mean += X[i];
    }
    mean /= nvalues;

    float var = 0.0f;
    for (int i=0; i<nvalues; i++) {
        var += (X[i] - mean)*(X[i] - mean) / nvalues;
    }

    int *recvcnts = NULL;
    int *displs = NULL;

    /*
    Only the root process computes the displacements and counts for the MPI_Gatherv call.
    This is because the displacements and counts are only needed by the root process to gather the 
    results from all processes. The other processes do not need this information.
    */
    if (my_rank == 0) {
        recvcnts = (int*)malloc( comm_sz * sizeof(*recvcnts) );
        displs = (int*)malloc( comm_sz * sizeof(*displs) );
   
        for (int i = 0; i < comm_sz; i++) {
            int start = (maxshifts * i) / comm_sz;
            int end = (maxshifts * (i + 1)) / comm_sz;
            recvcnts[i] = end - start;
            displs[i] = start;
        }

        coef = (float*)malloc(maxshifts * sizeof(*coef));
        assert(coef != NULL);
    }
    
    /*
    Compute start index, end index and  maxshifts for each process
    in order to partition the outer loop.
    By doing so, maxshifts doesn't need to be divisible by comm_sz,
    ensuring load balancing among processes.
    */
    const int local_start = (maxshifts * my_rank) / comm_sz;
    const int local_end = (maxshifts * (my_rank + 1)) / comm_sz;
    const int local_maxshifts  = local_end - local_start;

    float *local_coef = (float*)malloc(local_maxshifts * sizeof(*local_coef));
    assert(local_coef != NULL);

    for (int h=local_start; h<local_end; h++) {
        float ac = 0.0f; 
        for (int i=0; i < nvalues; i++) {
            ac += (X[i] - mean)*(X[(i+h)%nvalues] - mean) / nvalues;
        }
        local_coef[h - local_start] = ac / var;
    }

    MPI_Gatherv(local_coef, local_maxshifts, MPI_FLOAT,
                coef, recvcnts, displs, MPI_FLOAT,
                0, MPI_COMM_WORLD);
                
    free(local_coef);
    free(recvcnts);
    free(displs);
}

void store( FILE* f )
{
    for (int h=0; h<maxshifts; h++) {
        fprintf(f, "%f\n", coef[h]);
    }
}

int main( int argc, char *argv[] )
{    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    FILE *inputf = NULL;
    FILE *outputf = NULL;
    const char* outputf_name = "out";

    if (argc < 2 || argc > 4) {
        if(my_rank == 0) fprintf(stderr, "Usage: %s input_file [output_file [max_shifts]]\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (argc > 3) {
        maxshifts = atoi(argv[3]);
    }

    if(my_rank == 0) {
        if (argc > 2) {
            outputf_name = argv[2];
        }

        if ((inputf = fopen(argv[1], "r")) == NULL) {
            fprintf(stderr, "FATAL: can not open \"%s\"\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        if ((outputf = fopen(outputf_name, "w")) == NULL) {
            fprintf(stderr, "FATAL: can not create \"%s\"", outputf_name);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        read_signal(inputf);
        fclose(inputf);
    }

    const double start = MPI_Wtime();
    MPI_Bcast(&nvalues, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank != 0) {
        X = (float*)malloc(nvalues * sizeof(*X));
    }

    MPI_Bcast(X, nvalues, MPI_FLOAT, 0, MPI_COMM_WORLD);

    autocorrelate();
    const double elapsed = MPI_Wtime() - start;
    if (my_rank == 0) {
	    printf("%.3f\n", elapsed);
        store(outputf);
        fclose(outputf);
        free(coef);
    }

    free(X);
    MPI_Finalize();
    return EXIT_SUCCESS;
}
