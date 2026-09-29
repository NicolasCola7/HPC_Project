#!/bin/bash

#SBATCH -n 8
#SBATCH --time=0-00:15:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

echo "=== Compiling ==="
gcc -O2 -std=c99 -Wall -Wpedantic gen.c -o gen -lm
mpicc -O2 -std=c99 -Wall -Wpedantic mpi-correlogram.c -o mpi-correlogram -lm

echo "=== Generating Data (Once) ==="
./gen 2 1000000 data1M

# --- Setup the Output File ---
RESULTS_FILE="benchmark_results_mpi.txt"

header="Tasks"
for i in {1..20}; do header="${header}\tRun${i}"; done
echo -e "$header" > $RESULTS_FILE

echo "=== Starting Benchmarks ==="

CORES_PER_NODE=4
unset SLURM_NTASKS_PER_NODE

for tasks in $(seq 1 $SLURM_NTASKS)
do
    req_nodes=$(( (tasks + CORES_PER_NODE - 1) / CORES_PER_NODE ))

    echo "--- Testing with $tasks MPI tasks across $req_nodes node(s) ---"

    # NEW ADDITION: Print exactly which node each task is assigned to
    echo "Task Allocation map for $tasks tasks:"
    srun -N $req_nodes -n $tasks -l hostname

    # FIXED WARM-UP: Added the missing -N, -n, and --exact flags so the
    # warm-up run matches the actual benchmark runs perfectly.
    srun -N $req_nodes -n $tasks --exact --mpi=pmix ./mpi-correlogram data1M_noisy.txt out1M.txt 2048 > /dev/null 2>&1

    row="$tasks"

    for run in {1..20}
    do
        time_val=$(srun -N $req_nodes -n $tasks --exact --mpi=pmix ./mpi-correlogram data1M_noisy.txt out1M.txt 2048 2>/dev/null)
        row="${row}\t${time_val}"
    done

    echo -e "$row" >> $RESULTS_FILE
done

echo "=== End of Job ==="
tr '.' ',' < $RESULTS_FILE > excel_results_mpi.txt
echo "Done! Open 'excel_results_mpi.txt', copy all text, and paste directly into Excel."