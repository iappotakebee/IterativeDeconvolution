#!/bin/sh 
#PBS -q u-lecture 
#PBS -N test
#PBS -W group_list=gt15
#PBS -l select=1:mpiprocs=1:ompthreads=18
#PBS -l walltime=00:10:00
#PBS -o dcvl_omp_143000_18pe.lst
#PBS -e dcvl_omp_143000_18pe.err

cd $PBS_O_WORKDIR
. /etc/profile.d/modules.sh

#env OMP_NUM_THREADS=4 ./a.out | tee a.log
./a.out
