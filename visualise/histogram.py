#!/usr/bin/python

import fileinput
import sys
from pprint import pprint
import math

from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA
import matplotlib.pyplot as plt

def dup(list):
	return [0] + [e for t in zip(list, list, list, list) for e in t] + [0]

def dupp(list):
	return [0] + [e + i / 4 for t in zip(list, list, list, list) for (e, i) in zip(t, range(0, 4))] + [list[-1] + 1]

def filter(x, y):
	return list(zip(*[(x[i], y[i]) for i in range(len(x)) if y[i] > 0]))

ageList = []
humanMalesList = []
humanFemalesList = []
humanPregnantList = []
infectedMalesList = []
infectedFemalesList = []
infectedPregnantList = []
zombiesList = []

humanMales = 0
humanFemales = 0
humanPregnant = 0
infectedMales = 0
infectedFemales = 0
infectedPregnant = 0
zombies = 0

for line in fileinput.input('-'):
	(x, age, x, HM, x, HF, x, HP, x, IM, x, IF, x, IP, x, Z) = line.split()
	(age, HM, HF, HP, IM, IF, IP, Z) = \
			(int(age), int(HM), int(HF), int(HP), int(IM), int(IF), int(IP), int(Z))
	ageList.append(age)
	humanMalesList.append(HM); humanMales += HM
	humanFemalesList.append(HF); humanFemales += HF
	humanPregnantList.append(HP); humanPregnant += HP
	infectedMalesList.append(IM); infectedMales += IM
	infectedFemalesList.append(IF); infectedFemales += IF
	infectedPregnantList.append(IP); infectedPregnant += IP
	zombiesList.append(Z); zombies += Z
	
infected = infectedMales + infectedFemales > 0

host = host_subplot(111, axes_class=AA.Axes)
plt.subplots_adjust(right=0.75)

par1 = host.twinx()
if infected:
	par2 = host.twinx()
	offset = 80
	new_fixed_axis = par2.get_grid_helper().new_fixed_axis
	par2.axis["right"] = new_fixed_axis(loc="right", axes=par2, offset=(offset, 0))
	par2.axis["right"].toggle(all=True)
else:
	par2 = par1

host.set_xlabel("Age (Humans in Years, Zombies in Months)")
m = max(ageList)
host.set_xlim(0, m + 0.75)

hml = [m / max(humanMales, 1) for m in humanMalesList]
hfl = [m / max(humanFemales, 1) for m in humanFemalesList]
hpl = [m / max(humanPregnant, 1) for m in humanPregnantList]
host.plot(dupp(ageList), dup(hml), '-', color='#008800', linewidth=1.5, label='Human Males', zorder=10)
host.plot(dupp(ageList), dup(hfl), '-', color='#00dd00', linewidth=1.5, label='Human Females', zorder=10)
host.fill(dupp(ageList), dup(hpl), '-', color='#77dd00', alpha=0.5, linewidth=1, label='Pregnant Human Females', zorder=1)
m = max(max(hml), max(hfl))
host.set_ylim(-0.2 * m, 1.2 * m)
host.set_ylabel('Age Distribution of Humans')
host.axis['left'].label.set_color('green')
host.axis['right'].label.set_color('green')

if infected > 0:
	m = 0
	imlTouples = filter(dupp(ageList), dup([m / max(infectedMales, 1) for m in infectedMalesList]))
	if imlTouples != []:
		par1.plot(imlTouples[0], imlTouples[1], '.', color='#0000aa', linewidth=1, label='Infected Males', zorder=5)
		m = max(m, max(imlTouples[1]))
	iflTouples = filter(dupp(ageList), dup([m / max(infectedFemales, 1) for m in infectedFemalesList]))
	if iflTouples != []:
		par1.plot(iflTouples[0], iflTouples[1], '.', color='#8888dd', linewidth=1, label='Infected Females', zorder=5)
		m = max(m, max(iflTouples[1]))
	ipl = [m / max(infectedPregnant, 1) for m in infectedPregnantList]
	par1.fill(dupp(ageList), dup(ipl), '-', color='#dd88dd', alpha=0.5, linewidth=1, label='Pregnant Infected Females', zorder=2)
	
	par1.set_ylim(-0.2 * m, 1.2 * m)
	par1.set_ylabel('Age Distribution of Infected')
	par1.axis['left'].label.set_color('blue')
	par1.axis['right'].label.set_color('blue')

zl = [m / max(zombies, 1) for m in zombiesList]
par2.plot(dupp(ageList), dup(zl), '-', color='#dd0000', linewidth=1.5, label='Zombies', zorder=20)
m = max(zl)
par2.set_ylim(-0.2 * m, 1.2 * m)
par2.set_ylabel('Age Distribution of Zombies')
par2.axis['left'].label.set_color('red')
par2.axis['right'].label.set_color('red')


host.legend(fontsize='small')
plt.draw()

if(len(sys.argv) > 1):
	plt.savefig(sys.argv[1])
else:
	plt.show()
