import os, time, json, stat, shutil
from pathlib import Path
from collections import defaultdict
from IPS.EventMan.ievent_mediator import IEventMediator

class ReportDash:
    report_directory, input_hdf, output_hdf, report_gen_time = None, None, None, None
    Version_metadata, signal_name_set, datasource_type = [], set(), None
    signal_stats = defaultdict(lambda: defaultdict(lambda: {"match": 0, "mismatch": 0}))

    def __init__(self, mediator: IEventMediator):
        self._mediator = mediator
        ReportDash.signal_stats = defaultdict(lambda: defaultdict(lambda: {"match": 0, "mismatch": 0}))

    def update_signal_stats(self, sensor, signal, match=0, mismatch=0):
        ReportDash.signal_stats[sensor][signal]["match"] += match
        ReportDash.signal_stats[sensor][signal]["mismatch"] += mismatch

    def clear_data(self):
        ReportDash.report_directory = ReportDash.input_hdf = ReportDash.output_hdf = None

    def _safe_delete(self, path):
        path = os.path.abspath(path)
        if os.path.islink(path): os.unlink(path)
        elif os.path.isdir(path):
            for r, ds, fs in os.walk(path):
                for d in ds: os.chmod(os.path.join(r, d), stat.S_IWUSR | stat.S_IREAD)
                for f in fs: os.chmod(os.path.join(r, f), stat.S_IWUSR | stat.S_IREAD)

    def generate_rep(self):
        time.sleep(10)
        self._safe_delete(os.path.join(ReportDash.report_directory, "JSON"))
        sigs = list(ReportDash.signal_name_set) + ["range", "range_rate", "azimuth", "elevation", "snr"]
        self._gen_dashboard(ReportDash.report_directory + "/reports", ReportDash.signal_stats, ReportDash.input_hdf, ReportDash.output_hdf, ReportDash.report_gen_time, sigs, ReportDash.report_directory + "/KPI_DashBoard.html")

    def consume_event(self, sensor, event):
        if event != "HTML_GEN_DONE": return
        self._gen_dashboard(ReportDash.report_directory + "/reports", ReportDash.signal_stats, ReportDash.input_hdf, ReportDash.output_hdf, ReportDash.report_gen_time, [], ReportDash.report_directory + "/KPI_DashBoard.html")

    def _gen_dashboard(self, report_dir, stats, in_hdf, out_hdf, rep_time, sigs, out_html):
        meta = ReportDash.Version_metadata if ReportDash.datasource_type == "udpdc" else ["NA"]*6
        hdf_time, dc_ver, ocg_ver, olp_ver, sfl_ver, trk_ver = (meta + ["NA"]*6)[:6]
        main_keys = ['range', 'range_rate', 'azimuth', 'elevation', 'snr', 'std_rcs'] if ReportDash.datasource_type == "udpdc" else ['ran', 'vel', 'phi', 'theta', 'snr', 'rcs']
        extra_keys = ['vcs_pos_x', 'vcs_pos_y', 'vcs_vel_x', 'vcs_vel_y', 'vcs_accel_x', 'vcs_accel_y'] if ReportDash.datasource_type == "udpdc" else []
        report_path = Path(report_dir)
        if not report_path.exists(): return
        files = [f.name for f in report_path.glob("*.html") if f.is_file()]
        dashboard = {}
        for f in files:
            parts = f.split("_")
            if len(parts) >= 3: dashboard.setdefault(parts[0], {}).setdefault(parts[1], []).append(f)
        html = f'''<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Dashboard</title><link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet"><script src="https://cdn.jsdelivr.net/npm/chart.js"></script><style>body{{padding:2rem;background:#f9f9f9}}.chart-box{{width:100%;max-width:250px;height:200px}}</style></head><body><div class="container"><h2 class="text-center mb-4">Sensor Report Dashboard</h2><div class="accordion mb-4" id="hA"><div class="accordion-item"><h2 class="accordion-header"><button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#cM">Metadata</button></h2><div id="cM" class="accordion-collapse collapse"><div class="accordion-body"><p><b>Tool:</b> V2.0</p><p><b>Input:</b> {in_hdf}</p><p><b>Output:</b> {out_hdf}</p><p><b>DC:</b> {dc_ver}</p><p><b>OLP:</b> {olp_ver}</p><p><b>OCG:</b> {ocg_ver}</p><p><b>SFL:</b> {sfl_ver}</p><p><b>Tracker:</b> {trk_ver}</p><p><b>HDF Time:</b> {hdf_time}</p><p><b>Report Time:</b> {rep_time}s</p></div></div></div></div>'''
        for i, (sensor, streams) in enumerate(dashboard.items()):
            html += f'<div class="accordion mb-3" id="sA-{i}"><div class="accordion-item"><h2 class="accordion-header"><button class="accordion-button collapsed" type="button" data-bs-toggle="collapse" data-bs-target="#s-{i}">{sensor} Reports</button></h2><div id="s-{i}" class="accordion-collapse collapse"><div class="accordion-body row"><div class="col-md-6">'
            for stream, fs in streams.items():
                html += f"<h6>{stream.capitalize()}</h6>"
                for f in fs:
                    rel = os.path.relpath(report_path / f, Path(out_html).parent)
                    html += f"<a href='{rel}' target='_blank' class='btn btn-outline-primary btn-sm m-1'>{f}</a>"
            html += f'</div><div class="col-md-3 d-flex justify-content-center"><div class="chart-box"><canvas id="c-{sensor}"></canvas></div></div><div class="col-md-3 d-flex justify-content-center"><div class="chart-box"><canvas id="c2-{sensor}"></canvas></div></div></div></div></div></div>'
        html += f'''</div><script>const S={json.dumps(stats)},M={json.dumps(main_keys)},E={json.dumps(extra_keys)};function C(c,d,l1,l2){{const L=Object.keys(d),m=L.map(k=>d[k].match),x=L.map(k=>d[k].mismatch);new Chart(c,{{type:'bar',data:{{labels:L,datasets:[{{label:l1,data:m,backgroundColor:'rgba(40,167,69,0.7)'}},{{label:l2,data:x,backgroundColor:'rgba(220,53,69,0.7)'}}]}},options:{{responsive:true,maintainAspectRatio:false,scales:{{y:{{beginAtZero:true,max:100}}}}}}}})}}document.addEventListener('DOMContentLoaded',()=>{{Object.entries(S).forEach(([s,d])=>{{const mD=Object.fromEntries(Object.entries(d).filter(([k])=>M.includes(k))),eD=Object.fromEntries(Object.entries(d).filter(([k])=>E.includes(k)));C(document.getElementById(`c-${{s}}`).getContext('2d'),mD,'Match %','Mismatch %');if(Object.keys(eD).length>0)C(document.getElementById(`c2-${{s}}`).getContext('2d'),eD,'Match %','Mismatch %')}})}})</script><script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script></body></html>'''
        with open(out_html, "w", encoding="utf-8") as f: f.write(html)
        print(f"Dashboard: {out_html}")
