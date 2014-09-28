#!/usr/bin/python

import fileinput

from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
import matplotlib.pyplot as plt

timeList = []
humansList = []
infectedList = []
zombiesList = []

for line in fileinput.input():
	(x, time, x, humans, x, infected, x, zombies) = line.split()
	timeList.append(int(time))
	humansList.append(int(humans))
	infectedList.append(int(infected))
	zombiesList.append(int(zombies))

host = host_subplot(111, axes_class=AA.Axes)
plt.subplots_adjust(right=0.75)

par1 = host.twinx()
par2 = host.twinx()

offset = 60
new_fixed_axis = par2.get_grid_helper().new_fixed_axis
par2.axis["right"] = new_fixed_axis(loc="right",
                                    axes=par2,
                                    offset=(offset, 0))
par2.axis["right"].toggle(all=True)

host.set_xlabel("Time")
host.set_ylabel("Humans")
par1.set_ylabel("Infected")
par2.set_ylabel("Zombies")

p1, = host.plot(timeList, humansList, 'g-', label='Humans')
p2, = par1.plot(timeList, infectedList, 'b-', label='Infected')
p3, = par2.plot(timeList, zombiesList, 'r-', label='Zombies')

mHumans = max(humansList);
mInfected = max(infectedList);
mZombies = max(zombiesList);

host.set_ylim(-mHumans*0.2, mHumans*1.2)
par1.set_ylim(-mInfected*0.2, mInfected*1.2)
par2.set_ylim(-mZombies*0.2, mZombies*1.2)

host.legend()

host.axis["left"].label.set_color(p1.get_color())
par1.axis["right"].label.set_color(p2.get_color())
par2.axis["right"].label.set_color(p3.get_color())

plt.draw()
plt.show()
