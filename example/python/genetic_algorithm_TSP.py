"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

code reference : https://github.com/ezstoltz/genetic-algorithm/blob/master/genetic_algorithm_TSP.ipynb
"""

import numpy as np
import random
import operator
import pandas as pd
import math

# import sdmt
from sdmt import sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

import matplotlib.pyplot as plt

R = 6371 # radius of the Earth


def haversine_distance(fr, to):
	dLat = (fr[1] - to[1]) * (math.pi/180)
	dLon = (fr[2] - to[2]) * (math.pi/180)
	a = math.sin(dLat/2) * math.sin(dLat/2) + \
	(math.cos(fr[1]*(math.pi/180)) * math.cos(to[1]*(math.pi/180)) * math.sin(dLon/2) * math.sin(dLon/2))
	b = 2* math.atan2(math.sqrt(a), math.sqrt(1-a))
	return round(R*b, 5)

### create initial population
def createRoute(cityList):
	route = random.sample(cityList, len(cityList))
	return route

def initialPopulation(popSize, cityList):
	population = []
	for i in range(0, popSize):
		population.append(createRoute(cityList))
	return population

def fitness(pop):
	pathDistance = 0
	for i in range(0, len(pop)):
		fromCity = pop[i]
		toCity = None
		if i+1 < len(pop):
			toCity = pop[i+1]
		else:
			toCity = pop[0]
		pathDistance += haversine_distance(fromCity, toCity)
	return 1 / float(pathDistance)

### genetic algorithm
def rankRoutes(population):
	fitnessResults = {}
	for i in range(0, len(population)):
		fitnessResults[i] = fitness(population[i])
	return sorted(fitnessResults.items(), key = operator.itemgetter(1), reverse = True)


def selection(popRanked, eliteSize):
	selectionResults = []
	df = pd.DataFrame(np.array(popRanked), columns = ["index", "Fitness"])
	df['cum_sum'] = df.Fitness.cumsum()
	df['cum_perc'] = 100*df.cum_sum/df.Fitness.sum()
	for i in range(0, eliteSize):
		selectionResults.append(popRanked[i][0])
	for i in range(0, len(popRanked) - eliteSize):
		pick = 100 * random.random()
		for i in range(0, len(popRanked)):
			if pick <= df.iat[i,3]:
				selectionResults.append(popRanked[i][0])
				break
	return selectionResults

def matingPool(population, selectionResults):
	matingpool = []
	for i in range(0, len(selectionResults)):
		index = selectionResults[i]
		matingpool.append(population[index])
	return matingpool

### breed = crossover
def breed(parent1, parent2):
	child = []
	childP1 = []
	childP2 = []

	geneA = int(random.random() * len(parent1))
	geneB = int(random.random() * len(parent1))

	startGene = min(geneA, geneB)
	endGene = max(geneA, geneB)


	for i in range(startGene, endGene):
		childP1.append(parent1[i])
	childP2 = [item for item in parent2 if item not in childP1]

	i = 0
	j = 0
	k = 0
	while i < len(parent1):
		if i < startGene:
			child.append(childP2[k])
			k+=1
		elif i < endGene:
			child.append(childP1[j])
			j+=1
		else :
			child.append(childP2[k])
			k+=1
		i+=1
	return child

def breedPopulation(matingpool, eliteSize):
	children = []
	length = len(matingpool) - eliteSize
	pool = random.sample(matingpool, len(matingpool))
	for i in range(0, eliteSize):
		children.append(matingpool[i])
	for i in range(0, length):
		child = breed(pool[i], pool[len(matingpool)-i-1])
		children.append(child)
	return children

def mutate(individual, mutationRate):
	for swapped in range(len(individual)):
		if (random.random()<mutationRate):
			swapWith = int(random.random()*len(individual))
			city1 = individual[swapped]
			city2 = individual[swapWith]
			individual[swapped] = city2
			individual[swapWith] = city1
	return individual

def mutatePopulation(population, mutationRate):
	mutatedPop = []
	for ind in range(0, len(population)):
		mutatedInd = mutate(population[ind], mutationRate)
		mutatedPop.append(mutatedInd)
	return mutatedPop

def nextGeneration(currentGen, eliteSize, mutationRate):
	popRanked = rankRoutes(currentGen)
	selectionResults = selection(popRanked, eliteSize)
	matingpool = matingPool(currentGen, selectionResults)
	children = breedPopulation(matingpool, eliteSize)
	nextGeneration = mutatePopulation(children, mutationRate)
	return nextGeneration


def printRoute(route):
	cityDict = dict()
	cityfile = open("olympic_city.csv", 'r')
	while True:
		line = cityfile.readline()
		if not line : break
		spl = line.split(',')
		cityDict[float(spl[0])] = spl[1]
	pr = ""
	for i in range(0, len(route)):
		pr += cityDict[route[i][0]]
		if i != len(route)-1:
			pr += ","
	print (pr)



generations = 4000
popSize = 100
eliteSize = 15
mutationRate = 0.05

sdmt.init('./config_python_test.xml', True)

# check this execution is first try
# if it is, create segments
# else get recovered value

if not sdmt.exist('population'):
	sdmt.register('population', sdmt.vt.float, sdmt.dt.tensor, [100,23,3])
	population = np.array(sdmt.get('population'), copy=False)

	cityList = list()
	cityfile = open("olympic_city.csv", 'r')
	while True:
		line = cityfile.readline()
		if not line : break
		spl = line.split(',')
		cityList.append([float(spl[0]), float(spl[2]), float(spl[3])])
	pop = initialPopulation(popSize, cityList)
	pop_ = np.array(pop)
	for i in range(0, 100):
		for j in range(0, 23):
			for k in range(0, 3):
				population[i,j,k] = pop_[i,j,k]
	sdmt.checkpoint(1)
	print("initial distance : " + str(1 / rankRoutes(population)[0][1]))
else :
	population = np.array(sdmt.get('population'), copy=False)


it = sdmt.iter()
print('current iteration order : ', it)
sdmt.start()
pop = population.tolist()
while it < generations:
	if it % 100 == 0 :
		pop_ = np.array(pop)
		for i in range(0, 100):
			for j in range(0, 23):
				for k in range(0, 3):
					population[i,j,k] = pop_[i,j,k]
		sdmt.checkpoint(1)
		print(str(it) + " : "  + str(1 / rankRoutes(pop)[0][1]))
	pop = nextGeneration(pop, eliteSize, mutationRate)
	it=sdmt.next()
print("Final distance : " + str(1 / rankRoutes(pop)[0][1]))
population = np.array(sdmt.get("population"), copy=False)

bestRouteIndex = rankRoutes(pop)[0][0]
bestRoute = pop[bestRouteIndex]
printRoute(bestRoute)

sdmt.finalize()
