#!/usr/bin/python2

import os, sys
import subprocess

threads = [1, 2, 4, 8, 16, 32, 64]
size = [128, 256, 512, 1024, 2048, 4096, 8192, 16384]
steps = 1000

sbatch = """#!/bin/bash
#SBATCH --job-name="ZombieApocalypse-juriad-{size}_size-{threads}_threads"
#SBATCH --nodes=1
#SBATCH --time=1-0:0
export OMP_NUM_THREADS={threads}
srun --ntasks-per-node=1 ../../apocalypse/apocalypse {size} {size} 2 {steps}
"""

for s in size:
    dir = '1-' + str(s) + '-' + str(s)
    os.mkdir(dir)
    os.chdir(dir)
    for t in threads:
        dir2 = 't-' + str(t)
        os.mkdir(dir2)
        os.chdir(dir2)
        
        os.mkdir('images')
        os.mkdir('output')
        
        f = open('apocalypse.sbatch','w')
        filled = sbatch.format(size=s, threads=t, steps=steps)
        f.write(filled)
        f.close()
        
        subprocess.call(['sbatch', './apocalypse.sbatch'])
        
        os.chdir('..')
    os.chdir('..')