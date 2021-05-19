"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

# import sdmt
import sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

# init sdmt manager
sdmt.init('./config_python_test.xml', False)

# register new snapshot
# define 1 dimensional integer array
# initial size of array is 1024
size = 1024
sdmt.register('sdmttest_int1d', 'int', 'array', [size])

# get snapshot and write values
data = sdmt.get('sdmttest_int1d')
for i in range(size):
    data[i] = i

# start sdmt module
sdmt.start()
sdmt.checkpoint(1)

# update values, then recover
data.fill(0)
sdmt.recover()

# check correctness
for i in range(size):
    if data[i] != i:
        print('1 incorrect value {} : {}'.format(data[i], i))

# update definition of snapshot
size = size * 2
data = sdmt.change_segment('sdmttest_int1d', 'int', 'array', [size])

# write values
for i in range(size):
    data[i] = i * 2
sdmt.checkpoint(1)

# overwrite dummy values to segment memory
data.fill(0)

# recover checkpoint
sdmt.recover()

# check correctness
for i in range(0, size):
    if data[i] != i * 2:
        print('2 incorrect value {} : {}'.format(data[i], i))

# update definiation of snapshot
size = size // 4
data = sdmt.change_segment('sdmttest_int1d', 'int', 'array', [size])

# write values
for i in range(size):
    data[i] = i * 3
sdmt.checkpoint(1)

# overwrite dummy values to segment memory
data.fill(0)

#recover checkpoint
sdmt.recover()

# check correctness
for i in range(0 ,size):
    if data[i] != i *3:
        print('3 incorrect value {} : {}'.format(data[i], i))

# finalize sdmt module
sdmt.finalize()
