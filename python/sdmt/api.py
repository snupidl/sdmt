"""
Copyright 2021 PIDL(Petabyte-scale In-memory Database Lab) http://kdb.snu.ac.kr
This work was supported by Next-Generation Information Computing Development
Program through the National Research Foundation of Korea(NRF)
funded by the Ministry of Science, ICT (NRF-2016M3C4A7952587)
Author: Ilju Lee, Jongin Kim, Hyerim Jeon, Youngjune Park
Contact: sdmt@kdb.snu.ac.kr
"""

import sys, os
sdmt_path = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
sdmt_path = os.path.join(sdmt_path, '../build')
sys.path.append(sdmt_path)
import sdmtpy
import sdmt

import numpy as np

_vt_map = {
        'int': sdmtpy.vt.int,
        'long': sdmtpy.vt.long,
        'float': sdmtpy.vt.float,
        'double': sdmtpy.vt.float,
        }
_dt_map = {
        'scalar': sdmtpy.dt.scalar,
        'array': sdmtpy.dt.array,
        'matrix': sdmtpy.dt.matrix,
        'tensor': sdmtpy.dt.tensor,
        }

def init(config, restart=True):
    """Init SDMT library module
    Parameters:
    -----------
    path: path of config file
    restart: recover snapshot if archive exists
    """
    try:
        # initialize library
        sdmtpy.init(config, restart)
    except Exception as e:
        sdmt.cli.error('Failed to initialize sdmt library, ' + str(e))

def register_snapshot(name, vt=None, dt=None, dim=None, init=0):
    """Register snapshot to sdmt module
    Parameters:
    -----------
    name: name of snapshot
    vt: value type of snapshot
    dt: data type of snapshot
    dim: dimension of snapshot
    init: initial value of snapshot
    """
    try:
        if not sdmtpy.exist(name):
            if vt not in _vt_map:
                raise ValueError("invalied value type, "
                    "shoud be one of ('int', 'long', 'float', 'double')")
            else:
                vt = _vt_map[vt]

            if dt not in _dt_map:
                raise ValueError("invalied data type, "
                    "shoud be one of ('scalar', 'array', 'matrix', 'tensor')")
            else:
                dt = _dt_map[dt]

            if (dt == sdmtpy.dt.scalar and dim != None) \
                    or (dt == sdmtpy.dt.array and len(dim) != 1) \
                    or (dt == sdmtpy.dt.matrix and len(dim) != 2) \
                    or (dt == sdmtpy.dt.tensor and len(dim) < 1):
                raise ValueError('wrong dimension definition')

            sdmtpy.register(name, vt, dt, dim)
            res = np.array(sdmtpy.get(name), copy=False)
            if type(init) in (int, float):
                res.fill(init)
            elif type(init) == list or isinstance(init, np.ndarray):
                np.copyto(res, init)
            elif callable(init):
                for i in range(res.size):
                    res.itemset(i, init())
            return res
        else:
            return np.array(sdmtpy.get(name), copy=False)

    except Exception as e:
        sdmt.cli.error('Failed to register sdmt snapshot, ' + str(e))

def start():
    """Start sdmt module
    """
    try:
        sdmtpy.start()
    except Exception as e:
        sdmt.cli.error('Failed to start sdmt, ' + str(e))

def checkpoint(level=1):
    """Generate a snapshot
    Parameters:
    -----------
    level: snapshot level(arguments for FTI library)
    """
    try:
        sdmtpy.checkpoint(level)
    except Exception as e:
        sdmt.cli.error('Failed to make sdmt snapshot, ' + str(e))

def recover():
    """Recover snapshots
    """
    try:
        sdmtpy.recover()
    except Exception as e:
        sdmt.cli.error('Failed to recover snapshot, ' + str(e))

def get(name):
    """Get snapshot
    Parameters:
    -----------
    name: name of snapshot
    """
    try:
        return np.array(sdmtpy.get(name), copy=False)
    except Exception as e:
        sdmt.cli.error('Failed to get sdmt snapshot, ' + str(e))

def change(name, vt=None, dt=None, dim=None, init=0):
    """Assign new snapshot size
    Parameters:
    -----------
    name: name of snapshot
    vt: value type of new snapshot
    dt: data type of new snapshot
    dim: dimension of new snapshot
    init: initial value of new snapshot
    """
    try:
        if not sdmtpy.exist(name):
            raise ValueError('not exsiting snapshot name')
        else:
            if vt not in _vt_map:
                raise ValueError("invalied value type, "
                    "shoud be one of ('int', 'long', 'float', 'double')")
            else:
                vt = _vt_map[vt]

            if dt not in _dt_map:
                raise ValueError("invalied data type, "
                    "shoud be one of ('scalar', 'array', 'matrix', 'tensor')")
            else:
                dt = _dt_map[dt]

            # update snapshot
            sdmtpy.change_segment(name, vt, dt, dim)

            res = np.array(sdmtpy.get(name), copy=False)
            if type(init) in (int, float):
                res.fill(init)
            elif type(init) == list or isinstance(init, np.ndarray):
                np.copyto(res, init)
            elif callable(init):
                for i in range(res.size):
                    res.itemset(i, init())
            return res

    except Exception as e:
        sdmt.cli.error('Failed to change sdmt snapshot, ' + str(e))

def finalize():
    """Finalize sdmt module
    """
    try:
        sdmtpy.finalize()
    except Exception as e:
        sdmt.cli.error('Failed to finalize sdmt, ' + str(e))

def iter():
    """Get global iteration count in sdmt module
    """
    try:
        return sdmtpy.iter()
    except Exception as e:
        sdmt.cli.error('Failed to get sdmt iter count, ' + str(e))

def next():
    """Proceed global iteration count in sdmt module
    """
    try:
        return sdmtpy.next()
    except Exception as e:
        sdmt.cli.error('Failed to proceed sdmt iterator, ' + str(e))

def exist(name):
    """Check snapshot exists in archive
    Parameters:
    -----------
    name: name of snapshot
    """
    try:
        return sdmtpy.exist(name)
    except Exception as e:
        sdmt.cli.error('Failed to check whether snapshot exists, ' + str(e))

def register_param(name, value):
    """ register int parameter in parameter.txt
    Parameters:
    -----------
    name: name of parameter
    value: value of parameter
    """
    try:
        if type(value) == int:
            return sdmt.register_int(name, value)
        elif type(value) == float:
            return sdmt.register_float(name, value)
        else:
            raise Exception("invalid value type")
    except Exception as e:
        sdmt.cli.error('Failed to register sdmt int parameter,' + str(e))

def get_param(name, vt):
    """	Get int parameter in parameter.txt
    Parameters:
    -----------
    name: name of parameter
    vt: value type of parameter
    """
    try:
        if vt == 'int':
            return sdmtpy.get_int(name)
        elif vt == 'float':
            return sdmtpy.get_float(name)
        else:
            raise Exception("invalid value type")
    except Exception as e:
        sdmt.cli.error('Failed to get sdmt parameter, ' + str(e))
