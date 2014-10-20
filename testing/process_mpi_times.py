import os
from os.path import join, getsize
import fileinput
import sys
import math
import re

for dirpath, dirs, files in os.walk('.'):
    if dirpath.endswith('output'):
        m = re.search('n-(\d+)/s-(\d+)-\d+/t-(\d+)', dirpath)
        nodes = m.group(1);
        size = m.group(2);
        threads = m.group(3);
        
        borders = []
        ghosts = []
        times = []
        for f in files:
            if f.endswith(".err"):
                for line in fileinput.input(os.path.join(dirpath, f)):
                    if line.startswith("DEBUG: Waited for borders for "):
                        m = re.search('([0-9.]+) milliseconds', line)
                        borders.append(float(m.group(1)))
                    elif line.startswith("DEBUG: Waited for ghosts for "):
                        m = re.search('([0-9.]+) milliseconds', line)
                        ghosts.append(float(m.group(1)))
                    elif line.startswith("TIME: Simulation took"):
                        m = re.search('([0-9.]+) milliseconds', line)
                        times.append(float(m.group(1)))
        border = sum(borders) / len(borders)
        ghost = sum(ghosts) / len(ghosts)
        time = sum(times) / len(times)/1000
        print("{nodes}\t{size}\t{threads}\t{time:.3f}\t{border:.3f}\t{ghost:.3f}\t{waiting:.3f}"
              .format(nodes=nodes, size=size, threads=threads, border=border, ghost=ghost, time=time, waiting=(border+ghost)*100/time))