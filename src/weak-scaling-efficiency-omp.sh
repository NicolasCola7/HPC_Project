#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4       # Max 4 cores for OpenMP
#SBATCH --time=0-00:10:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

echo "=== Compiling ==="
gcc -O2 -std=c99 -Wall -Wpedantic gen.c -o gen -lm
gcc -O2 -std=c99 -Wall -Wpedantic -fopenmp omp-correlogram.c -o omp-correlogram

RESULTS_FILE="weak_scaling_omp.txt"
header="Cores"
for i in {1..20}; do header="${header}\tRun${i}"; done
echo -e "$header" > $RESULTS_FILE

echo "=== Generating Data (Once) ==="
./gen 2 1000000 data1M

echo "=== Starting OpenMP Weak Scaling Benchmarks ==="

BASE_M=2048

for cpus in $(seq 1 $SLURM_CPUS_PER_TASK)
do
    export OMP_NUM_THREADS=$cpus
    
    scaled_M=$(( BASE_M * cpus ))
    
    echo "--- Testing with $cpus threads, N=1000000, maxshifts=$scaled_M ---"

    row="${cpus}"
    for run in {1..20}
    do
        time_val=$(./omp-correlogram data1M_noisy.txt out1M.txt $scaled_M)
        row="${row}\t${time_val}"
    done
    echo -e "$row" >> $RESULTS_FILE

done

echo "=== End of Job ==="
tr '.' ',' < $RESULTS_FILE > excel_weak_omp.txt
echo "Done! Open 'excel_weak_omp.txt' for Italian Excel."