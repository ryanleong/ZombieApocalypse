#!/usr/bin/python

import fileinput
import sys
from pprint import pprint
import math

from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
import matplotlib.pyplot as plt

timeList = []
humansList = []
infectedList = []
zombiesList = []

for line in fileinput.input('-'):
	if not line.startswith('Time:'):
		continue;
	(x, time, x, humans, x, infected, x, zombies) = line.split()
	timeList.append(int(time))
	humansList.append(int(humans))
	infectedList.append(int(infected))
	zombiesList.append(int(zombies))


timeList2 = []
humansList2 = []
infectedList2 = []
zombiesList2 = []

length = len(timeList);
each = math.ceil(length / min(length, 10000))
for i in [i for i in range(length) if i % each == 0]:
	timeList2.append(timeList[i])
	humansList2.append(humansList[i])
	infectedList2.append(infectedList[i])
	zombiesList2.append(zombiesList[i])




host = host_subplot(111, axes_class=AA.Axes)
plt.subplots_adjust(right=0.75)

par1 = host.twinx()
par2 = host.twinx()



entities = [ {'name': "Humans", 'list': humansList2, 'style': "g-", 'zorder': 10, 'coef': 1},
	{'name': "Zombies", 'list': zombiesList2, 'style': "r-", 'zorder': 5, 'coef': 1},
	{'name': "Infected", 'list': infectedList2, 'style': "b-", 'zorder': 1, 'coef': 1.4} ]
plots = [host, par1, par2]



offset = 80
new_fixed_axis = par2.get_grid_helper().new_fixed_axis
par2.axis["right"] = new_fixed_axis(loc="right",
                                    axes=par2,
                                    offset=(offset, 0))
par2.axis["right"].toggle(all=True)

host.set_xlabel("Time")
for i in range(len(entities)):
	plots[i].set_ylabel(entities[i]['name'])
	p, = plots[i].plot(timeList2, entities[i]['list'], entities[i]['style'], label=entities[i]['name'], zorder=entities[i]['zorder'])
	entities[i]['plot'] = p

	m = max(entities[i]['list']) * entities[i]['coef']
	plots[i].set_ylim(-0.2 * m, 1.2 * m)
	plots[i].axis['left'].label.set_color(p.get_color())
	plots[i].axis['right'].label.set_color(p.get_color())
	
host.legend()
plt.draw()

if(len(sys.argv) > 1):
	plt.savefig(sys.argv[1])
else:
	plt.show()
