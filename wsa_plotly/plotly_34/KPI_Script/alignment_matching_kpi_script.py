import sys
import os
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio

def main(veh_csv_list, sim_csv_list):
    #################################
    # Global variables
    #################################
    global html_content
    global az_misalign_threshold, el_misalign_threshold
    global number_of_scans_in_real, number_of_scans_in_sim, number_of_scans_in_both_real_and_sim
    global max_num_of_si_to_process
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, el_misalign_est_diff_list
    #################################

    print("Reading the CSVs...")
    #################################
    # Read Vehicle and Resim data
    #################################
    cols_of_interest = [
        "scan_index",
        "vacs_boresight_az_nominal",
        "vacs_boresight_az_kf_internal",
        "vacs_boresight_az_estimated",
        "vacs_boresight_el_nominal",
        "vacs_boresight_el_kf_internal",
        "vacs_boresight_el_estimated",
    ]
    veh_df = pd.concat([pd.read_csv(file, usecols=cols_of_interest) for file in veh_csv_list])
    sim_df = pd.concat([pd.read_csv(file, usecols=cols_of_interest) for file in sim_csv_list])
    number_of_scans_in_real = veh_df.shape[0]
    number_of_scans_in_sim = sim_df.shape[0]
    #################################

    print("Merging the dataframes...")
    #################################
    # Merge Vehicle and Resim data
    #################################
    scan_index = "scan_index"
    final_df = pd.merge(veh_df, sim_df, on=scan_index, how="inner", suffixes=("_real", "_sim"))
    final_df = final_df.iloc[1:]
    # print(final_df.info())
    number_of_scans_in_both_real_and_sim = final_df.shape[0]
    #################################

    #################################
    # Loop through each scan index
    #################################
    if max_num_of_si_to_process != 0:
        final_df = final_df.iloc[:max_num_of_si_to_process]

    scan_index_list = final_df["scan_index"].values.tolist()
    # The conversion from radians to degrees has to be done for each signal as they will be plotted
    final_df["az_misalign_est_real"] = (final_df["vacs_boresight_az_nominal_real"] - final_df["vacs_boresight_az_estimated_real"]) * (180 / np.pi)
    final_df["az_misalign_ref_real"] = (final_df["vacs_boresight_az_nominal_real"] - final_df["vacs_boresight_az_kf_internal_real"]) * (180 / np.pi)
    final_df["az_misalign_est_sim"] = (final_df["vacs_boresight_az_nominal_sim"] - final_df["vacs_boresight_az_estimated_sim"]) * (180 / np.pi)
    final_df["az_misalign_ref_sim"] = (final_df["vacs_boresight_az_nominal_sim"] - final_df["vacs_boresight_az_kf_internal_sim"]) * (180 / np.pi)
    final_df["el_misalign_est_real"] = (final_df["vacs_boresight_el_nominal_real"] - final_df["vacs_boresight_el_estimated_real"]) * (180 / np.pi)
    final_df["el_misalign_ref_real"] = (final_df["vacs_boresight_el_nominal_real"] - final_df["vacs_boresight_el_kf_internal_real"]) * (180 / np.pi)
    final_df["el_misalign_est_sim"] = (final_df["vacs_boresight_el_nominal_sim"] - final_df["vacs_boresight_el_estimated_sim"]) * (180 / np.pi)
    final_df["el_misalign_ref_sim"] = (final_df["vacs_boresight_el_nominal_sim"] - final_df["vacs_boresight_el_kf_internal_sim"]) * (180 / np.pi)
    final_df["az_misalign_est_diff_real"] = final_df["az_misalign_est_real"] - final_df["az_misalign_est_sim"]
    final_df["el_misalign_est_diff_real"] = final_df["el_misalign_est_real"] - final_df["el_misalign_est_sim"]
    # print(final_df.info())
    az_misalign_est_real_list = final_df["az_misalign_est_real"].values.tolist()
    az_misalign_ref_real_list = final_df["az_misalign_ref_real"].values.tolist()
    az_misalign_est_sim_list = final_df["az_misalign_est_sim"].values.tolist()
    az_misalign_ref_sim_list = final_df["az_misalign_ref_sim"].values.tolist()
    el_misalign_est_real_list = final_df["el_misalign_est_real"].values.tolist()
    el_misalign_ref_real_list = final_df["el_misalign_ref_real"].values.tolist()
    el_misalign_est_sim_list = final_df["el_misalign_est_sim"].values.tolist()
    el_misalign_ref_sim_list = final_df["el_misalign_ref_sim"].values.tolist()
    az_misalign_est_diff_list = final_df["az_misalign_est_diff_real"].values.tolist()
    el_misalign_est_diff_list = final_df["el_misalign_est_diff_real"].values.tolist()
    number_of_scans_with_matching_az_misalign = len(final_df[abs(final_df["az_misalign_est_diff_real"]) < az_misalign_threshold])
    number_of_scans_with_matching_el_misalign = len(final_df[abs(final_df["el_misalign_est_diff_real"]) < el_misalign_threshold])
    #################################

    kpis_align = {'result1':
                   {'numerator': number_of_scans_with_matching_az_misalign,
                    'denominator': number_of_scans_in_both_real_and_sim,
                    'value': round((number_of_scans_with_matching_az_misalign / number_of_scans_in_both_real_and_sim) * 100, 2)},
               'result2':
                   {'numerator': number_of_scans_with_matching_el_misalign,
                    'denominator': number_of_scans_in_both_real_and_sim,
                    'value': round((number_of_scans_with_matching_el_misalign / number_of_scans_in_both_real_and_sim) * 100, 2)},
               }
    #################################
    # HTML Content
    #################################
    html_content += f"""
        <b>KPI:</b> Az Accuracy: ({kpis_align['result1']['numerator']}/{kpis_align['result1']['denominator']}) --> <b>{kpis_align['result1']['value']}%</b>
        El Accuracy: ({kpis_align['result2']['numerator']}/{kpis_align['result2']['denominator']}) --> <b>{kpis_align['result2']['value']}%</b>
        <details>
            <br>
        """
    #################################

