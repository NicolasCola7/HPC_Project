#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4      # <-- Set this to your node's maximum cores
#SBATCH --time=0-00:10:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

echo "=== Compiling ==="
gcc -O2 -std=c99 -Wall -Wpedantic gen.c -o gen -lm
gcc -O2 -std=c99 -Wall -Wpedantic -fopenmp omp-correlogram.c -o omp-correlogram

echo "=== Generating Data (Once) ==="
./gen 2 1000000 data1M

# --- Setup the Output File ---
RESULTS_FILE="benchmark_results.txt"

# Create the header row (Cores, Run1, Run2 ... Run20) separated by tabs
header="Cores"
for i in {1..20}; do header="${header}\tRun${i}"; done
echo -e "$header" > $RESULTS_FILE

echo "=== Starting Benchmarks ==="

# Outer loop: Number of CPUs
for cpus in $(seq 1 $SLURM_CPUS_PER_TASK)
do
    export OMP_NUM_THREADS=$cpus
    echo "--- Testing with $OMP_NUM_THREADS threads ---"

    # Warm-up run (discard output)
    ./omp-correlogram data1M_noisy.txt out1M.txt 2048 > /dev/null

    # Start building the row string, beginning with the number of cores
    row="$cpus"

    # Inner loop: 20 repetitions
    for run in {1..20}
    do
        # Directly capture the raw execution time printed by your program
        time_val=$(./omp-correlogram data1M_noisy.txt out1M.txt 2048)

        # Append the execution time to the row string, separated by a tab (\t)
        row="${row}\t${time_val}"
    done

    # Append the fully constructed row to our results file
    echo -e "$row" >> $RESULTS_FILE

done

echo "=== End of Job ==="
tr '.' ',' < $RESULTS_FILE > excel_results_omp.txt
echo "Done! Open 'excel_results_omp.txt', copy all text, and paste directly into Excel."