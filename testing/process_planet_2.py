#!/usr/bin/python

import os
from os.path import join, getsize
import fileinput
import sys
import math
import re

for dirpath, dirs, files in os.walk('.'):
    if dirpath.endswith('output'):
        nodes = 128;
        threads = 32;
        
        borders = []
        ghosts = []
        for f in files:
            if f.endswith(".err"):
                for line in fileinput.input(os.path.join(dirpath, f)):
                    try:
                        if line.startswith("DEBUG: Waited for borders for "):
                            m = re.search('([0-9.]+) milliseconds', line)
                            borders.append(float(m.group(1)))
                        elif line.startswith("DEBUG: Waited for ghosts for "):
                            m = re.search('([0-9.]+) milliseconds', line)
                            ghosts.append(float(m.group(1)))
                    except:
                        pass
        border = sum(borders) / len(borders)
        ghost = sum(ghosts) / len(ghosts)
        print("{nodes}\t{size}\t{threads}\t{time:.3f}\t{border:.3f}\t{ghost:.3f}"
              .format(nodes=nodes, size="8192x4096", threads=threads, border=border, ghost=ghost, time=7200))