import gc, os
import plotly.graph_objects as go
import matplotlib.pyplot as plt
from IPS.DashManager.report_dash import ReportDash
from IPS.DataPrep.plot_dataprep import PlotDataPreparation
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.icollectdata import IDataCollect
from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.Sig_Prep.isig_observer import ISigObserver
from IPS.Sig_Prep.sig_filter import SignalFilter

class DCDataToJSONConvert(ISigObserver):
    def __init__(self, mediator: IEventMediator, radar_ds: IDataStore, plot_ds: IDataStore, json_ds: IDataStore, plot_prep: PlotDataPreparation, sig_fil: SignalFilter, dc_collector: IDataCollect):
        self._mediator = mediator
        self._radar_ds, self._plot_ds, self._json_ds = radar_ds, plot_ds, json_ds
        self._sig_fil, self._dc_collector = sig_fil, dc_collector

    def final_poi_sensor_datapath(self, d): pass

    def consume_event(self, info, event):
        if event != "JDS_UPDATED": return
        parts = info.split('#')
        if parts[1] != 'scan_index':
            self._generate(parts[0], parts[1], parts[2])

    def _generate(self, sensor, stream, sig):
        unit = self._dc_collector.get_unit(sig)
        self._scatter(sensor, stream, sig, unit)
        self._histogram(sensor, stream, sig)

    def _write_fig(self, fig, sensor, stream, sig, suffix, ds_type):
        folder = os.path.join(ReportDash.report_directory, "JSON", sensor)
        os.makedirs(folder, exist_ok=True)
        stream = stream.split('/')[-2] if '/' in stream else stream.replace("_", "")
        path = os.path.join(folder, f"{sensor}_{stream}_{suffix}_{sig}.json")
        PlotDataPreparation.list_json_path.append(os.path.basename(path))
        self._json_ds.update_data(sensor, path, ds_type)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(fig.to_json())
        gc.collect()

    def _histogram(self, sensor, stream, sig):
        in_data, out_data = self._plot_ds.get_data(sig, "in"), self._plot_ds.get_data(sig, "out")
        if in_data is None and out_data is None: return
        fig = go.Figure([go.Histogram(x=in_data.tolist() if in_data is not None else None, opacity=0.5, name='input', marker_color='blue'),
                         go.Histogram(x=out_data.tolist() if out_data is not None else None, opacity=0.5, name='output', marker_color='red')])
        fig.update_layout(barmode='overlay', title='Histogram', xaxis_title='Value', yaxis_title='Count')
        self._write_fig(fig, sensor, stream, sig, "histogram", "Histogram")

    def _scatter(self, sensor, stream, sig, unit):
        if sig == "scan_index": return
        in_data = self._plot_ds.get_data(sig, "in")
        out_data = self._plot_ds.get_data(sig, "out")
        in_idx = self._plot_ds.get_data("DC", "in_scan_index", sensor) if in_data is not None else None
        out_idx = self._plot_ds.get_data("DC", "out_scan_index", sensor) if out_data is not None else None
        fig = go.Figure()
        fig.add_trace(go.Scattergl(x=in_idx.tolist() if in_idx is not None else None, y=in_data.tolist() if in_data is not None else None, mode='markers', marker=dict(size=2, opacity=0.5, color="blue"), name="input"))
        fig.add_trace(go.Scattergl(x=out_idx.tolist() if out_idx is not None else None, y=out_data.tolist() if out_data is not None else None, mode='markers', marker=dict(size=2, opacity=0.5, color="red"), name="output"))
        fig.update_layout(xaxis_title="Scan Index (ms)", yaxis_title=str(unit))
        self._write_fig(fig, sensor, stream, sig, "scatter", "scatter")
        if ReportDash.datasource_type == "udpdc":
            self._save_png(sensor, stream, sig, unit, in_idx, in_data, out_idx, out_data)

    def _save_png(self, sensor, stream, sig, unit, x1, y1, x2, y2):
        folder = os.path.join(ReportDash.report_directory, "image", sensor)
        os.makedirs(folder, exist_ok=True)
        stream = stream.split('/')[-2] if '/' in stream else stream.replace("_", "")
        path = os.path.join(folder, f"{sensor}_{stream}_scatter_{sig}.png")
        try:
            fig_m, ax = plt.subplots(figsize=(8, 5), dpi=150)
            if y1 is not None: ax.scatter(x1 if x1 is not None else range(len(y1)), y1, s=2, alpha=0.5, c="blue", label="input", rasterized=True)
            if y2 is not None: ax.scatter(x2 if x2 is not None else range(len(y2)), y2, s=2, alpha=0.5, c="red", label="output", rasterized=True)
            ax.set_xlabel("Scan Index (ms)")
            ax.set_ylabel(str(unit))
            ax.legend(loc="best")
            ax.grid(True, linestyle="--", alpha=0.4)
            fig_m.tight_layout()
            fig_m.savefig(path, bbox_inches="tight")
            plt.close(fig_m)
        except Exception as e:
            print(f"PNG export failed: {e}")
