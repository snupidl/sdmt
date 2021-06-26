"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr

Solving target function using Particle Swarm Optimization
implemented by referring to

code reference : https://medium.com/@mamady94/a-tutorial-on-optimization-algorithms-the-example-of-particle-swarm-optimization-981d883be9d5
"""

import numpy as np

# import sdmt
import sdmt

# import mpi module
from mpi4py import MPI

def update_position(x, v):
    new_x = x + v
    return new_x
def update_velocity(x, v, p_best, g_best, c0=0.5, c1=1.5, w=0.75):
    r = np.random.uniform()
    new_v = w*v + c0*r*(p_best-x) + c1*r*(g_best-x)
    return new_v

def sphere(x): ### global solution (0,0,0)
    return np.sum(np.square(x))

### target function
def func(x) :
    return (x[0]**2 + x[1] -11)**2 + (x[0] + x[1]**2 -7)**2

# init sdmt library
sdmt.init('./config_python_test.xml', True)

# register sdmt snapshot, restore if exists
particles_pos = sdmt.register_snapshot('particles_pos', 'double', 'matrix', [50, 2], 0)
velocities = sdmt.register_snapshot('velocities', 'double', 'matrix', [50, 2], np.random.uniform)
p_best = sdmt.register_snapshot('p_best', 'double', 'matrix', [50, 2], 0)

n_particles = 50
g_best = [0, 0]
maxiter = 1500

# get current iteration sequence
it = sdmt.iter()

# start sdmt module
sdmt.start()

print('current iteration order : ', it)
while it < maxiter :
    if it % 100 == 0:
        sdmt.checkpoint(1)
        print('current iteration order : ', it)

    for i in range(n_particles):
        x = particles_pos[i]
        v = velocities[i]
        velocities[i] = update_velocity(x, v, p_best[i], g_best)
        particles_pos[i] = update_position(x,v)
        if func(particles_pos[i]) < func(p_best[i]):
            p_best[i] = particles_pos[i]
        if func(particles_pos[i]) < func(g_best):
            g_best = particles_pos[i]

    # move to next iteration
    it = sdmt.next()

# print result
print('solution : ', g_best)
print('results : ', func(g_best))

# finalize sdmt module
sdmt.finalize();
