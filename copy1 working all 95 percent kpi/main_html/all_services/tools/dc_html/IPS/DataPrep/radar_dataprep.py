import numpy as np
from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.radar_hdf_datacollect import RadarHDFDataCollect

class RadarDataPreparation:
    input_scan_index_arr = output_scan_index_arr = None

    def __init__(self, mediator: IEventMediator, radar_ds: IDataStore, plot_ds: IDataStore):
        self._mediator = mediator
        self._radar_ds = radar_ds
        self._plot_ds = plot_ds

    def clear_data(self):
        RadarDataPreparation.input_scan_index_arr = RadarDataPreparation.output_scan_index_arr = None

    def consume_event(self, sensor, event):
        if event != "RDS_UPDATED": return
        sigs = RadarHDFDataCollect.sensor_to_sig[sensor]
        if 'scan_index' in sigs:
            sigs.remove('scan_index')
            sigs.insert(0, 'scan_index')
        for sig in sigs:
            if sig == 'scan_index':
                self._flatten_scan_index(sensor, sig)
            else:
                self._flatten_radar_data(sensor, sig)
        self._mediator.notify_event(sensor, "PDS_UPDATED")

    def _flatten_scan_index(self, sensor, sig):
        for src in ('in', 'out'):
            arr = self._radar_ds.get_data(sensor, sig, src).ravel()
            self._plot_ds.update_data(sig, arr, src)

    def _flatten_radar_data(self, sensor, sig):
        for src, scan_src in [('in', 'in'), ('out', 'out')]:
            data = self._radar_ds.get_data(sensor, sig, src)
            scale = data.shape[1] if data.ndim == 2 else None
            if scale:
                self._plot_ds.update_data(f"{sig}Dim", scale, src)
                scan = self._plot_ds.get_data('scan_index', src)
                if src == 'out':
                    in_scan = self._plot_ds.get_data('scan_index', 'in')
                    mask = np.isin(scan, in_scan)
                    data = data[mask]
                    scan = scan[mask]
                scaled = np.repeat(scan, scale)
                self._plot_ds.update_data(sig, scaled, f"{src}_scan_index", None, sensor)
            flat = data.ravel().astype(np.float32)
            if sig in ("theta", "phi", "std_theta_1", "std_phi_1"):
                np.degrees(flat, out=flat)
            self._plot_ds.update_data(sig, flat, src)
