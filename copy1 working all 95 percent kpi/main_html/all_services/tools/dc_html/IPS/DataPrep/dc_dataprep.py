import numpy as np
from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.Sig_Prep.sig_filter import SignalFilter
from IPS.DataCollect.icollectdata import IDataCollect
from IPS.DashManager.report_dash import ReportDash
from IPS.RepGen.dc_json_to_html_convert import DCJsonToHtmlConvertor

class DCPlotDataPreparation:
    def __init__(self, mediator: IEventMediator, radar_ds: IDataStore, plot_ds: IDataStore, json_ds: IDataStore, sig_fil: SignalFilter, dc_collector: IDataCollect):
        self._mediator = mediator
        self._plot_ds = plot_ds
        self._radar_ds = radar_ds
        self._json_ds = json_ds
        self._sig_fil = sig_fil
        self._dc_collector = dc_collector

    def clear_data(self): pass

    def consume_event(self, sensor, event):
        if event != "DC_DET_RDS_UPDATED": return
        for stream in set(DCJsonToHtmlConvertor.stream_list):
            sigs = DCJsonToHtmlConvertor.stream_signals.get(stream, set())
            for sig in sigs:
                sensors = ["FL", "FR", "RR", "RL"] if stream.startswith("Detections") else ["DC"]
                for s in sensors:
                    self._flatten(s, sig)
                    if sig in ["range", "range_rate", "azimuth", "elevation", "snr", "vcs_pos_x", "vcs_pos_y", "vcs_vel_x", "vcs_vel_y", "vcs_accel_x", "vcs_accel_y"]:
                        self._compare(sig, s)
                    self._mediator.notify_event(f"{s}#{stream}#{sig}", "JDS_UPDATED")

    def _flatten(self, sensor, sig):
        for src in ('in', 'out'):
            try:
                data = self._radar_ds.get_data(sensor, sig, src)
                rounded = (data * 100).astype(int) / 100.0
                scale = rounded.shape[1] if rounded.ndim == 2 else None
                if scale:
                    self._plot_ds.update_data(f"{sig}Dim", scale, src)
                    scan = self._radar_ds.get_data("DC", "scan_index", src)
                    self._plot_ds.update_data("DC", np.repeat(scan, scale), f"{src}_scan_index", None, sensor)
                self._plot_ds.update_data(sig, rounded.ravel(), src)
            except: pass

    def _pad(self, arr, rows, cols):
        p = np.full((rows, cols), np.nan)
        p[:arr.shape[0], :arr.shape[1]] = arr
        return p

    def _compare(self, sig, sensor):
        try:
            vehicle, resim = self._radar_ds.get_data(sensor, sig, "in"), self._radar_ds.get_data(sensor, sig, "out")
        except: return
        if vehicle is None or resim is None: return
        in_idx, out_idx = self._radar_ds.get_data("DC", "scan_index", "in"), self._radar_ds.get_data("DC", "scan_index", "out")
        tol = {"range": 0.1, "range_rate": 0.015, "elevation": 0.00873, "azimuth": 0.00873, "snr": 0, "std_rcs": 0, "vcs_pos_x": 0, "vcs_pos_y": 0, "vcs_vel_x": 0, "vcs_vel_y": 0, "vcs_accel_x": 0, "vcs_accel_y": 0}.get(sig, 0)
        common = np.intersect1d(in_idx, out_idx)
        in_pos, out_pos = np.nonzero(np.isin(in_idx, common))[0], np.nonzero(np.isin(out_idx, common))[0]
        a1, a2 = vehicle[in_pos], resim[out_pos]
        rows, cols = max(a1.shape[0], a2.shape[0]), max(a1.shape[1], a2.shape[1])
        a1_pad, a2_pad = self._pad(a1, rows, cols), self._pad(a2, rows, cols)
        mask = np.isclose(a1_pad, a2_pad, atol=tol, equal_nan=False)
        matched = np.where(mask, a1_pad, np.nan)[:, ~np.all(np.isnan(np.where(mask, a1_pad, np.nan)), axis=0)]
        mismatch = np.where(mask, np.nan, a1_pad)[:, ~np.all(np.isnan(np.where(mask, np.nan, a1_pad)), axis=0)]
        total = np.count_nonzero(~np.isnan(a1_pad))
        match_pct = (np.count_nonzero(~np.isnan(matched)) / total * 100) if total else 0
        mismatch_pct = (np.count_nonzero(~np.isnan(mismatch)) / total * 100) if total else 0
        if sig in ["range", "range_rate", "azimuth", "elevation", "snr", "vcs_pos_x", "vcs_pos_y", "vcs_vel_x", "vcs_vel_y", "vcs_accel_x", "vcs_accel_y"]:
            ReportDash.signal_stats[sensor][sig]["match"] = match_pct
            ReportDash.signal_stats[sensor][sig]["mismatch"] = mismatch_pct
