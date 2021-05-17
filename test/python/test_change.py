"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

# import sdmt
from sdmt import sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

# init sdmt manager
sdmt.init('./config_python_test.xml', False)

# request a sdmt segment
# define 1 dimensional integer array
# the size of array is 1024

size = 6
sdmt.register('sdmttest_int1d', sdmt.vt.int, sdmt.dt.array, [size])

# get segment and convert to numpy.ndarray
segment = sdmt.get('sdmttest_int1d')
data = np.array(segment, copy=False)

# write values to segment memory
for i in range(0, size):
    data[i] = i

# start sdmt module
sdmt.start()
sdmt.checkpoint(1)

for i in range(size):
    data[i] = 0
sdmt.recover()

for i in range(0, size):
    if data[i] != i:
        print('1 incorrect value {} : {}'.format(data[i], i))

# change size
size = size * 2
sdmt.change_segment('sdmttest_int1d', sdmt.vt.int, sdmt.dt.array, [size])
segment = sdmt.get('sdmttest_int1d')
data = np.array(segment, copy=False)
for i in range(0, size):
    data[i] = i * 2
sdmt.checkpoint(1)
# overwrite dummy values to segment memory
for i in range(size):
    data[i] = 0

# recover checkpoint
sdmt.recover()

# check recovered valuse
for i in range(0, size):
    if data[i] != i * 2:
        print('2 incorrect value {} : {}'.format(data[i], i))
# finalize sdmt module

# change size
size = int(size / 4)
sdmt.change_segment('sdmttest_int1d', sdmt.vt.int, sdmt.dt.array, [size])
segment = sdmt.get('sdmttest_int1d')
data = np.array(segment, copy=False)
for i in range(0, size):
	data[i] = i * 3
sdmt.checkpoint(1)

for i in range(0, size):
    data[i] = 0


#recover checkpoint
sdmt.recover()
for i in range(0 ,size):
    if data[i] != i *3:
        print('3 incorrect value {} : {}'.format(data[i], i))

sdmt.finalize()
