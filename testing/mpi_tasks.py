#!/usr/bin/python2

import os, sys
import subprocess
import math

nodes = [1, 4, 16, 64, 256]
sizes = [2048, 4096, 8192, 16384]
threads = [32, 64]
steps = 1000

sbatch = """#!/bin/bash
#SBATCH --job-name="ZombieApocalypse-juriad-{nodes}_nodes-{size}_size-{threads}_threads"
#SBATCH --nodes={nodes}
# time is in minutes; see sbatch man page
#SBATCH --time={minutes}

export OMP_NUM_THREADS={threads}
srun --ntasks-per-node=1 ../../../apocalypse/apocalypse {size} {size} 2 {steps}
"""

def estimate(nodes, size, threads):
    return max(int(5 * (size / 2048) * (size / 2048) / nodes * max(math.log(nodes, 2), 1)), 10)

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
            filled = sbatch.format(nodes=n, size=s, threads=t, steps=steps, minutes=estimate(n, s, t))
            f.write(filled)
            f.close()
            
            subprocess.call(['sbatch', './apocalypse.sbatch'])
            
            os.chdir('..')
        os.chdir('..')
    os.chdir('..')