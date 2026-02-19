import sys
import os
import re
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
import json
import logging
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage

# Global variables (should be initialized before use)
html_content = ""
az_misalign_threshold = 1.0  # Example value, set appropriately
el_misalign_threshold = 1.0  # Example value, set appropriately
max_num_of_si_to_process = 0  # 0 means process all
file_suffix = "_veh.csv"  # Example suffix, set appropriately

def process_alignment_kpi(input_data, ):
    global html_content
    global az_misalign_threshold, el_misalign_threshold
    global max_num_of_si_to_process
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, el_misalign_est_diff_list

    print("[INFO] Reading the CSVs...")

    # Get number of scans in both real and sim
    data_dict = input_data
    number_of_scans_in_both_real_and_sim = data_dict['vacs_boresight_az_nominal']["SI"]
    # if number_of_scans_in_both_real_and_sim == 0:
    #     print("[WARNING] Filtered vehicle data is empty")
    #     return False



    signals = [
        "vacs_boresight_az_nominal",
        "vacs_boresight_az_estimated", 
        "vacs_boresight_az_kf_internal",
        "vacs_boresight_el_nominal",
        "vacs_boresight_el_estimated",
        "vacs_boresight_el_kf_internal"
    ]
    final_df = {}
    for signal in signals:
        if signal in data_dict:
            final_df[f"{signal}_real"] = data_dict[f"{signal}"]['I']
            final_df[f"{signal}_sim"] = data_dict[f"{signal}"]['O']
        else:
            final_df[f"{signal}_real"] = None
            final_df[f"{signal}_sim"] = None


    # Calculate misalignments in degrees
    final_df["az_misalign_est_real"] = (
        final_df["vacs_boresight_az_nominal_real"] - final_df["vacs_boresight_az_estimated_real"]
    ) * (180 / np.pi)
    final_df["az_misalign_ref_real"] = (
        final_df["vacs_boresight_az_nominal_real"] - final_df["vacs_boresight_az_kf_internal_real"]
    ) * (180 / np.pi)
    final_df["az_misalign_est_sim"] = (
        final_df["vacs_boresight_az_nominal_sim"] - final_df["vacs_boresight_az_estimated_sim"]
    ) * (180 / np.pi)
    final_df["az_misalign_ref_sim"] = (
        final_df["vacs_boresight_az_nominal_sim"] - final_df["vacs_boresight_az_kf_internal_sim"]
    ) * (180 / np.pi)
    final_df["el_misalign_est_real"] = (
        final_df["vacs_boresight_el_nominal_real"] - final_df["vacs_boresight_el_estimated_real"]
    ) * (180 / np.pi)
    final_df["el_misalign_ref_real"] = (
        final_df["vacs_boresight_el_nominal_real"] - final_df["vacs_boresight_el_kf_internal_real"]
    ) * (180 / np.pi)
    final_df["el_misalign_est_sim"] = (
        final_df["vacs_boresight_el_nominal_sim"] - final_df["vacs_boresight_el_estimated_sim"]
    ) * (180 / np.pi)
    final_df["el_misalign_ref_sim"] = (
        final_df["vacs_boresight_el_nominal_sim"] - final_df["vacs_boresight_el_kf_internal_sim"]
    ) * (180 / np.pi)

    # Calculate differences
    final_df["az_misalign_est_diff_real"] = final_df["az_misalign_est_real"] - final_df["az_misalign_est_sim"]
    final_df["el_misalign_est_diff_real"] = final_df["el_misalign_est_real"] - final_df["el_misalign_est_sim"]

    # Count matches within thresholds
    number_of_scans_with_matching_az_misalign = len(
        final_df[np.abs(final_df["az_misalign_est_diff_real"]) < az_misalign_threshold]
    )
    number_of_scans_with_matching_el_misalign = len(
        final_df[np.abs(final_df["el_misalign_est_diff_real"]) < el_misalign_threshold]
    )

    # Extract lists for plotting
    az_misalign_est_real_list = final_df["az_misalign_est_real"].tolist()
    az_misalign_ref_real_list = final_df["az_misalign_ref_real"].tolist()
    az_misalign_est_sim_list = final_df["az_misalign_est_sim"].tolist()
    az_misalign_ref_sim_list = final_df["az_misalign_ref_sim"].tolist()
    el_misalign_est_real_list = final_df["el_misalign_est_real"].tolist()
    el_misalign_ref_real_list = final_df["el_misalign_ref_real"].tolist()
    el_misalign_est_sim_list = final_df["el_misalign_est_sim"].tolist()
    el_misalign_ref_sim_list = final_df["el_misalign_ref_sim"].tolist()
    az_misalign_est_diff_list = final_df["az_misalign_est_diff_real"].tolist()
    el_misalign_est_diff_list = final_df["el_misalign_est_diff_real"].tolist()

    # KPI calculation
    kpis_align = {
        'result1': {
            'numerator': number_of_scans_with_matching_az_misalign,
            'denominator': len(final_df),
            'value': round((number_of_scans_with_matching_az_misalign / len(final_df)) * 100, 2)
        },
        'result2': {
            'numerator': number_of_scans_with_matching_el_misalign,
            'denominator': len(final_df),
            'value': round((number_of_scans_with_matching_el_misalign / len(final_df)) * 100, 2)
        }
    }
    # HTML Content for KPI
    html_content += f"""
        <b>KPI:</b> Az Accuracy: ({kpis_align['result1']['numerator']}/{kpis_align['result1']['denominator']}) --> <b>{kpis_align['result1']['value']}%</b>
        El Accuracy: ({kpis_align['result2']['numerator']}/{kpis_align['result2']['denominator']}) --> <b>{kpis_align['result2']['value']}%</b>
        <details>
            <br>
        """
    return True

