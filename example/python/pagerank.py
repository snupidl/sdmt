"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

data reference : https://www.kaggle.com/rozemberczki/musae-facebook-pagepage-network?select=musae_facebook_target.csv
code reference : https://gist.github.com/diogojc/1338222/84d767a68da711a154778fb1d00e772d65322187
"""

import numpy as np
from scipy.sparse import csc_matrix

#import sdmt
from sdmt import sdmt

#import mpi module
from mpi4py import MPI

def pageRank(G, r, it, s= .85, maxerr = .0001):

	"""
	G : matrix representing state transitions
	Gij = binary value

	s : damping factor

	maxerr : converged point
	"""

	n = G.shape[0]
	# transform G into markov matrix A
	A = csc_matrix(G, dtype=np.float)
	rsums = np.array(A.sum(1))[:,0]
	ri, ci = A.nonzero()
	A.data /= rsums[ri]

	# bool array of sink states
	sink = rsums==0
	ro = np.zeros(n)
	while np.sum(np.abs(r-ro)) > maxerr:
		ro = r.copy()
		# calculate each pagerank at a time
		for i in range(0, n):
			# inlinks of state i
			Ai = np.array(A[:,i].todense())[:,0]
			# account for sink states
			Di = sink / float(n)
			# account for teleportation to state i
			Ei = np.ones(n) / float(n)
			r[i] = ro.dot(Ai*s + Di*s + Ei*(1-s))
		it = sdmt.next()
		sdmt.checkpoint(1)
	# return normalized pagerank
	r = r / float(sum(r))
	rank = list()
	for i in range(0, len(r)):
		rank.append([i, r[i]])
	ranks = sorted(rank, key = lambda rnk : rnk[1], reverse=True)
	return ranks

def top10(ranks):
	facebook_target = open('musae_facebook_target.csv', 'r')
	line = facebook_target.readline()
	metadata = dict()
	while True :
		line = facebook_target.readline()
		if not line : break
		spls = line.split('\n')
		spl = spls[0].split(',')
		metadata[int(spl[0])] = (spl[1], spl[2], spl[3])
	### top 10
	for i in range(0, 10):
		print(ranks[i][1], metadata[ranks[i][0]])

sdmt.init('./config_python_test.xml', True)

if not sdmt.exist('facebook'):
	sdmt.register('facebook', sdmt.vt.int, sdmt.dt.matrix, [22470,22470])
	sdmt.register('r', sdmt.vt.double, sdmt.dt.array, [22470])

	facebook = np.array(sdmt.get('facebook'), copy=False)
	for i in range(0, 22470):
		for j in range(0, 22470):
			facebook[i, j] = 0

	r = np.array(sdmt.get('r'), copy=False)
	for i in range(0, 22470):
		r[i] = 1.0

	facebook_edge = open('musae_facebook_edges.csv', 'r')
	line = facebook_edge.readline()
	while True:
		line = facebook_edge.readline()
		if not line : break
		spl = line.split(',')
		facebook[int(spl[0]),int(spl[1])] = 1
		facebook[int(spl[1]),int(spl[0])] = 1
	sdmt.checkpoint(1)
else:
	facebook = np.array(sdmt.get('facebook'), copy=False)
	r = np.array(sdmt.get('r'), copy=False)
sdmt.start()
it = sdmt.iter()
ranks = pageRank(facebook, r, it, s=.86, maxerr = 0.0000000001)
top10(ranks)

sdmt.finalize()
