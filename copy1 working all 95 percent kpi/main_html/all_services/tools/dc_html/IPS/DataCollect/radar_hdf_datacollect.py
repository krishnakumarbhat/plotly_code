import h5py, os
import numpy as np
from collections import defaultdict
from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.icollectdata import IDataCollect
from IPS.Sig_Prep.isig_observer import ISigObserver

class RadarHDFDataCollect(IDataCollect, ISigObserver):
    sensor_to_sig = defaultdict(list)

    def __init__(self, mediator: IEventMediator, radar_ds: IDataStore, plot_ds: IDataStore):
        self._mediator = mediator
        self._radar_ds = radar_ds
        self._plot_ds = plot_ds
        self._in_file = self._out_file = None
        self._poi_paths = defaultdict(list)

    def clear_data(self):
        RadarHDFDataCollect.sensor_to_sig.clear()
        self._in_file = self._out_file = None
        self._poi_paths.clear()

    def collect_data(self, inp=None, out=None):
        self._in_file, self._out_file = inp, out
        self._parse_and_collect()
        self._mediator.notify_event(self, "JSON_GEN_DONE")

    def _parse_and_collect(self):
        for sensor, paths in self._poi_paths.items():
            print(f"----------------------------\nprocessing sensor {sensor}")
            self._parse_hdf(str(self._in_file), "in", paths)
            self._parse_hdf(str(self._out_file), "out", paths)
            self._mediator.notify_event(sensor, "RDS_UPDATED")

    def final_poi_sensor_datapath(self, data):
        self._poi_paths = data
        for sensor, paths in self._poi_paths.items():
            for p in paths:
                RadarHDFDataCollect.sensor_to_sig[sensor].append(os.path.basename(p))

    def _parse_hdf(self, hdf_file, src, paths):
        with h5py.File(hdf_file, 'r') as f:
            for p in paths:
                if p not in f:
                    print(f"Dataset {p} not in file {hdf_file}")
                    continue
                sig = os.path.basename(p)
                sensor = p.split('/')[0]
                data = f[p][()]
                # Replace exponential values with zero
                mask = np.vectorize(lambda x: 'e' in format(x, '.15e') and (abs(x) < 1e-3 or abs(x) >= 1e4))(data)
                data[mask] = 0
                # Remove zeros from each row and pad
                rows = [r[r != 0.0] for r in data]
                maxlen = max(len(r) for r in rows) if rows else 0
                filtered = np.array([np.pad(r, (0, maxlen - len(r)), constant_values=0) for r in rows])
                self._radar_ds.update_data(sensor, sig, filtered, src)

    def consume_event(self, sensor, event): pass
