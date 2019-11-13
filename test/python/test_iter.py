"""
Copyright 2019 PIDL(Petabyte-scale In-memory Database Lab)http://kdb.snu.ac.kr
"""

# import sdmt
import sys
sys.path.append('../..')
import sdmtpy as sdmt

# import mpi module
from mpi4py import MPI

# init sdmt manager
sdmt.init('./config_python_test.xml', False)

# get iteration seqeunce
it = sdmt.iter()

# increase iteration sequence
# 0 -> 1
it = sdmt.next();

# start sdmt module
sdmt.start();

# generate checkpoint
sdmt.checkpoint(1);

# increase iteration sequence
# 1 -> 2
it = sdmt.next()

# recover checkpoint
sdmt.recover()

# check recovered values
it = sdmt.iter()
if it != 1:
    print('incorrect value {} : {}'.format(it, 1))

# finalize sdmt module
sdmt.finalize()
