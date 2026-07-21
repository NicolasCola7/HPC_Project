% Using the HPC cluster @ DISI
% [Moreno Marzolla](https://www.unibo.it/sitoweb/moreno.marzolla)
% Last updated: 2026-03-04

This page explains how to access and use the High Performance
Computing (HPC) cluster of the Department of Computer Science and
Engineering, University of Bologna.


# Getting access

To request access follow the instructions
[here](https://disi.unibo.it/it/dipartimento/servizi-tecnici-e-amministrativi/servizi-informatici/utilizzo-cluster-hpc). Specify
that you request access for lab sessions and final project of the
Parallel Computing course until the end of February 2027.


# Connecting to the cluster

To access the cluster, you connect to the frontend `giano.cs.unibo.it`
using `ssh`. The username is your full email address as username
(e.g., `maria.rossi@studio.unibo.it`), and the password is your
University password (the one you use to read the email).

To connect to the frontend from a Linux shell, you must protect the
`@` character of the username like this:

```bash
ssh maria.rossi\@studio.unibo.it@giano.cs.unibo.it
```

If you user the default ssh command-line client of Windows, do not put
the `\` character in front of the first `@`.


# Using the cluster

There are two partitions, _rtx2080_ and _l40_, whose features are
shown in Table 1. The number of available nodes may change, e.g., due
to scheduled maintenance or other circumstances.

: Table 1: Slurm partitions available on the HPC cluster.

----------------- ------------ ---------------------------------------
Partition name    l40          rtx2080
Nodes             4            10
CPU               AMD(kvm)     Intel(R) Xeon(R) W-2123 CPU @ 3.60GHz
Cores per node    8            4
RAM               64GB         44GB
GPU               NVidia L40   GeForce RTX 2080 Ti
GPU cores         18176        4352
GPU memory        45GB         10GB
----------------- ------------ ---------------------------------------


You can select the partition using the `--partition=xxx` directive in
the Slurm scripts (see below); the default partition is _rtx2080_.

To find out more about the execution nodes, you can use `srun` to
execute a command such as `cat /proc/meminfo` or `lscpu`. Note that
`srun` waits for an execution slot to become available, so you may
have to wait.

```bash
srun -p l40 cat /proc/meminfo
srun -p l40 lscpu
srun -p l40 lspci
```

If you want to execute a program that accesses the GPU, you need to
add the `--gres=gpu:1` flag:

```bash
srun -p rtx2080 --gres=gpu:1 ./deviceQuery
```


## Basic Slurm commands

The HPC cluster uses the ubiquitous
[Slurm](https://www.schedmd.com/slurm/) scheduler. Slurm handles
queues of jobs that are executed on the cluster when resources become
available. The basic Slurm commands are:

`squeue`
: List all the jobs in the queue.

`sbatch <script.sh>`
: Submit a job.

`scancel <job id>`
: Cancel a job, either a running one or a job still in the queue.

`sinfo`
: Get information on the partitions, such as the number of nodes available.

`srun -p <partition> <command>`
: Run `<command>` on partition `<partition>`, e.g., `srun -p l40 lscpu`.


## Running OpenMP jobs

Suppose you want to run [this OpenMP program](omp-program.c):

```C
// omp-program.c
#include <stdio.h>
#include <omp.h>

int main( void )
{
#pragma omp parallel
    printf( "Hello from core %d of %d\n",
            omp_get_thread_num(), omp_get_num_threads() );
    return 0;
}
```

First, create a Slurm script [run-omp-program.sh](run-omp-program.sh)
with the following content:

```bash
#!/bin/bash
# run-omp-program.sh
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task 4
#SBATCH --time=0-00:05:00
#SBATCH --output slurm-%j.out
#SBATCH --partition=rtx2080

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "=== Compiling ==="
gcc -std=c99 -Wall -Wpedantic -fopenmp omp-program.c -o omp-program

echo "=== Running with $OMP_NUM_THREADS threads ==="
./omp-program

echo "=== End of Job ==="
```

The script will first compile the program, since no compiler is
installed on the frontend, and then executes it with the
`OMP_NUM_THREADS` variable set to the number of OpenMP threads to use.

The `--time=0-00:05:00` option sets a limit of 5 minutes on the
execution time. This parameter can be changed or omitted; in this case
the limit is the one defined by the Slurm administrator. It is always
a good idea to set a time limit to avoid consuming resources if a
misbehaving program enters an infinite loop.

The `--partition=rtx2080` option selects the _rtx2080_ partition,
whose nodes have four cores per CPU. If you select _l40_, you may
request uyp to 8 cores per task.

The `--cpus-per-task 4` option requests Slurm to allocate four cores,
which is the maximum for the _rtx2080_ partition.

Then, submit the job using `sbatch`:

```bash
sbatch run-omp-program.sh
```

You can select a different number of cores using the `-c N` parameter
of `sbatch`; this will override the value specified in
`--cpus-per-task`. For example, to use two cores:

```bash
sbatch -c 2 run-omp-program.sh
```

To monitor the state of the job, use the `squeue` command. To remove
the job from the queue or abort its execution, use `scancel <jobid>`.

When the job completes, the output is saved to a file
`slurm-<jobid>.out`.


# Running CUDA jobs

Suppose you want to run [this CUDA program](cuda-program.cu):

```C
// cuda-program.cu
#include <stdio.h>

__global__ void mykernel(void) { }

int main(void)
{
	mykernel<<<1,1>>>( );
	printf("Hello World!\n");
	return 0;
}
```

Similarly to OpenMP, you create a
[run-cuda-program.sh](run-cuda-program.sh) script:

```bash
#!/bin/bash
# run-cuda-program.sh
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --gres=gpu:1
#SBATCH --time=0-00:05:00
#SBATCH --output slurm-%j.out

echo "=== Compiling ==="
nvcc cuda-program.cu -o cuda-program

echo "=== CUDA program starts ==="
./cuda-program

echo "=== End of Job ==="
```

The script first compiles the program, and then executes it. The
`--gres=gpu:1` asks Slurm to allocate one GPU.

Job submission and management is the same as the OpenMP case.


# Running MPI jobs

Suppose you want to run [this MPI program](mpi-program.c):

```C
// mpi-program.c
#include <stdio.h>
#include <mpi.h>

int main( int argc, char *argv[] )
{
	int rank, size, len;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank);
	MPI_Comm_size( MPI_COMM_WORLD, &size);
	MPI_Get_processor_name( hostname, &len );
	printf( "Hello from processor %d of %d running on %s\n",
		rank, size, hostname);
	MPI_Finalize();
	return 0;
}
```

This is the [run-mpi-program.sh](run-mpi-program.sh) script:

```bash
#!/bin/bash
# run-mpi-program.sh
#SBATCH -n 4
#SBATCH --output slurm-%j.out
#SBATCH --time=0-00:05:00
#SBATCH --partition=rtx2080

echo "=== Compiling ==="
mpicc -std=c99 -Wall -Wpedantic mpi-program.c -o mpi-program

echo "== Running with $SLURM_NTASKS tasks =="
srun --mpi=pmix ./mpi-program

echo "== End of Job =="
```

The `-n 4` option requests four cores, and can be overridden using the
`-n` option of `sbatch`:

```bash
sbatch -n 8 run-mpi-program.sh
```

In the example above, 8 cores are required, so Slurm will allocate two
nodes with 4 cores each on the _rtx2080_ partition.

> *Note*: The cluster is configured to allow a maximum of two nodes (8
> cores) per MPI job, and a maximum of three MPI jobs per user.

