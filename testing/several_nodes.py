#!/usr/bin/python2

import os, sys
import subprocess

nodes = [1, 4, 16, 64, 256]
size = [2048, 4096, 8192, 16384]
threads = [32, 64]
steps = 1000

sbatch = """#!/bin/bash
#SBATCH --job-name="ZombieApocalypse-juriad-{nodes}_nodes-{size}_size-{threads}_threads"
#SBATCH --nodes={nodes}
#SBATCH --time=1-0:0
export OMP_NUM_THREADS={threads}
srun --ntasks-per-node=1 ../../../apocalypse/apocalypse {size} {size} 2 {steps}
"""

for n in nodes:
    ndir = 'n-' + str(n)
    os.mkdir(ndir)
    os.chdir(ndir)
    for s in size:
        sdir = 's-' + str(s) + '-' + str(s)
        os.mkdir(sdir)
        os.chdir(sdir)
        for t in threads:
            tdir = 't-' + str(t)
            os.mkdir(tdir)
            os.chdir(tdir)
            
            os.mkdir('images')
            os.mkdir('output')
            
            f = open('apocalypse.sbatch','w')
            filled = sbatch.format(nodes=n, size=s, threads=t, steps=steps)
            f.write(filled)
            f.close()
            
            subprocess.call(['sbatch', './apocalypse.sbatch'])
            
            os.chdir('..')
        os.chdir('..')
    os.chdir('..')