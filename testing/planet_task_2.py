#!/usr/bin/python2

# to be run in a subdirectory of the root directory of the project

import os, sys
import subprocess

nodes = 128
size1 = 8192
size2 =4096
threads = 32
steps = 1000000

sbatch = """#!/bin/bash
#SBATCH --job-name="ZombieApocalypse-planet-juriad-{nodes}_nodes-{size1}-times-{size2}_size-{threads}_threads"
#SBATCH --nodes={nodes}
#SBATCH --time=1-0:0
export OMP_NUM_THREADS={threads}
srun --ntasks-per-node=1 ../apocalypse/apocalypse {size1} {size2} 2 {steps}
"""

os.mkdir('images')
os.mkdir('output')

f = open('apocalypse.sbatch','w')
filled = sbatch.format(nodes=nodes, size1=size1, size2=size2, threads=threads, steps=steps)
f.write(filled)
f.close()

subprocess.call(['sbatch', './apocalypse.sbatch'])