def plot_stats():
    #############################################################################################################
    # Global variables
    #############################################################################################################
    global html_content
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, az_misalign_est_diff_list
    #############################################################################################################

    #############################################################################################################
    # Create subplots for Misalignment v/s scanindex:
    #############################################################################################################
    fig_line = sp.make_subplots(rows=2, cols=1, horizontal_spacing=0.04, vertical_spacing=0.2)

    # Manually add traces for line plots
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=az_misalign_est_real_list,
            mode="lines",
            name="Veh Az Est",
            line=dict(color="blue"),
        ),
        row=1,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=az_misalign_ref_real_list,
            mode="lines",
            name="Veh Az Ref",
            line=dict(color="blue"),
            opacity=0.2,
        ),
        row=1,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=az_misalign_est_sim_list,
            mode="lines",
            name="Sim Az Estd",
            line=dict(color="red"),
        ),
        row=1,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=az_misalign_ref_sim_list,
            mode="lines",
            name="Sim Az Ref",
            line=dict(color="red"),
            opacity=0.2,
        ),
        row=1,
        col=1,
    )
    fig_line.update_yaxes(title_text="Azimuth misalignment", row=1, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=1, col=1)

    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=el_misalign_est_real_list,
            mode="lines",
            name="Veh El Est",
            line=dict(color="blue"),
        ),
        row=2,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=el_misalign_ref_real_list,
            mode="lines",
            name="Veh El Ref",
            line=dict(color="blue"),
            opacity=0.2,
        ),
        row=2,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=el_misalign_est_sim_list,
            mode="lines",
            name="Sim El Est",
            line=dict(color="red"),
        ),
        row=2,
        col=1,
    )
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list,
            y=el_misalign_ref_sim_list,
            mode="lines",
            name="Sim El Ref",
            line=dict(color="red"),
            opacity=0.2,
        ),
        row=2,
        col=1,
    )
    fig_line.update_yaxes(title_text="Elevation misalignment", row=2, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=2, col=1)

    # Update layout for bar plot
    fig_line.update_layout(
        height=1000, width=1250, title_text="Misalignment v/s scanindex", showlegend=True
    )
    fig_line.update_traces(marker_color="red")
    fig_line.update_xaxes(zeroline=False, showgrid=False, type="category")
    fig_line.update_yaxes(zeroline=False, showgrid=False)
    fig_line.show()

    misalign_line_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs="cdn")
    #############################################################################################################

    #############################################################################################################
    # Create subplots for Misalignment difference v/s scanindex:
    #############################################################################################################
    fig_line = sp.make_subplots(rows=2, cols=1, horizontal_spacing=0.04, vertical_spacing=0.2)

    # Manually add traces for line plots
    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list, y=az_misalign_est_diff_list, mode="lines", name="Az diff", line=dict(color="blue")
        ),
        row=1,
        col=1,
    )
    fig_line.update_yaxes(title_text="Azimuth misalignment diff", row=1, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=1, col=1)

    fig_line.add_trace(
        go.Scatter(
            x=scan_index_list, y=el_misalign_est_diff_list, mode="lines", name="El diff", line=dict(color="blue")
        ),
        row=2,
        col=1,
    )
    fig_line.update_yaxes(title_text="Elevation misalignment diff", row=2, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=2, col=1)

    # Update layout for bar plot
    fig_line.update_layout(
        height=1000,
        width=1250,
        title_text="Misalignment difference v/s scanindex",
        showlegend=True,
    )
    fig_line.update_traces(marker_color="red")
    fig_line.update_xaxes(zeroline=False, showgrid=False, type="category")
    fig_line.update_yaxes(zeroline=False, showgrid=False)
    fig_line.show()

    misalign_diff_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs="cdn")
    #############################################################################################################

    #############################################################################################################
    # HTML Content
    #############################################################################################################
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
    #############################################################################################################

