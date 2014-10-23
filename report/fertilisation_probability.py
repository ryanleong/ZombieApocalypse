#!/usr/bin/python

import math
import sys

from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
import matplotlib.pyplot as plt

MAX_POP = 21250
SITUATION_AWARENESS_COEFFICIENT = 2.5
PROBABILITY_FERTILIZATION = 0.00073


population = range(500, 21250);
probability1 = [ min(1, PROBABILITY_FERTILIZATION) for i in population ]
probability2 = [ min(1, PROBABILITY_FERTILIZATION * MAX_POP / i) for i in population ]
probability3 = [ min(1, PROBABILITY_FERTILIZATION * (MAX_POP / i)**6) for i in population ]
probability4 = [ min(1, PROBABILITY_FERTILIZATION * (MAX_POP / i)**(MAX_POP / i * SITUATION_AWARENESS_COEFFICIENT)) for i in population ]

host = host_subplot(111, axes_class=AA.Axes)
host.set_xlabel("Population size")
host.set_ylabel("Fertilisation probability")
host.plot(population, probability1, '-', label='Constant')
host.plot(population, probability2, '-', label='Linear density ratio')
host.plot(population, probability3, '-', label='Power of density ratio')
host.plot(population, probability4, '-', label='Exponential density ratio')

host.set_yscale('log')
host.set_ylim(ymax=1.5)
host.legend();
plt.draw();

if len(sys.argv) > 1:
	plt.savefig(sys.argv[1])
else:
	plt.show()

