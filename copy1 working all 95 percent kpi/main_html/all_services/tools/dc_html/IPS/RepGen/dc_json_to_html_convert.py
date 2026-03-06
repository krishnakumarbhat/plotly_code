import os, string
import plotly.io as pio
from collections import defaultdict
from IPS.DashManager.report_dash import ReportDash
from IPS.DataStore.idatastore import IDataStore
from IPS.EventMan.ievent_mediator import IEventMediator

class DCJsonToHtmlConvertor:
    sensor_list, stream_list = [], []
    stream_signals = defaultdict(set)
    sensor_stream = defaultdict(set)

    def __init__(self, mediator: IEventMediator, radar_ds: IDataStore, plot_ds: IDataStore, json_ds: IDataStore, report_dash: ReportDash):
        self._mediator, self._radar_ds, self._plot_ds, self._json_ds, self._report_dash = mediator, radar_ds, plot_ds, json_ds, report_dash

    def clear_data(self): pass
    def _filter_word(self, sub, lst): return [p for p in lst if sub.casefold() in os.path.splitext(os.path.basename(p))[0].casefold().split('_')]

    def consume_event(self, sensor, event):
        if event != "DC_JSON_GEN_DONE": return
        self._convert()

    def _convert(self):
        for sensor in set(DCJsonToHtmlConvertor.sensor_list):
            scatter = self._json_ds.get_data(sensor, "scatter")
            hist = self._json_ds.get_data(sensor, "Histogram")
            for stream in set(DCJsonToHtmlConvertor.sensor_stream.get(sensor, set())):
                stream_clean = stream.replace("_", "")
                if stream_clean in ["Detections", "Detections1", "Detections2"]:
                    f_sc = self._filter_word(stream_clean, [f for f in scatter if stream_clean in f.split('_')])
                    if f_sc: self._gen_html(f_sc, f"{sensor.replace('_', '')}_{stream_clean}_scatter.html")
                    f_h = self._filter_word(stream_clean, [f for f in hist if stream_clean in f.split('_')])
                    if f_h: self._gen_html(f_h, f"{sensor.replace('_', '')}_{stream_clean}_histogram.html")
                else:
                    f_sc = self._filter_word(stream_clean, scatter)
                    self._gen_html(f_sc, f"DC_{stream_clean}_scatter.html")
                    f_h = self._filter_word(stream_clean, hist)
                    self._gen_html(f_h, f"DC_{stream_clean}_histogram.html")

    def _gen_html(self, paths, out):
        html = ['<html><head><meta charset="UTF-8"><title>Report</title><script src="https://cdn.plot.ly/plotly-latest.min.js"></script><style>body{font-family:sans-serif;background:#f9f9f9;padding:20px}.grid{display:grid;grid-template-columns:repeat(2,1fr);gap:20px}.card{background:white;padding:10px;border-radius:6px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}.title{font-weight:bold;margin-bottom:8px;text-align:center}@media(max-width:1200px){.grid{grid-template-columns:1fr}}</style></head><body><h1>Report</h1><div class="grid">']
        for p in paths:
            if not os.path.isfile(p): continue
            try:
                fig = pio.read_json(p)
                fig.update_layout(autosize=True, margin=dict(l=50, r=50, t=50, b=80), legend=dict(orientation='h', y=-0.25), height=500)
                fig_html = pio.to_html(fig, include_plotlyjs=False, full_html=False, config={"responsive": True}, default_width="100%", default_height="500px")
                html.append(f'<div class="card"><div class="title">{os.path.splitext(os.path.basename(p))[0]}</div>{fig_html}</div>')
            except Exception as e:
                html.append(f'<div class="card"><div class="title">{p}</div><p style="color:red;">Error: {e}</p></div>')
        html.append('</div></body></html>')
        folder = os.path.join(ReportDash.report_directory, "reports")
        os.makedirs(folder, exist_ok=True)
        with open(os.path.join(folder, out), 'w') as f: f.write('\n'.join(html))
        print(f"Report: {out}")
