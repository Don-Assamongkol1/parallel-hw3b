#!/bin/bash
#
#SBATCH --mail-user=dassamongkol@cs.uchicago.edu
#SBATCH --mail-type=ALL
#SBATCH --output=/home/dassamongkol/slurm/HW3b/slurm_out/%j.%N.stdout
#SBATCH --error=/home/dassamongkol/slurm/HW3b/slurm_out/%j.%N.stderr
#SBATCH --chdir=/home/dassamongkol/slurm/HW3b/parallel-hw3b
#SBATCH --job-name=dassamongkol-hw-3b
#SBATCH --nodes=1 
#SBATCH --ntasks=1
#SBATCH --time=55:00
#SBATCH --partition=general
#SBATCH --exclusive

python3 ./tests/idle_overhead_test.py