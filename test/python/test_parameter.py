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

def first():

	# init sdmt manager
	sdmt.init('./config_python_test.xml', False)
	sdmt.register_int('test_para', 100)

	# get parameter from parameter.txt file
	data = sdmt.get_int('test_para')

	# check whether parameter value read is same as initial
	if data != 100:
		print('incorrect value {}'.format(data))
	# finalize sdmt module
	sdmt.finalize()

def second():
	# init sdmt manager
	sdmt.init('./config_python_test2.xml', False)

	# get parameter from parameter.txt file
	data = sdmt.get_int('test_para')

	# check whether parameter value read is same as initial
	if data != 200:
		print('incorrect value {}'.format(data))
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
