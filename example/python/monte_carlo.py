"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

estimating the value of Pi using Monte Carlo method
implemented by referring to
https://www.geeksforgeeks.org/estimating-value-pi-using-monte-carlo/
"""

# import sdmt
import sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

import random

_interval =  1000

# init sdmt library
sdmt.init('./config_python_test.xml', True)

# register sdmt snapshot, restore if exists
pi = sdmt.register('mc_pi', 'double', 'array', [1])
circle_points = sdmt.register('mc_circle', 'int', 'array', [1], 0)
square_points = sdmt.register('mc_square', 'int', 'array', [1], 0)

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
        print(f'{it}th iteration has processed, current Pi is {pi[0]:.4f}')

    # move to next iteration
    it = sdmt.next()

# print result
print('final estimation of Pi = {:.4f}'.format(pi[0]))

# finalize sdmt module
sdmt.finalize();
