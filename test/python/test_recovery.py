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

# request a sdmt segment
# define 1 dimensional integer array
# the size of array is 1024
data = sdmt.register('sdmttest_int1d', 'int', 'array', [1024])

# write values to segment memory
for i in range(1024):
    data[i] = i * i

# start sdmt module
sdmt.start()

# generate checkpoint
sdmt.checkpoint(1)

# overwrite dummy values to segment memory
data.fill(0)

# recover checkpoint
sdmt.recover()

# check recovered valuse
for i in range(1024):
    if data[i] != i * i:
        print('incorrect value {} : {}'.format(data[i], i * i))

# finalize sdmt module
sdmt.finalize()
