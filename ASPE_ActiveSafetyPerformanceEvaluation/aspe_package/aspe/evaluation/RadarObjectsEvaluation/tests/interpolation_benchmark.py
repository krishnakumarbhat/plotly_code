import numpy as np
import pandas as pd
from enum import Enum
from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.providers.Preprocessing.ScanIndexSynch import ScanIndexInterpolationSynch


class EnumA(Enum):
    A = 0
    B = 1
    C = 2
    D = 3
    E = 4


class EnumB(Enum):
    a = 0
    b = 1
    c = 2
    d = 3
    e = 4
    f = 5


def get_signle_object_signals(life_time, unique_id):
    obj = {
        'timestamp': np.linspace(0, life_time*0.05, life_time) + np.random.rand() * 0.1,
        'scan_index': np.arange(0, life_time),
        'unique_id': np.full(life_time, unique_id),
        'sig_a': np.random.rand(life_time),
        'sig_b': np.random.rand(life_time),
        'sig_c': np.random.rand(life_time),
        'sig_d': np.random.rand(life_time),
        'sig_e': np.random.rand(life_time),
        'sig_f': np.random.rand(life_time),
        'sig_g': np.random.rand(life_time),
        'sig_h': np.random.rand(life_time),
        'sig_i': np.random.rand(life_time),
        'sig_j': np.random.rand(life_time),
        'enum_a': np.array([*EnumA], object)[np.random.randint(0, 5, size=life_time)],
        'enum_b': np.array([*EnumB], object)[np.random.randint(0, 6, size=life_time)],
        'bool_a': np.random.randint(0, 2, size=life_time).astype(bool),
        'bool_b': np.random.randint(0, 2, size=life_time).astype(bool),
    }
    df = pd.DataFrame(obj)
    return df


def get_n_objs(n_objs, life_time):
    dfs = []
    for n in range(n_objs):
        obj = get_signle_object_signals(life_time, n)
        dfs.append(obj)
    objs = pd.concat(dfs).reset_index(drop=True)
    return objs


def get_master_slave_data_sets(n_slave_objs, life_time):
    slave_signals = get_n_objs(n_slave_objs, life_time)
    slave_ds = IObjects()
    slave_ds.signals = slave_signals

    master_signals = {
        'timestamp': np.linspace(0, life_time*0.05, life_time),
        'scan_index': np.arange(life_time)
    }
    master_ds = IObjects()
    master_ds.signals = pd.DataFrame(master_signals)
    return master_ds, slave_ds


if __name__ == '__main__':
    import time
    n_objs = 1000
    obj_lifetime = 100
    master, slave = get_master_slave_data_sets(n_objs, obj_lifetime)
    interp = ScanIndexInterpolationSynch()
    t1 = time.time()
    interp.synch(None, None, master, slave)
    print(f"{time.time() - t1}")