def plot_stats():
    global html_content
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, el_misalign_est_diff_list

    # Plot misalignment vs scan index
    fig_line = sp.make_subplots(rows=2, cols=1, vertical_spacing=0.2)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=az_misalign_est_real_list, mode="lines", name="Veh Az Est", line=dict(color="blue")), row=1, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list,y=az_misalign_ref_real_list, mode="lines", name="Veh Az Ref", line=dict(color="blue"), opacity=0.2), row=1, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=az_misalign_est_sim_list, mode="lines", name="Sim Az Estd", line=dict(color="red")), row=1, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=az_misalign_ref_sim_list, mode="lines", name="Sim Az Ref", line=dict(color="red"), opacity=0.2), row=1, col=1)
    fig_line.update_yaxes(title_text="Azimuth misalignment", row=1, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=1, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=el_misalign_est_real_list, mode="lines", name="Veh El Est", line=dict(color="blue")), row=2, col=1)
    fig_line.add_trace(go.Scatter( x=scan_index_list, y=el_misalign_ref_real_list, mode="lines", name="Veh El Ref", line=dict(color="blue"), opacity=0.2), row=2, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=el_misalign_est_sim_list, mode="lines", name="Sim El Est", line=dict(color="red")), row=2, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=el_misalign_ref_sim_list, mode="lines", name="Sim El Ref", line=dict(color="red"), opacity=0.2), row=2, col=1)
    fig_line.update_yaxes(title_text="Elevation misalignment", row=2, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=2, col=1)
    # Update layout for bar plot
    fig_line.update_layout(height=1000, width=1250, title_text="Misalignment v/s scanindex", showlegend=True)
    fig_line.update_traces(marker_color="red")
    fig_line.update_xaxes(zeroline=False, showgrid=False, type="category")
    fig_line.update_yaxes(zeroline=False, showgrid=False)
    misalign_line_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs="cdn")

    # Plot misalignment difference vs scan index
    fig_diff = sp.make_subplots(rows=2, cols=1, horizontal_spacing=0.04, vertical_spacing=0.2)
    fig_diff.add_trace(go.Scatter(x=scan_index_list, y=az_misalign_est_diff_list, mode="lines", name="Az diff", line=dict(color="blue")), row=1, col=1)
    fig_diff.update_yaxes(title_text="Azimuth misalignment diff", row=1, col=1)
    fig_diff.update_xaxes(title_text="Scan Index", row=1, col=1)
    fig_diff.add_trace(go.Scatter(x=scan_index_list, y=el_misalign_est_diff_list, mode="lines", name="El diff", line=dict(color="blue")), row=2, col=1)
    fig_diff.update_yaxes(title_text="Elevation misalignment diff", row=2, col=1)
    fig_diff.update_xaxes(title_text="Scan Index", row=2, col=1)
    fig_diff.update_layout(height=1000, width=1250, title_text="Misalignment difference v/s scanindex", showlegend=True)
    fig_diff.update_xaxes(zeroline=False, showgrid=False, type="category")
    fig_diff.update_yaxes(zeroline=False, showgrid=False)
    misalign_diff_plot_html = pio.to_html(fig_diff, full_html=False, include_plotlyjs="cdn")

    # HTML Content for plots
    html_content += f"""
        <b><u>Plots:</u></b>
        <details>
            <summary><i>Plot A</i></summary>
            {misalign_line_plot_html}
        </details>
        <details>
            <summary><i>Plot B</i></summary>
            {misalign_diff_plot_html}
        </details>
    </details>
    <hr>
    </body>
    </html>
    """