def process_logs(data_files):
    #################################
    # Global variables
    #################################
    global html_content
    global scan_index_list
    global az_misalign_est_real_list, az_misalign_ref_real_list, az_misalign_est_sim_list, az_misalign_ref_sim_list
    global el_misalign_est_real_list, el_misalign_ref_real_list, el_misalign_est_sim_list, el_misalign_ref_sim_list
    global az_misalign_est_diff_list, el_misalign_est_diff_list
    #################################

    veh_csv_list = [data_files['input'][i] for i in range(len(data_files['input']))]
    sim_csv_list = [data_files['output'][i] for i in range(len(data_files['output']))]
    for sensor_pos in ['FL', 'FR', 'RL', 'RR']:
        veh_csv_sp_list = [veh_csv for veh_csv in veh_csv_list if sensor_pos in veh_csv]
        sim_csv_sp_list = [sim_csv for sim_csv in sim_csv_list if sensor_pos in sim_csv]

        if veh_csv_sp_list and sim_csv_sp_list:
            file_name = os.path.basename(veh_csv_sp_list[0])
            log_name_wo_ext = file_name.replace("_UDP_GEN7_DYNAMIC_ALIGNMENT_STREAM.csv", "")
            html_content += f"""
                    <b>Log:</b> {log_name_wo_ext}({len(veh_csv_sp_list)} logs)
                    """

            #################################
            # Reset the below variables
            #################################
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

            main(veh_csv_sp_list, sim_csv_sp_list)
            plot_stats()

def find_data_files(base_path):
    data_files = {
        'input': [],
        'output': []
    }
    files = os.listdir(base_path)
    files_of_interest = [s for s in files if '_UDP_GEN7_DYNAMIC_ALIGNMENT_STREAM.csv' in s]
    unique_files = set(s.replace('_UDP_GEN7_DYNAMIC_ALIGNMENT_STREAM.csv', "").strip() for s in files_of_interest)
    for filename in sorted(unique_files):
        file_path = os.path.join(base_path, filename+'_UDP_GEN7_DYNAMIC_ALIGNMENT_STREAM.csv')
        if "_r0" in filename:
            data_files['output'].append(file_path)
        else:
            data_files['input'].append(file_path)
    return data_files

