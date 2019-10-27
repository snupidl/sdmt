"""
Copyright 2018 PIDL(Petabyte-scale In-memory Database Lab)http://kdb.snu.ac.kr
"""

# import sdmt
import sys
sys.path.append('../../build')
import sdmtpy as sdmt

sdmt.init()
sdmt.start()
sdmt.finalize()