def find_file_pairs(base_path):
    resim_version_pattern = re.compile(r'_r\d{8}_')
    input_output_file_dict = {}
    for filename in os.listdir(base_path):
        if resim_version_pattern.search(filename):
            base_name = resim_version_pattern.sub('_', filename)
            input_output_file_dict[base_name] = filename
    return input_output_file_dict

def find_data_files(base_path):
    data_files = {'input': [], 'output': []}
    file_pair_dict = find_file_pairs(base_path)
    input_files = list(file_pair_dict.keys())
    files_of_interest = [s for s in input_files if file_suffix in s]
    unique_input_files = set(s.replace(file_suffix, "").strip() for s in files_of_interest)
    for filename in sorted(unique_input_files):
        align_filename = filename + file_suffix
        input_file_path = os.path.join(base_path, align_filename)
        if(align_filename in file_pair_dict.keys()):
            output_file_path = os.path.join(base_path, file_pair_dict[align_filename])
            data_files['input'].append(input_file_path)
            data_files['output'].append(output_file_path)
    return data_files

def process_logs(data_files):
    global html_content
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, el_misalign_est_diff_list

    veh_csv_list = []
    sim_csv_list = []
    num_of_logs = len(data_files['input'])
    for i in range(num_of_logs):
        input_file = data_files['input'][i]
        output_file = data_files['output'][i]
        veh_csv_list.append(input_file)
        sim_csv_list.append(output_file)

    for sensor_pos in ['FC', 'FL', 'FR', 'RL', 'RR']:
        veh_csv_sp_list = [veh_csv for veh_csv in veh_csv_list if sensor_pos in veh_csv]
        sim_csv_sp_list = [sim_csv for sim_csv in sim_csv_list if sensor_pos in sim_csv]
        if veh_csv_sp_list and sim_csv_sp_list:
            file_name = os.path.basename(veh_csv_sp_list[0])
            log_name_wo_ext = file_name.replace(file_suffix, "")
            html_content += f"""
                <b>Log:</b> {log_name_wo_ext}({len(veh_csv_sp_list)} logs)
                """
            # Reset variables for each sensor position
            scan_index_list = []
            az_misalign_est_real_list = []
            az_misalign_ref_real_list = []
            az_misalign_est_sim_list = []
            az_misalign_ref_sim_list = []
            el_misalign_est_real_list = []
            el_misalign_ref_real_list = []
            el_misalign_est_sim_list = []
            el_misalign_ref_sim_list = []
            az_misalign_est_diff_list = []
            el_misalign_est_diff_list = []
            status = process_alignment_kpi(veh_csv_sp_list, sim_csv_sp_list, None)
            if status:
                plot_stats()
            else:
                html_content += """
                    <b>NA</b>
                    <hr>
                    """
