"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

import sys

# import sdmt
import sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

def first():
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

    # end process w/o finalize
    # it will invoke FTI_Status on next(2nd) test
    #sdmt.finalize()

def second():
    # init sdmt manager
    sdmt.init('./config_python_test.xml', True)

    # get segment and convert to numpy.ndarray
    data = sdmt.register('sdmttest_int1d', 'int', 'array', [1024])

    # check recovered valuse
    for i in range(1024):
        if data[i] != i * i:
            print('incorrect value {} : {}'.format(data[i], i * i))

    # finalize sdmt module
    sdmt.finalize()

def help():
    print('USAGE: python test_restart.py 1(or 2)')

if __name__ == '__main__':
    try:
        test_num = int(sys.argv[1])
        if test_num == 1:
            first()
        elif test_num == 2:
            second()
        else: help()
    except Exception as e:
        print(e)
        help()
