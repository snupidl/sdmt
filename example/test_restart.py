"""
Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab)http://kdb.snu.ac.kr
"""

# import sdmt
import sys
sys.path.append('../build')
import sdmtpy as sdmt

# import mpi module
from mpi4py import MPI

# import numpy
import numpy as np

def first():
    # init sdmt manager
    sdmt.init('../config.xml', False)

    # request a sdmt segment
    # define 1 dimensional integer array
    # the size of array is 1024
    sdmt.register('sdmttest_int1d', sdmt.vt.int, sdmt.dt.array, [1024])

    # get segment and convert to numpy.ndarray
    segment = sdmt.get('sdmttest_int1d')
    data = np.array(segment, copy=False)

    # write values to segment memory
    for i in range(1024):
        data[i] = i * i

    # start sdmt module
    sdmt.start()

    # generate checkpoint
    sdmt.checkpoint()

    # overwrite dummy values to segment memory
    for i in range(1024):
        data[i] = 0

    # end process w/o finalize
    # it will invoke FTI_Status on next(2nd) test
    #sdmt.finalize()

def second():
    # init sdmt manager
    sdmt.init('../config.xml', True)

    # get segment and convert to numpy.ndarray
    segment = sdmt.get('sdmttest_int1d')
    data = np.array(segment, copy=False)

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
    except:
        help()

