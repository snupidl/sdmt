"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

estimating pagerank using facebook data
implemented by referring to
https://gist.github.com/diogojc/1338222/84d767a68da711a154778fb1d00e772d65322187
https://www.kaggle.com/rozemberczki/musae-facebook-pagepage-network?select=musae_facebook_target.csv
"""

import numpy as np
from scipy.sparse import csc_matrix

#import sdmt
import sdmt

#import mpi module
from mpi4py import MPI

def pageRank(G, r, it, s= .85, maxerr = .0001):
    """
    G : matrix representing state transitions, Gij = binary value
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
    with open('./dataset/musae_facebook_target.csv') as f:
        data = f.readlines()
        metadata = dict()
        for line in data[1:]:
            if not line : break
            line = line.split(',')
            metadata[int(line[0])] = (line[1], line[2], line[3])

        ### top 10
        for i in range(10):
            print(ranks[i][1], metadata[ranks[i][0]])


# init sdmt library
sdmt.init('./config_python_test.xml', True)

if not sdmt.exist('facebook'):
    facebook = sdmt.register('facebook', 'int', 'matrix', [22470, 22470], 0)
    r = sdmt.register('r', 'double', 'array', [22470], 1.0)

    # init edge data
    with open('./dataset/musae_facebook_edges.csv') as f:
        data = f.readlines()
        for line in data[1:]:
            if not line : break
            line = line.split(',')
            u, v = int(line[0]), int(line[1])
            facebook[u, v] = 1
            facebook[v, u] = 1
else:
    facebook = sdmt.get('facebook')
    r = sdmt.get('r')

# start sdmt module
sdmt.start()

# get current iteration sequence
it = sdmt.iter()

# run
ranks = pageRank(facebook, r, it, s=.86, maxerr = 0.0000000001)
top10(ranks)

# finalize sdmt module
sdmt.finalize()
