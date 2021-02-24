"""
Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab)http://kdb.snu.ac.kr

estimating the value of Pi using Monte Carlo method
implemented by referring to
https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/

Author: Ilju Lee, ijlee@kdb.snu.ac.kr
"""

# import sdmt
import sys
sys.path.append('../..')
import sdmtpy as sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

import random

_interval =  1000

sdmt.init('./config_python_test.xml', True)

# check this execution is first try
# if it is, create segments
# else get recovered value
if not sdmt.exist('mc_pi'):
    sdmt.register('mc_pi', sdmt.vt.double, sdmt.dt.array, [1])
    sdmt.register('mc_circle', sdmt.vt.int, sdmt.dt.array, [1])
    sdmt.register('mc_square', sdmt.vt.int, sdmt.dt.array, [1])

    pi = np.array(sdmt.get('mc_pi'), copy=False)
    circle_points = np.array(sdmt.get('mc_circle'), copy=False)
    square_points = np.array(sdmt.get('mc_square'), copy=False)

    circle_points[0] = 0
    square_points[0] = 0
else:
    pi = np.array(sdmt.get('mc_pi'), copy=False)
    circle_points = np.array(sdmt.get('mc_circle'), copy=False)
    square_points = np.array(sdmt.get('mc_square'), copy=False)

# get current iteration sequence
it = sdmt.iter()

# start sdmt module
sdmt.start();

while it < (_interval * _interval):
    # randomly generated x and y values
    rand_x = random.randint(0, _interval) / _interval
    rand_y = random.randint(0, _interval) / _interval

    # distance between (x, y) from the origin
    origin_dist = rand_x * rand_x + rand_y * rand_y

    # checking if (x, y) lies inside the define
    # circle with R=1
    if origin_dist <= 1:
        circle_points[0] += 1

    # total number of points generated
    square_points[0] += 1

    # estimated pi after this iteration
    pi[0] = (4 * circle_points[0]) / square_points[0]

    # checkpoint for every 10 interval
    if it % (_interval * 10) == 0:
	    sdmt.checkpoint(1)
	    print('{}th iteration has processed, current Pi is {:.4f}'.format(it, pi[0]))
    # move to next iteration
    it = sdmt.next()

# final result
print('final estimation of Pi = {:.4f}'.format(pi[0]))

# finalize sdmt module
sdmt.finalize();
