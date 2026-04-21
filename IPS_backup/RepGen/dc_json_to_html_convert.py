"""
File Name: json_to_html_convert.py
Author: Bharanidharan Subramani
Email : Bharanidharan.s@aptiv.com
Description:
This module converts DC json to html
"""

from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore

from IPS.DashManager.report_dash import ReportDash
import numpy as np
import plotly.io as pio
from html import escape
import json
import os, string
import re
from collections import defaultdict


class DCJsonToHtmlConvertor:
    output_scan_index_scaled = np.array([])
    input_scan_index_scaled = np.array([])
    list_json_path = []
    sensor_list = []
    stream_list = []
    stream_signals = defaultdict(set)
    sensor_stream = defaultdict(set)
    mismatch_input_value = np.array([])
    mismatch_output_value = np.array([])
    mismatch_input_scan_index = np.array([])
    mismatch_output_scan_index = np.array([])

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 jsonpath_datastore: IDataStore,
                 report_dash: ReportDash,
                 ):
        self._data_event_mediator_obj = event_mediator
        self._radar_datastore_obj = radar_datastore
        self._plot_datastore_obj = plot_datastore
        self._jsonpath_datastore_obj = jsonpath_datastore
        self._report_dash = report_dash


    def clear_data(self):
        pass

    def is_substring_match_in_longstring(self, substring, long_string_list):
        return any(substring.casefold() in long.casefold() for long in long_string_list)

    def filter_strings_with_substring(self, substring, long_string_list):
        return [s for s in long_string_list if substring.casefold() in s.casefold()]

    def filter_json_by_word_in_name(self, substring, json_list):
        lowered_substring = substring.casefold()
        result = []
        for json_path in json_list:
            filename = os.path.splitext(os.path.basename(json_path))[0]
            words = filename.casefold().split('_')
            if lowered_substring in words:
                result.append(json_path)
        return result

    def filter_strings_with_exact_match(self, substring, json_list):
        lowered_substring = substring.casefold()
        result = []
        for path in json_list:
            # Extract the file name without extension
            file_name = os.path.splitext(os.path.basename(path))[0]
            # Split into words, strip punctuation
            words = [word.strip(string.punctuation) for word in file_name.casefold().split()]
            if lowered_substring in words:
                result.append(path)
        return result

    def consume_event(self, sensor, event):

        if event == "DC_JSON_GEN_DONE":
            self.trigger_json_to_html_conversion()

    def trigger_json_to_html_conversion(self):

        for sensor in set(DCJsonToHtmlConvertor.sensor_list):
            json_list = self._jsonpath_datastore_obj.get_data(sensor, "scatter")
            for stream in set(DCJsonToHtmlConvertor.sensor_stream.get(sensor, set())):
                stream = stream.replace("_", "")
                if stream == "Detections" or stream == "Detections1" or stream == "Detections2":
                    filtered_list = [f for f in json_list if stream in f.split('_')]
                    json_list_filtered_detection = self.filter_json_by_word_in_name(stream, filtered_list)

                    if json_list_filtered_detection:
                        sensor = sensor.replace("_", "")
                        stream = stream.replace("_", "")
                        self.generate_plotly_html_report_new4(json_list_filtered_detection,
                                                              sensor + "_" + stream + "_scatter.html")
                else:
                    json_list_filtered = self.filter_json_by_word_in_name(stream, json_list)
                    sensor = "DC"
                    stream = stream.replace("_", "")
                    self.generate_plotly_html_report_new4(json_list_filtered,
                                                          sensor + "_" + stream + "_scatter.html")

            json_list = self._jsonpath_datastore_obj.get_data(sensor, "Histogram")
            for stream in set(DCJsonToHtmlConvertor.sensor_stream.get(sensor, set())):
                stream = stream.replace("_", "")
                if stream == "Detections" or stream == "Detections1" or stream == "Detections2":
                    filtered_list = [f for f in json_list if stream in f.split('_')]
                    json_list_filtered_detection = self.filter_json_by_word_in_name(stream, filtered_list)

                    if json_list_filtered_detection:
                        sensor = sensor.replace("_", "")
                        stream = stream.replace("_", "")
                        self.generate_plotly_html_report_new4(json_list_filtered_detection,
                                                              sensor + "_" + stream + "_histogram.html")
                else:
                    json_list_filtered = self.filter_json_by_word_in_name(stream, json_list)
                    sensor = "DC"
                    stream = stream.replace("_", "")
                    self.generate_plotly_html_report_new4(json_list_filtered,
                                                          sensor + "_" + stream + "_histogram.html")

    def generate_plotly_html_report_new4(self, json_paths, output_html_path):
        # print("generate_plotly_html_report_new4")
        html_parts = [
            "<html><head><meta charset='UTF-8'>",
            "<title>Plotly Report</title>",
            "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>",
            "<style>",
            "body { font-family: sans-serif; background: #f9f9f9; padding: 20px; margin: 0; }",
            ".grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 20px; }",
            ".card { background: white; padding: 10px; border-radius: 6px;",
            "box-shadow: 0 2px 4px rgba(0,0,0,0.1); overflow-x: auto; min-width: 0; }",
            ".title { font-weight: bold; margin-bottom: 8px; font-size: 15px; text-align: center; }",
            "@media (max-width: 1200px) { .grid { grid-template-columns: 1fr; } }",
            "</style></head><body>",
            "<h1>Plotly Interactive Chart Report</h1>",
            "<div class='grid'>"
        ]

        for path in json_paths:
            if not os.path.isfile(path):
                html_parts.append(
                    f"<div class='card'><div class='title'>{path}</div><p style='color:red;'>File not found</p></div>")
                continue

            try:
                fig = pio.read_json(path)

                # Move legend below plot to avoid side squeeze
                fig.update_layout(
                    autosize=True,
                    margin=dict(l=50, r=50, t=50, b=80),
                    legend=dict(orientation='h', y=-0.25),
                    height=500
                )

                fig_html = pio.to_html(
                    fig,
                    include_plotlyjs=False,
                    full_html=False,
                    config={"responsive": True},
                    default_width="100%",
                    default_height="500px"
                )

                # title = os.path.basename(path)
                title = os.path.splitext(os.path.basename(path))[0]
                html_parts.append(f"<div class='card'><div class='title'>{title}</div>{fig_html}</div>")
            except Exception as e:
                html_parts.append(
                    f"<div class='card'><div class='title'>{path}</div><p style='color:red;'>Error: {e}</p></div>")

        html_parts.append("</div></body></html>")

        report_folder = ReportDash.report_directory + "/reports"
        os.makedirs(report_folder, exist_ok=True)
        output_html_path = os.path.join(report_folder, output_html_path)
        with open(output_html_path, 'w') as f:
            f.write('\n'.join(html_parts))

        print(f"✅ Final Plotly HTML report created: {output_html_path}")

    def generate_plotly_html_report_new5(self, json_paths, output_html_path):
        html_parts = [
            "<html><head><meta charset='UTF-8'>",
            "<title>Plotly Report</title>",
            "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>",
            "<style>",
            "body { font-family: sans-serif; background: #f9f9f9; padding: 20px; margin: 0; }",
            ".grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 20px; }",
            ".card { background: white; padding: 10px; border-radius: 6px;",
            "box-shadow: 0 2px 4px rgba(0,0,0,0.1); overflow-x: auto; min-width: 0; }",
            ".title { font-weight: bold; margin-bottom: 8px; font-size: 15px; text-align: center; }",
            "@media (max-width: 1200px) { .grid { grid-template-columns: 1fr; } }",
            "</style></head><body>",
            "<h1>Plotly Interactive Chart Report</h1>",
            "<div class='grid'>"
        ]

        for path in json_paths:
            if not os.path.isfile(path):
                html_parts.append(
                    f"<div class='card'><div class='title'>{path}</div><p style='color:red;'>File not found</p></div>")
                continue

            try:
                fig = pio.read_json(path)

                # Move legend below plot to avoid side squeeze
                fig.update_layout(
                    autosize=True,
                    margin=dict(l=50, r=50, t=50, b=80),
                    legend=dict(orientation='h', y=-0.25),
                    height=500
                )

                fig_html = pio.to_html(
                    fig,
                    include_plotlyjs=False,
                    full_html=False,
                    config={"responsive": True},
                    default_width="100%",
                    default_height="500px"
                )

                title = os.path.basename(path)
                html_parts.append(f"<div class='card'><div class='title'>{title}</div>{fig_html}</div>")
            except Exception as e:
                html_parts.append(
                    f"<div class='card'><div class='title'>{path}</div><p style='color:red;'>Error: {e}</p></div>")

        html_parts.append("</div></body></html>")

        report_folder = ReportDash.report_directory + "/reports"
        os.makedirs(report_folder, exist_ok=True)
        output_html_path = os.path.join(report_folder, output_html_path)
        with open(output_html_path, 'w') as f:
            f.write('\n'.join(html_parts))

        # print(f"✅ Final Plotly HTML report created: {output_html_path}")
