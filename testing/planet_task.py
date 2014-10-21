#!/usr/bin/python2

import os, sys
import subprocess

nodes = [64, 128, 256]
size1 = 8192
size2 =4096
threads = [32, 64]
steps = 1000

sbatch = """#!/bin/bash
#SBATCH --job-name="ZombieApocalypse-planet-juriad-{nodes}_nodes-{size1}-times-{size2}_size-{threads}_threads"
#SBATCH --nodes={nodes}
#SBATCH --time=0-0:10
export OMP_NUM_THREADS={threads}
srun --ntasks-per-node=1 ../../apocalypse/apocalypse {size1} {size2} 2 {steps}
"""

for n in nodes:
    ndir = 'n-' + str(n)
    os.mkdir(ndir)
    os.chdir(ndir)
    for t in threads:
        tdir = 't-' + str(t)
        os.mkdir(tdir)
        os.chdir(tdir)
        
        os.mkdir('images')
        os.mkdir('output')
        
        f = open('apocalypse.sbatch','w')
        filled = sbatch.format(nodes=n, size1=size1, size2=size2, threads=t, steps=steps)
        f.write(filled)
        f.close()
        
        subprocess.call(['sbatch', './apocalypse.sbatch'])
        
        os.chdir('..')
    os.chdir('..')