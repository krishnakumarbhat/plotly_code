import os
import json
import time
from pathlib import Path
from IPS.EventMan.ievent_mediator import IEventMediator

from collections import defaultdict

import os
import time
import json
from pathlib import Path

import shutil
import os
import os
import shutil
import stat
import platform


class ReportDash:
    '''
    signal_stats = {

        "FC": {
            "ran": {"match": 0, "mismatch": 0},
            "vel": {"match": 0, "mismatch": 0},
            "phi": {"match": 0, "mismatch": 0},
            "theta": {"match": 0, "mismatch": 0},
            "snr": {"match": 0, "mismatch": 0},
            "rcs": {"match": 0, "mismatch": 0}
        },

        "FL": {
            "ran": {"match": 0, "mismatch": 0},
            "vel": {"match": 0, "mismatch": 0},
            "phi": {"match": 0, "mismatch": 0},
            "theta": {"match": 0, "mismatch": 0},
            "snr": {"match": 0, "mismatch": 0},
            "rcs": {"match": 0, "mismatch": 0}
        }
        ,
        "FR": {
            "ran": {"match": 0, "mismatch": 0},
            "vel": {"match": 0, "mismatch": 0},
            "phi": {"match": 0, "mismatch": 0},
            "theta": {"match": 0, "mismatch": 0},
            "snr": {"match": 0, "mismatch": 0},
            "rcs": {"match": 0, "mismatch": 0}
        }
        ,
        "RL": {
            "ran": {"match": 0, "mismatch": 0},
            "vel": {"match": 0, "mismatch": 0},
            "phi": {"match": 0, "mismatch": 0},
            "theta": {"match": 0, "mismatch": 0},
            "snr": {"match": 0, "mismatch": 0},
            "rcs": {"match": 0, "mismatch": 0}
        }
        ,
        "RR": {
            "ran": {"match": 0, "mismatch": 0},
            "vel": {"match": 0, "mismatch": 0},
            "phi": {"match": 0, "mismatch": 0},
            "theta": {"match": 0, "mismatch": 0},
            "snr": {"match": 0, "mismatch": 0},
            "rcs": {"match": 0, "mismatch": 0}
        }
    }
    '''

    def create_signal_stats(self):
        return defaultdict(lambda: defaultdict(lambda: {"match": 0, "mismatch": 0}))

    def update_signal_stats(self, sensor, signal, match_count=0, mismatch_count=0):
        ReportDash.signal_stats[sensor][signal]["match"] += match_count
        ReportDash.signal_stats[sensor][signal]["mismatch"] += mismatch_count

    report_directory = None
    input_hdf = None
    output_hdf = None
    report_gen_time = None
    Version_metadata = []
    signal_stats = defaultdict(lambda: defaultdict(lambda: {"match": 0, "mismatch": 0}))

    signal_name_set = set()
    datasource_type = None

    def __init__(self, event_mediator: IEventMediator):
        # print("ReportDash")
        self._data_event_mediator_obj = event_mediator
        ReportDash.signal_stats = self.create_signal_stats()

    def clear_data(self):
        ReportDash.report_directory = None
        ReportDash.input_hdf = None
        ReportDash.output_hdf = None
        # ReportDash.report_gen_time = None

    def safe_delete_folder(self, folder_path):
        # Normalize the path
        folder_path = os.path.abspath(folder_path)
        print(f"🔍 Checking folder: {folder_path}")

        if os.path.exists(folder_path):
            if os.path.islink(folder_path):
                print(f"⚠️ '{folder_path}' is a symbolic link. Unlinking it.")
                try:
                    os.unlink(folder_path)
                    print(f"✅ Symbolic link '{folder_path}' has been removed.")
                except Exception as e:
                    print(f"❌ Failed to remove symbolic link: {e}")
            elif os.path.isdir(folder_path):
                print(f"📁 '{folder_path}' is a directory. Preparing to delete.")
                try:
                    # Make all contents writable (especially for Linux/Docker)
                    for root, dirs, files in os.walk(folder_path):
                        for d in dirs:
                            os.chmod(os.path.join(root, d), stat.S_IWUSR | stat.S_IREAD)
                        for f in files:
                            os.chmod(os.path.join(root, f), stat.S_IWUSR | stat.S_IREAD)

                    # shutil.rmtree(folder_path)
                    print(f"✅ Folder '{folder_path}' and all its contents have been deleted.")
                except PermissionError:
                    print(f"❌ Permission denied while deleting '{folder_path}'. Try running with elevated privileges.")
                except Exception as e:
                    print(f"❌ An error occurred while deleting the folder: {e}")
            else:
                print(f"⚠️ '{folder_path}' exists but is not a directory or symlink.")
        else:
            print(f"ℹ️ The folder '{folder_path}' does not exist.")

    def generate_rep(self):
        time.sleep(10)
        self.safe_delete_folder(os.path.join(ReportDash.report_directory, "JSON"))
        '''
        folder_path = os.path.join(ReportDash.report_directory, "JSON")

        # Normalize the path for cross-platform compatibility
        folder_path = os.path.abspath(folder_path)

        if os.path.exists(folder_path) and os.path.isdir(folder_path):
            try:
                shutil.rmtree(folder_path)
                print(f"✅ Folder '{folder_path}' and all its contents have been deleted.")
            except PermissionError:
                print(f"❌ Permission denied while deleting '{folder_path}'. Try running with elevated privileges.")
            except Exception as e:
                print(f"❌ An error occurred while deleting the folder: {e}")
        else:
            print(f"ℹ️ The folder '{folder_path}' does not exist.")

        # print("Generation time in dash module", ReportDash.report_gen_time)
        '''
        signal_name_list = list(ReportDash.signal_name_set)
        signal_name_list.append("range")
        signal_name_list.append("range_rate")
        signal_name_list.append("azimuth")
        signal_name_list.append("elevation")
        signal_name_list.append("snr")

        print("signal_name_list", signal_name_list)
        print("ReportDash.signal_stats", ReportDash.signal_stats)

        self.generate_dashboard_from_path(
            report_dir=ReportDash.report_directory + "/reports",
            signal_stats=ReportDash.signal_stats,
            input_hdf=ReportDash.input_hdf,
            output_hdf=ReportDash.output_hdf,
            rep_time=ReportDash.report_gen_time,
            signames=signal_name_list,
            output_html=ReportDash.report_directory + "/KPI_DashBoard.html",

        )

    def consume_event(self, sensor, event):
        # print(f"ReportDash # consume_event # event= {event} for {sensor}")
        if event == "HTML_GEN_DONE":
            # print("Consumed Event HTML_GEN_DONE in ReportDash")

            folder_path = os.path.join(ReportDash.report_directory, "JSON")

            # Normalize the path for cross-platform compatibility
            folder_path = os.path.abspath(folder_path)

            if os.path.exists(folder_path) and os.path.isdir(folder_path):
                try:
                    # shutil.rmtree(folder_path)
                    print(f"✅ Folder '{folder_path}' and all its contents have been deleted.")
                except PermissionError:
                    print(f"❌ Permission denied while deleting '{folder_path}'. Try running with elevated privileges.")
                except Exception as e:
                    print(f"❌ An error occurred while deleting the folder: {e}")
            else:
                print(f"ℹ️ The folder '{folder_path}' does not exist.")

            print("Generation time in dash module", ReportDash.report_gen_time)

            self.generate_dashboard_from_path(
                report_dir=ReportDash.report_directory + "/reports",
                signal_stats=ReportDash.signal_stats,
                input_hdf=ReportDash.input_hdf,
                output_hdf=ReportDash.output_hdf,
                rep_time=ReportDash.report_gen_time,

                output_html=ReportDash.report_directory + "/KPI_DashBoard.html",

            )

    def generate_dashboard_from_path(self, report_dir, signal_stats, input_hdf, output_hdf, rep_time,
                                     signames,

                                     output_html="dash.html",
                                     ):
        start_time = time.time()
        print("rep_time", rep_time)

        if ReportDash.datasource_type == "udpdc":
            print("version_info", ReportDash.Version_metadata)
            hdf_generate_time = ReportDash.Version_metadata[0]
            dc_version = ReportDash.Version_metadata[1]
            ocg_version = ReportDash.Version_metadata[2]
            olp_version = ReportDash.Version_metadata[3]
            sfl_version = ReportDash.Version_metadata[4]
            tracker_version = ReportDash.Version_metadata[5]

            main_keys = ['range', 'range_rate', 'azimuth', 'elevation', 'snr', 'std_rcs']
            extra_keys = ['vcs_pos_x', 'vcs_pos_y', 'vcs_vel_x', 'vcs_vel_y', 'vcs_accel_x', 'vcs_accel_y']




        else:
            dc_version = "NA"
            ocg_version = "NA"
            olp_version = "NA"
            sfl_version = "NA"
            tracker_version = "NA"
            hdf_generate_time = "NA"
            main_keys = ['ran', 'vel', 'phi', 'theta', 'snr', 'rcs']
            extra_keys = []

        report_dir = Path(report_dir)
        if not report_dir.exists():
            raise FileNotFoundError(f"Report path not found: {report_dir}")

        report_files = [f.name for f in report_dir.glob("*.html") if f.is_file()]

        dashboard_data = {}
        for file in report_files:
            parts = file.split("_")
            if len(parts) >= 3:
                sensor = parts[0]
                stream = parts[1]
                dashboard_data.setdefault(sensor, {}).setdefault(stream, []).append(file)

        signal_stats_json = json.dumps(signal_stats)
        js_signal_array = json.dumps(signames)

        html_content = f"""
        <!DOCTYPE html>
        <html lang=\"en\">
        <head>
            <meta charset=\"UTF-8\">
            <title>Sensor Report Dashboard</title>
            <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css\" rel=\"stylesheet\">
            <script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>
            <style>
                body {{ padding: 2rem; background-color: #f9f9f9; }}
                .btn-sensor {{ margin: 0.2rem; font-size: 0.85rem; }}
                .chart-box {{ width: 100%; max-width: 250px; height: 200px; }}
            </style>
        </head>
        <body>
        <div class=\"container\">
            <h2 class=\"text-center mb-4\">📊 Sensor Report Dashboard</h2>

            <div class=\"accordion mb-4\" id=\"headerAccordion\">
                <div class=\"accordion-item\">
                    <h2 class=\"accordion-header\" id=\"headingMeta\">
                        <button class=\"accordion-button collapsed\" type=\"button\" data-bs-toggle=\"collapse\" data-bs-target=\"#collapseMeta\">
                            🔧 Dashboard Metadata
                        </button>
                    </h2>
                    <div id=\"collapseMeta\" class=\"accordion-collapse collapse\">
                        <div class=\"accordion-body\">
                            <p><strong>Tool version:</strong> {"V2.0"}</p>
                            <p><strong>Input HDF File:</strong> {input_hdf}</p>
                            <p><strong>Output HDF File:</strong> {output_hdf}</p>
                            <p><strong>DC Version:</strong> {dc_version}</p>
                            <p><strong>OLP Version:</strong> {olp_version}</p>
                            <p><strong>OCG Version:</strong> {ocg_version}</p>
                            <p><strong>SFL Version:</strong> {sfl_version}</p>
                            <p><strong>Tracker Version:</strong> {tracker_version}</p>
                            <p><strong>HDF Geneartion Date:</strong> {hdf_generate_time}</p>
                            <p><strong>Report Generation time:</strong> {str(rep_time)} seconds</p>

                        </div>
                    </div>
                </div>
            </div>
        """

        for idx, (sensor, streams) in enumerate(dashboard_data.items()):
            html_content += f"""
                <div class=\"accordion mb-3\" id=\"sensorAccordion-{idx}\">
                    <div class=\"accordion-item\">
                        <h2 class=\"accordion-header\">
                            <button class=\"accordion-button collapsed\" type=\"button\" data-bs-toggle=\"collapse\" data-bs-target=\"#sensor-{idx}\">
                                {sensor} Reports
                            </button>
                        </h2>
                        <div id=\"sensor-{idx}\" class=\"accordion-collapse collapse\">
                            <div class=\"accordion-body row\">
                                <div class=\"col-md-6\">
                """
            for stream, files in streams.items():
                html_content += f"<h6>{stream.capitalize()} Stream</h6>"
                for file in files:
                    full_file_path = report_dir / file
                    relative_path = os.path.relpath(full_file_path, Path(output_html).parent)
                    # print("relative_path", relative_path)
                    html_content += f"<a href='{relative_path}' target='_blank' class='btn btn-outline-primary btn-sensor'>{file}</a>"

            html_content += f"""
                                </div>
                                <div class=\"col-md-3 d-flex justify-content-center\">
                                    <div class=\"chart-box\">
                                        <canvas id=\"chart-{sensor}\"></canvas>
                                    </div>
                                </div>
                                <div class=\"col-md-3 d-flex justify-content-center\">
                                    <div class=\"chart-box\">
                                        <canvas id=\"chart2-{sensor}\"></canvas>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                """

        html_content += f"""
        </div>
        <script>
        const signalStats = {signal_stats_json};
        const signalarray= {js_signal_array};

        function createChart(ctx, data, label1, label2) {{
            const labels = Object.keys(data);
            const matchData = labels.map(k => data[k].match);
            const mismatchData = labels.map(k => data[k].mismatch);

            new Chart(ctx, {{
                type: 'bar',
                data: {{
                    labels: labels,
                    datasets: [
                        {{ label: label1, data: matchData, backgroundColor: 'rgba(40, 167, 69, 0.7)' }},
                        {{ label: label2, data: mismatchData, backgroundColor: 'rgba(220, 53, 69, 0.7)' }}
                    ]
                }},
                options: {{
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {{ y: {{ beginAtZero: true, max: 100 }} }}
                }}
            }});
        }}

        document.addEventListener('DOMContentLoaded', () => {{
            Object.entries(signalStats).forEach(([sensor, data]) => {{
                
                
                const mainKeys = {json.dumps(main_keys)};
                const extraKeys = {json.dumps(extra_keys)};

                const mainData = Object.fromEntries(Object.entries(data).filter(([k, _]) => mainKeys.includes(k)));
                const extraData = Object.fromEntries(Object.entries(data).filter(([k, _]) => extraKeys.includes(k)));

                const ctx1 = document.getElementById(`chart-${{sensor}}`).getContext('2d');
                createChart(ctx1, mainData, 'Match %', 'Mismatch %');

                if (Object.keys(extraData).length > 0) {{
                    const ctx2 = document.getElementById(`chart2-${{sensor}}`).getContext('2d');
                    createChart(ctx2, extraData, 'Match %', 'Mismatch %');
                }}
            }});
        }});
        </script>
        <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js\"></script>
        </body>
        </html>
        """

        with open(output_html, "w", encoding="utf-8") as f:
            f.write(html_content)

        print(f"✅ HTML dashboard saved as: {output_html}")