#####################################################################
# START
#####################################################################
if len(sys.argv) != 4:
    print("Usage: python alignment_matching_kpi_script.py log_path.txt meta_data.txt C:\\Gitlab\\gen7v1_resim_kpi_scripts")
    sys.exit(1)

log_path_file = sys.argv[1]
meta_data_file = sys.argv[2]
output_folder = sys.argv[3]

if not os.path.exists(log_path_file):
    print(f"Error: File {log_path_file} does not exist.")
    sys.exit(1)
if not os.path.exists(meta_data_file):
    print(f"Error: File {meta_data_file} does not exist.")
    sys.exit(1)
#################################
# User modifiable variables
#################################
max_num_of_si_to_process = 0
#################################

#################################
# Variables that get updated
#################################
epsilon = 0.0000001
az_misalign_threshold = 0.01 + epsilon
el_misalign_threshold = 0.01 + epsilon
number_of_scans_in_real = 0
number_of_scans_in_sim = 0
number_of_scans_in_both_real_and_sim = 0
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

#################################
# Read meta data
#################################
# Initialize an empty dictionary
metadata_dict = {}

# Open and read the file
with open(meta_data_file, "r") as file:
    for line in file:
        # Split each line by the first whitespace to get key and value
        key, value = line.strip().split(maxsplit=1)
        # Add to dictionary
        metadata_dict[key] = value
#print(metadata_dict)
#################################

#################################
# HTML Content
#################################
html_content = f"""
<html>
<head>
    <title>Alignment KPIs and Plots</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        table {{ width: 75%; border-collapse: collapse; margin: 20px 0; }}
        th, td {{ border: 1px solid #dddddd; text-align: left; padding: 8px; }}
        th {{ background-color: #f2f2f2; }}
    </style>
</head>
<body>
    <h1>Alignment KPIs and Plots</h1>
    <ul>
        <li><b>SiL_Engine:</b> {metadata_dict['SiL_Engine']}</li>
        <li><b>SW:</b> {metadata_dict['SW']}</li>
        <li><b>RSP_SiL:</b> {metadata_dict['RSP_SiL']}</li>
        <li><b>Tracker:</b> {metadata_dict['Tracker']}</li>
        <li><b>Mode:</b> {metadata_dict['Mode']}</li>
    </ul>
    <details>
        <summary><b><i>Glossary</i></b></summary>
        <ol>
            <li><b>Definition of match:</b>
                A scan is said to match a re-simulated scan if the difference(error) in the mis-alignment is within the threshold mentioned below
                <ul>
                    <li>Az mis-alignment : {round(az_misalign_threshold, 5)} deg</li>
                    <li>El mis-alignment : {round(el_misalign_threshold, 5)} deg</li>
                </ul> 
           <li><b>Accuracy:</b> (Number of matching scans / total number of scans) * 100
			<li><b>Plot A:</b> Plot of mis-alignment across scan indices
			<li><b>Plot B:</b> Plot of mis-alignment difference across scan indices
        </ol>
        <b>Note:</b> The plots are interactive
        <br>
        <b>Note:</b> Blank plots indicate no data or zero error
    </details>

    <hr width="100%" size="2" color="blue" noshade>
"""
#################################

with open(log_path_file, 'r') as f:
    directory = f.readline().strip()
    data_files = find_data_files(directory)
    #print(data_files)
    if data_files['input'] and data_files['output']:
        process_logs(data_files)

html_content += "</table></body></html>"

# Write HTML content to file
output_html = output_folder + "/alignment_kpi_report.html"
with open(output_html, "w") as f:
    f.write(html_content)
#####################################################################
# END
#####################################################################