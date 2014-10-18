#!/usr/bin/python

import fileinput
import sys
import math

def toSeconds(time):
    (h, m, s) = time.split(':')
    return int(h) * 3600 + int(m) * 60 + math.floor(float(s))

for line in fileinput.input('-'):
    (nodes, size, threads, end, start) = line.split()
    print("{nodes}\t{size}\t{threads}\t{time}"
          .format(nodes=nodes, size=size, threads=threads,
                  time=toSeconds(end) - toSeconds(start)))
