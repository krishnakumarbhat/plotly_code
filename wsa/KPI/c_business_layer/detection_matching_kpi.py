# #################################
# # Packages
# #################################
# import sys
# import os
# import glob
# import re
# import numpy as np
# import pandas as pd
# import plotly.graph_objs as go
# import plotly.subplots as sp
# import plotly.io as pio
# from collections import Counter
# import uuid
# import time
# pd.options.display.width = 0
# #################################
# # User modifiable variables
# #################################
# FILE_VERSION = "v1.1"
# MAX_NUM_OF_SI_TO_PROCESS = 0
# # Constants
# FILE_TITLE = "Detection KPIs and Plots"
# UNIQUE_KEY = str(uuid.uuid4())
# DET_FILE_SUFFIX = '_UDP_GEN7_DET_CORE.csv'
# RDD_FILE_SUFFIX = '_UDP_GEN7_RDD_CORE.csv'
# CDC_FILE_SUFFIX = '_UDP_CDC.csv'
# VSE_FILE_SUFFIX = '_UDP_GEN7_VSE_CORE.csv'
# PRINT_LOG_FILE_NAME = 'det_kpi_print_log.txt'
# DEFAULT_ACC_THRESHOLD = 99
# MAX_LOGS_IN_ONE_REPORT = 20
# MAX_NUM_OF_AF_DETS_FRONT_RADAR = 768
# MAX_NUM_OF_AF_DETS_CORNER_RADAR = 680
# MAX_NUM_OF_RDD_DETS = 512
# RADAR_CYCLE_S = 0.05
# M_2_KM = 0.001
# EPSILON = 0.0000001
# RAN_THRESHOLD = 0.01 + EPSILON
# VEL_THRESHOLD = 0.015 + EPSILON
# THETA_THRESHOLD = 0.00873 + EPSILON
# PHI_THRESHOLD= 0.00873 + EPSILON
# # Modified via meta data file, below is init value
# RESIM_MODE = "CDC"
# MAX_CDC_RECORDS = 5016
# RANGE_SATURATION_THRESHOLD_FRONT_RADAR = 135
# RANGE_SATURATION_THRESHOLD_CORNER_RADAR = 135
# # Modified based on sensor position, below is init value
# MAX_NUM_OF_AF_DETS = 0
# RANGE_SATURATION_THRESHOLD = 0
# #################################
# def check_file(csv_file) -> bool:
#     result = False
#     try:
#         if os.path.exists(csv_file):
#             try:
#                 if os.path.getsize(csv_file) > 0:
#                     result = True
#                 else:
#                     custom_print(f"[WARNING] {csv_file} file is empty")
#             except OSError as e:
#                 custom_print(f"[WARNING] {csv_file} file size check failed: {e}")
#         else:
#             custom_print(f"[WARNING] {csv_file} file is missing")
#     except Exception as e:
#         custom_print(f"[WARNING] {csv_file} file unexpected error: {e}")
#     return result
# def main(veh_csv, sim_csv, veh_rdd_csv, sim_rdd_csv, veh_cdc_csv, sim_cdc_csv, veh_vse_csv, sim_vse_csv) -> bool:
#     #################################
#     # Global variables
#     #################################
#     global num_of_SI_in_veh_af, num_of_SI_in_sim_af, num_of_same_SI_in_veh_and_sim_af
#     global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list, snr_diff_list, rcs_diff_list
#     global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list, saturated_list
#     global min_accuracy, overall_accuracy
#     global cdc_data_available, perc_of_scans_with_cdc_saturation, perc_of_scans_with_range_saturation
#     global num_of_SI_available_in_veh, num_of_SI_available_for_kpi
#     global dist_travelled_by_veh, dist_travelled_by_sim
#     global html_content
#     #################################
#     custom_print("[INFO] Reading the CSVs...")
#     #################################
#     # Read Vehicle and Resim data
#     # Not all scans available in vehicle DET csv will be available in sim DET csv
#     # Not all scans available in vehicle DET csv will be available in vehicle RDD csv
#     # Not all scans available in sim DET csv will be available in sim RDD csv
#     #################################
#     if (MAX_NUM_OF_SI_TO_PROCESS != 0):
#         nrows = MAX_NUM_OF_SI_TO_PROCESS
#     else:
#         nrows = None
#     #################################
#     # Read DET csv
#     #################################
#     custom_print("[INFO] Reading DET CSV...")
#     det_cols_of_interest = ['scan_index', 'num_af_det']
#     det_cols_of_interest = det_cols_of_interest + [item for i in range(MAX_NUM_OF_AF_DETS) for item in (f"rdd_idx_{i}", f"ran_{i}", f"vel_{i}", f"theta_{i}", f"phi_{i}", f"f_single_target_{i}", f"f_superres_target_{i}", f"f_bistatic_{i}")]
#     veh_det_df = pd.read_csv(veh_csv, usecols = det_cols_of_interest, nrows=nrows, memory_map=True)
#     sim_det_df = pd.read_csv(sim_csv, usecols = det_cols_of_interest, nrows=nrows, memory_map=True)
#     # Keep only non-zero scan indices
#     veh_det_df = veh_det_df[veh_det_df['scan_index'] != 0]
#     sim_det_df = sim_det_df[sim_det_df['scan_index'] != 0]
#     # Keep only those scans which have non-zero AF dets
#     veh_det_df = veh_det_df[veh_det_df['num_af_det'] != 0]
#     sim_det_df = sim_det_df[sim_det_df['num_af_det'] != 0]
#     # The below should be assigned after all filtering
#     num_of_SI_available_in_veh = veh_det_df.shape[0]
#     #################################
#     #################################
#     # Read CDC csv
#     #################################
#     veh_cdc_df_non_saturated = pd.DataFrame()
#     if "CDC" in RESIM_MODE:
#         custom_print("[INFO] Reading CDC CSV...")
#         def read_all_related_cdc_csvs(base_filename):
#             """
#             Read all related CSV files into a single pandas DataFrame.
#             Parameters:
#             base_filename (str): The name of the first file (e.g., 'base_CDC.csv')
#             Returns:
#             pandas.DataFrame: Combined DataFrame from all related CSV files
#             """
#             # Extract the base pattern from the filename
#             file_prefix = base_filename.split('.')[0]  # Remove extension
#             # Create a pattern to match all related files
#             pattern = f"{file_prefix}*.csv"
#             # Get the directory of the base file
#             directory = os.path.dirname(base_filename) or '.'
#             # Find all matching files
#             all_files = glob.glob(os.path.join(directory, pattern))
#             #print(f"Found {len(all_files)} files matching pattern: {pattern}")
#             # Read and combine all files
#             df_list = []
#             for file in sorted(all_files):
#                 #print(f"Reading {file}...")
#                 cdc_cols_of_interest = ['Scan_Index']
#                 df = pd.read_csv(file, usecols = cdc_cols_of_interest)
#                 df_list.append(df)
#             # Combine all dataframes
#             if df_list:
#                 combined_df = pd.concat(df_list, ignore_index=True)
#                 #print(f"Combined DataFrame shape: {combined_df.shape}")
#                 return combined_df
#             else:
#                 #print("No files found!")
#                 return pd.DataFrame()
#         if check_file(veh_cdc_csv):
#             veh_cdc_df = read_all_related_cdc_csvs(veh_cdc_csv)
#             # Keep only non-zero scan indices
#             veh_cdc_df = veh_cdc_df.rename(columns={'Scan_Index': 'scan_index'})
#             veh_cdc_df = veh_cdc_df[veh_cdc_df['scan_index'] != 0]
#             veh_cdc_df = veh_cdc_df.groupby(['scan_index']).size().reset_index(name='num_cdc_records')
#             num_of_scans_with_cdc_saturation = veh_cdc_df[veh_cdc_df['num_cdc_records'] == MAX_CDC_RECORDS].shape[0]
#             if veh_cdc_df.shape[0] != 0:
#                 perc_of_scans_with_cdc_saturation = round((num_of_scans_with_cdc_saturation/veh_cdc_df.shape[0])*100, 2)
#             else:
#                 perc_of_scans_with_cdc_saturation = 0
#             veh_cdc_df_non_saturated = veh_cdc_df[veh_cdc_df['num_cdc_records'] < MAX_CDC_RECORDS]
#             cdc_data_available = True
#             custom_print("[INFO] CDC data available...")
#         else:
#             cdc_data_available = False
#             # Letting perc_of_scans_with_cdc_saturation be 0 could falsely mean that
#             # there is no CDC saturation, but in reality there is no CDC data itself
#             # So to identify it properly, I am setting perc_of_scans_with_cdc_saturation to -10
#             perc_of_scans_with_cdc_saturation = -10
#             custom_print("[WARNING] CDC data not available...")
#     #################################
#     #################################
#     # Keep only those DET scans which are not saturated
#     #################################
#     if cdc_data_available:
#         veh_det_df = veh_det_df[veh_det_df['scan_index'].isin(veh_cdc_df_non_saturated['scan_index'])]
#         veh_det_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     #################################
#     #################################
#     # Read RDD csv
#     #################################
#     custom_print("[INFO] Reading RDD CSV...")
#     rdd_cols_of_interest = ['scan_index', 'rdd1_num_detect']
#     rdd_cols_of_interest = rdd_cols_of_interest + [item for i in range(MAX_NUM_OF_RDD_DETS) for item in (f"rdd1_rindx_{i}", f"rdd1_dindx_{i}", f"rdd2_range_{i}", f"rdd2_range_rate_{i}")]
#     veh_rdd_df = pd.read_csv(veh_rdd_csv, usecols = rdd_cols_of_interest, nrows=nrows, memory_map=True)
#     sim_rdd_df = pd.read_csv(sim_rdd_csv, usecols = rdd_cols_of_interest, nrows=nrows, memory_map=True)
#     # Keep only non-zero scan indices
#     veh_rdd_df = veh_rdd_df[veh_rdd_df['scan_index'] != 0]
#     sim_rdd_df = sim_rdd_df[sim_rdd_df['scan_index'] != 0]
#     # Keep only those rows which have RDD1 dets
#     veh_rdd_df = veh_rdd_df[veh_rdd_df['rdd1_num_detect'] != 0]
#     sim_rdd_df = sim_rdd_df[sim_rdd_df['rdd1_num_detect'] != 0]
#     # Keep only those RDD scans which are not saturated
#     if cdc_data_available:
#         veh_rdd_df = veh_rdd_df[veh_rdd_df['scan_index'].isin(veh_cdc_df_non_saturated['scan_index'])]
#         veh_rdd_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     # Check if there is data after above filtering
#     num_of_SI_in_veh_rdd = veh_rdd_df.shape[0]
#     num_of_SI_in_sim_rdd = sim_rdd_df.shape[0]
#     if(0 == num_of_SI_in_veh_rdd):
#         custom_print("[WARNING] Filtered " + veh_rdd_csv + " is empty")
#         return False
#     if(0 == num_of_SI_in_sim_rdd):
#         custom_print("[WARNING] Filtered " + sim_rdd_csv + " is empty")
#         return False
#     #################################
#     #################################
#     # Read VSE csv
#     #################################
#     vse_data_available = False
#     custom_print("[INFO] Reading VSE CSV...")
#     veh_vse_df = pd.DataFrame()
#     sim_vse_df = pd.DataFrame()
#     if check_file(veh_vse_csv) and check_file(sim_vse_csv):
#         vse_cols_of_interest = ['scan_index', 'veh_speed']
#         veh_vse_df = pd.read_csv(veh_vse_csv, usecols = vse_cols_of_interest, nrows=nrows, memory_map=True)
#         sim_vse_df = pd.read_csv(sim_vse_csv, usecols = vse_cols_of_interest, nrows=nrows, memory_map=True)
#         # Keep only non-zero scan indices
#         veh_vse_df = veh_vse_df[veh_vse_df['scan_index'] != 0]
#         sim_vse_df = sim_vse_df[sim_vse_df['scan_index'] != 0]
#         vse_data_available = True
#         custom_print("[INFO] VSE data available...")
#     else:
#         vse_data_available = False
#         custom_print("[WARNING] VSE data not available...")
#     #################################
#     #################################
#     # Keep only those scans which are available in both DET and RDD csvs
#     # #################################
#     veh_det_df = veh_det_df[veh_det_df['scan_index'].isin(veh_rdd_df['scan_index'])]
#     veh_det_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     sim_det_df = sim_det_df[sim_det_df['scan_index'].isin(sim_rdd_df['scan_index'])]
#     sim_det_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     # Check if there is data after above filtering
#     num_of_SI_in_veh_af = veh_det_df.shape[0]
#     num_of_SI_in_sim_af = sim_det_df.shape[0]
#     if(0 == num_of_SI_in_veh_af):
#         custom_print("[WARNING] Filtered " + veh_csv + " is empty")
#         return False
#     if(0 == num_of_SI_in_sim_af):
#         custom_print("[WARNING] Filtered " + sim_csv + " is empty")
#         return False
#     ##################################
#     #################################
#     # RDD Stream matching
#     #################################
#     custom_print("[INFO] RDD Detection Matching...")
#     # Set constants
#     SCALE_P21_TO_FLOAT = (4.768371582e-07)
#     # Step 1: Merge the dataframes on 'scan_index'
#     merged_df = pd.merge(veh_rdd_df, sim_rdd_df, on='scan_index', suffixes=('_veh', '_sim'))
#     merged_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     num_of_same_SI_in_veh_and_sim_rdd = merged_df.shape[0]
#     num_of_SI_with_same_num_of_rdd1_dets = merged_df[merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']].shape[0]
#     # Generate column names
#     rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(MAX_NUM_OF_RDD_DETS)]
#     dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(MAX_NUM_OF_RDD_DETS)]
#     range_cols_veh = [f"rdd2_range_{i}_veh" for i in range(MAX_NUM_OF_RDD_DETS)]
#     range_rate_cols_veh = [f"rdd2_range_rate_{i}_veh" for i in range(MAX_NUM_OF_RDD_DETS)]
#     rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(MAX_NUM_OF_RDD_DETS)]
#     dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(MAX_NUM_OF_RDD_DETS)]
#     range_cols_sim = [f"rdd2_range_{i}_sim" for i in range(MAX_NUM_OF_RDD_DETS)]
#     range_rate_cols_sim = [f"rdd2_range_rate_{i}_sim" for i in range(MAX_NUM_OF_RDD_DETS)]
#     # Step 2: Compute matched (rdd1_rindx, rdd1_dindx) pairs
#     def count_rindx_dindx_matches(row):
#         """Count matching (rindx, dindx) pairs between veh and sim."""
#         num_detect_veh = int(row['rdd1_num_detect_veh'])
#         num_detect_sim = int(row['rdd1_num_detect_sim'])
#         veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
#         sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
#         match_count = 0
#         for veh_pair in veh_pairs:
#             if veh_pair in sim_pairs:
#                 match_count += 1
#                 sim_pairs.remove(veh_pair)  # Avoid reuse
#         return match_count
#     merged_df['matched_rindx_dindx_pairs'] = merged_df.apply(count_rindx_dindx_matches, axis=1)
#     # Step 3: Compute matches within thresholds for rdd2_range and rdd2_range_rate
#     def count_range_matches(row):
#         """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
#         num_detect_veh = int(row['rdd1_num_detect_veh'])
#         num_detect_sim = int(row['rdd1_num_detect_sim'])
#         veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
#         sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
#         sim_data = dict(zip(sim_pairs, zip(row[range_cols_sim[:num_detect_sim]], row[range_rate_cols_sim[:num_detect_sim]])))
#         match_count = 0
#         for idx, veh_pair in enumerate(veh_pairs):
#             if veh_pair in sim_data:
#                 sim_range, sim_range_rate = sim_data[veh_pair]
#                 sim_range = round(sim_range * SCALE_P21_TO_FLOAT, 3)
#                 sim_range_rate = round(sim_range_rate * SCALE_P21_TO_FLOAT, 3)
#                 veh_range = row[range_cols_veh[idx]]
#                 veh_range_rate = row[range_rate_cols_veh[idx]]
#                 veh_range = round(veh_range * SCALE_P21_TO_FLOAT, 3)
#                 veh_range_rate = round(veh_range_rate * SCALE_P21_TO_FLOAT, 3)
#                 if (abs(sim_range - veh_range) <= RAN_THRESHOLD
#                     and abs(sim_range_rate - veh_range_rate) <= VEL_THRESHOLD):
#                     match_count += 1
#         return match_count
#     merged_df['range_rangerate_matches'] = merged_df.apply(count_range_matches, axis=1)
#     merged_df['same_num_of_RDD1_detections'] = merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']
#     merged_df['matching_pct_rindx_dindx_pairs'] = merged_df['matched_rindx_dindx_pairs']/merged_df['rdd1_num_detect_veh']
#     merged_df['matching_pct_range_rangerate_pairs'] = merged_df['range_rangerate_matches'] / merged_df['matched_rindx_dindx_pairs']
#     num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair = merged_df[(merged_df['same_num_of_RDD1_detections'] == 1) & (merged_df['matching_pct_rindx_dindx_pairs'] == 1)].shape[0]
#     num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair = merged_df[(merged_df['same_num_of_RDD1_detections'] == 1) & (merged_df['matching_pct_rindx_dindx_pairs'] == 1) & (merged_df['matching_pct_range_rangerate_pairs'] == 1)].shape[0]
#     kpis_rdd = {'result1':
#                 {'numerator': num_of_SI_with_same_num_of_rdd1_dets,
#                  'denominator': num_of_same_SI_in_veh_and_sim_rdd,
#                  'value': round((num_of_SI_with_same_num_of_rdd1_dets/num_of_same_SI_in_veh_and_sim_rdd)*100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None},
#             'result2':
#                 {'numerator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
#                  'denominator': num_of_SI_with_same_num_of_rdd1_dets,
#                  'value': round((num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair / num_of_SI_with_same_num_of_rdd1_dets) * 100, 2) if (num_of_SI_with_same_num_of_rdd1_dets != 0) else None},
#             'result3':
#                 {'numerator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
#                  'denominator': num_of_same_SI_in_veh_and_sim_rdd,
#                  'value': round((num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair / num_of_same_SI_in_veh_and_sim_rdd) * 100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None},
#             'result4':
#                 {'numerator': num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair,
#                  'denominator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
#                  'value': round((num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair / num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair) * 100, 2) if (num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair != 0) else None},
#             'result5':
#                 {'numerator': num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair,
#                  'denominator': num_of_same_SI_in_veh_and_sim_rdd,
#                  'value': round((num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair / num_of_same_SI_in_veh_and_sim_rdd) * 100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None },
#             }
#     custom_print(f"Number of SI in (vehicle, simulation): ({num_of_SI_in_veh_rdd}, {num_of_SI_in_sim_rdd})")
#     custom_print(f"Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_rdd}")
#     custom_print(f"% of SI with same number of RDD1 detections: "
#           f"{kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']} --> {kpis_rdd['result1']['value']}%" )
#     custom_print(f"% of SI with 100% matching (rindx, dindx) pair: "
#           f"{kpis_rdd['result2']['numerator']}/{kpis_rdd['result2']['denominator']} --> {kpis_rdd['result2']['value']}%, "
#           f"{kpis_rdd['result3']['numerator']}/{kpis_rdd['result3']['denominator']} --> {kpis_rdd['result3']['value']}%")
#     custom_print(f"% of SI with 100% matching (range, rangerate) pair: "
#           f"{kpis_rdd['result4']['numerator']}/{kpis_rdd['result4']['denominator']} --> {kpis_rdd['result4']['value']}%, "
#           f"{kpis_rdd['result5']['numerator']}/{kpis_rdd['result5']['denominator']} --> {kpis_rdd['result5']['value']}%")
#     #################################
#     # DET Stream matching
#     #################################
#     custom_print("[INFO] AF Detection Matching...")
#     #################################
#     # Extract and append rdd1_rindx and rdd1_dindx values from veh_rdd_df to veh_det_df
#     # based on rdd_idx columns of veh_det_df
#     #################################
#     # Generate a dictionary for the new columns 'rdd1_rindx' and 'rdd1_dindx' with `None` values
#     new_data = {f"rdd1_rindx_{i}": None for i in range(MAX_NUM_OF_AF_DETS)}
#     new_data.update({f"rdd1_dindx_{i}": None for i in range(MAX_NUM_OF_AF_DETS)})
#     # Create a new DataFrame with these columns
#     new_columns_df = pd.DataFrame(new_data, index=veh_det_df.index)
#     # Concatenate the new columns to the existing DataFrame
#     veh_det_df = pd.concat([veh_det_df, new_columns_df], axis=1)
#     # Iterate through each row of veh_det_df
#     for idx, row in veh_det_df.iterrows():
#         scan_index = row['scan_index']
#         # Extract corresponding row in veh_rdd_df
#         rdd_row = veh_rdd_df[veh_rdd_df['scan_index'] == scan_index]
#         # If a matching row is found in veh_rdd_df
#         if not rdd_row.empty:
#             # Iterate over each rdd_idx column in veh_det_df
#             for i in range(len([col for col in veh_det_df.columns if col.startswith('rdd_idx')])):
#                 rdd_idx = row[f'rdd_idx_{i}']
#                 # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
#                 veh_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
#                 veh_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]
#     # Display the updated veh_det_df
#     #custom_print(veh_det_df)
#     #################################
#     #################################
#     # Extract and append rdd1_rindx and rdd1_dindx values from sim_rdd_df to sim_det_df
#     # based on rdd_idx columns of sim_det_df
#     #################################
#     new_columns_df = pd.DataFrame(new_data, index=sim_det_df.index)
#     # Concatenate the new columns to the existing DataFrame
#     sim_det_df = pd.concat([sim_det_df, new_columns_df], axis=1)
#     # Iterate through each row of sim_det_df
#     for idx, row in sim_det_df.iterrows():
#         scan_index = row['scan_index']
#         # Extract corresponding row in veh_rdd_df
#         rdd_row = sim_rdd_df[sim_rdd_df['scan_index'] == scan_index]
#         # If a matching row is found in veh_rdd_df
#         if not rdd_row.empty:
#             # Iterate over each rdd_idx column in sim_det_df
#             for i in range(len([col for col in sim_det_df.columns if col.startswith('rdd_idx')])):
#                 rdd_idx = row[f'rdd_idx_{i}']
#                 # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
#                 sim_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
#                 sim_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]
#     # Display the updated sim_det_df
#     #custom_print(sim_det_df)
#     #################################
#     custom_print("[INFO] Merging the dataframes...")
#     #################################
#     # Merge Vehicle and Resim data
#     #################################
#     result_df = pd.merge(veh_det_df, sim_det_df, on='scan_index', how='inner', suffixes=('_veh', '_sim'))
#     result_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#     if (MAX_NUM_OF_SI_TO_PROCESS != 0):
#         result_df = result_df.iloc[:MAX_NUM_OF_SI_TO_PROCESS]
#     num_of_same_SI_in_veh_and_sim_af = result_df.shape[0]
#     num_of_SI_with_same_num_of_dets_af = result_df[result_df['num_af_det_veh'] == result_df['num_af_det_sim']].shape[0]
#     base_columns = ['scan_index', 'num_af_det_veh', 'num_af_det_sim']
#     repeated_columns = ['rdd_idx', 'rdd1_rindx', 'rdd1_dindx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 'f_superres_target', 'f_bistatic']
#     selected_columns_real = [f'{col}_{i}_veh' for col in repeated_columns for i in range(MAX_NUM_OF_AF_DETS)]
#     selected_columns_sim = [f'{col}_{i}_sim' for col in repeated_columns for i in range(MAX_NUM_OF_AF_DETS)]
#     selected_columns = base_columns + selected_columns_real + selected_columns_sim
#     final_df = result_df[selected_columns]
#     # The below should be assigned after all filtering
#     num_of_SI_available_for_kpi = final_df.shape[0]
#     #################################
#     #################################
#     # VSE data processing
#     #################################
#     if vse_data_available:
#         custom_print("[INFO] VSE Data Processing...")
#         dist_travelled_by_veh = (veh_vse_df['veh_speed'] * RADAR_CYCLE_S).sum()
#         # Keep only those scans of sim VSE stream which are available for KPI
#         sim_vse_df = sim_vse_df[sim_vse_df['scan_index'].isin(final_df['scan_index'])]
#         sim_vse_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
#         dist_travelled_by_sim = (sim_vse_df['veh_speed'] * RADAR_CYCLE_S).sum()
#     #################################
#     rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     ran_cols_veh = [f"ran_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     vel_cols_veh = [f"vel_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     theta_cols_veh = [f"theta_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     phi_cols_veh = [f"phi_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     single_cols_veh = [f"f_single_target_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     superres_cols_veh = [f"f_superres_target_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     bistatic_cols_veh = [f"f_bistatic_{i}_veh" for i in range(MAX_NUM_OF_AF_DETS)]
#     rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     ran_cols_sim = [f"ran_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     vel_cols_sim = [f"vel_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     theta_cols_sim = [f"theta_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     phi_cols_sim = [f"phi_{i}_sim" for i in range(MAX_NUM_OF_AF_DETS)]
#     scan_index_list = final_df['scan_index'].tolist()
#     num_af_det_veh_list = final_df['num_af_det_veh'].tolist()
#     num_af_det_sim_list = final_df['num_af_det_sim'].tolist()
#     def find_max_range(row):
#         max_range_veh = 0
#         max_range_sim = 0
#         saturated = False
#         max_range_veh = max(row[ran_cols_veh])
#         max_range_sim = max(row[ran_cols_sim])
#         if max_range_veh >= RANGE_SATURATION_THRESHOLD and max_range_sim < RANGE_SATURATION_THRESHOLD:
#             saturated = True
#         return max_range_veh, max_range_sim, saturated
#     final_df[['max_range_veh', 'max_range_sim', 'saturated']] = final_df.apply(find_max_range, axis=1, result_type="expand")
#     max_range_veh_list = final_df['max_range_veh'].tolist()
#     max_range_sim_list = final_df['max_range_sim'].tolist()
#     saturated_list = final_df['saturated'].tolist()
#     if len(saturated_list) != 0:
#         perc_of_scans_with_range_saturation = round((sum(saturated_list) / len(saturated_list)) * 100, 2)
#     else:
#         perc_of_scans_with_range_saturation = 0
#     def count_det_params_matches(row):
#         """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
#         num_detect_veh = int(row['num_af_det_veh'])
#         num_detect_sim = int(row['num_af_det_sim'])
#         veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
#         sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
#         sim_data = dict(zip(sim_pairs, zip(row[ran_cols_sim[:num_detect_sim]],
#                                            row[vel_cols_sim[:num_detect_sim]],
#                                            row[theta_cols_sim[:num_detect_sim]],
#                                            row[phi_cols_sim[:num_detect_sim]])))
#         subset_match_count = 0
#         all_match_count = 0
#         matched_sim_indices = set()
#         for idx, veh_pair in enumerate(veh_pairs):
#             for jdx, sim_pair in enumerate(sim_pairs):
#                 if jdx in matched_sim_indices:
#                     continue
#                 if veh_pair == sim_pair:
#                     sim_ran = row[ran_cols_sim[jdx]]
#                     sim_vel = row[vel_cols_sim[jdx]]
#                     sim_theta = row[theta_cols_sim[jdx]]
#                     sim_phi = row[phi_cols_sim[jdx]]
#                     veh_ran = row[ran_cols_veh[idx]]
#                     veh_vel = row[vel_cols_veh[idx]]
#                     veh_theta = row[theta_cols_veh[idx]]
#                     veh_phi = row[phi_cols_veh[idx]]
#                     veh_single_target = row[single_cols_veh[idx]]
#                     veh_superres_target = row[superres_cols_veh[idx]]
#                     veh_bistatic = row[bistatic_cols_veh[idx]]
#                     ran_diff = veh_ran - sim_ran
#                     ran_diff_abs = abs(ran_diff)
#                     vel_diff = veh_vel - sim_vel
#                     vel_diff_abs = abs(vel_diff)
#                     theta_diff = veh_theta - sim_theta
#                     theta_diff_abs = abs(theta_diff)
#                     phi_diff = veh_phi - sim_phi
#                     phi_diff_abs = abs(phi_diff)
#                     if(ran_diff_abs <= RAN_THRESHOLD and vel_diff_abs <= VEL_THRESHOLD):
#                         subset_match_count += 1
#                         if(theta_diff_abs <= THETA_THRESHOLD and phi_diff_abs <= PHI_THRESHOLD):
#                             all_match_count += 1
#                             matched_sim_indices.add(jdx)
#                             break
#                     if (ran_diff_abs > RAN_THRESHOLD):
#                         ran_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, ran_diff))
#                     if (vel_diff_abs > VEL_THRESHOLD):
#                         vel_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, vel_diff))
#                     if (theta_diff_abs > THETA_THRESHOLD):
#                         theta_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, theta_diff))
#                     if (phi_diff_abs > PHI_THRESHOLD):
#                         phi_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, phi_diff))
#         return subset_match_count, all_match_count
#     final_df[['det_subset_params_match_count', 'det_all_params_match_count']] = final_df.apply(count_det_params_matches, axis=1, result_type="expand")
#     final_df['same_num_of_AF_detections'] = final_df['num_af_det_veh'] == final_df['num_af_det_sim']
#     final_df['matching_pct_det_all_params'] = final_df['det_all_params_match_count']/final_df['num_af_det_veh']
#     final_df['matching_pct_det_subset_params'] = final_df['det_subset_params_match_count']/final_df['num_af_det_veh']
#     accuracy_list = final_df['matching_pct_det_all_params'].tolist()
#     num_of_dets_in_veh_af = sum(final_df['num_af_det_veh'])
#     num_of_dets_in_sim_af = sum(final_df['num_af_det_sim'])
#     num_of_dets_with_matching_rv_af = sum(final_df['det_subset_params_match_count'])
#     num_of_dets_with_matching_rvtp_af = sum(final_df['det_all_params_match_count'])
#     num_of_SI_with_matching_rv_af = final_df[(final_df['same_num_of_AF_detections'] == 1) & (final_df['matching_pct_det_subset_params'] == 1)].shape[0]
#     num_of_SI_with_matching_rvtp_af = final_df[(final_df['same_num_of_AF_detections'] == 1) & (final_df['matching_pct_det_all_params'] == 1)].shape[0]
#     if num_of_dets_in_veh_af != 0:
#         overall_accuracy = round((num_of_dets_with_matching_rvtp_af / num_of_dets_in_veh_af) * 100, 2)
#         min_accuracy = round(min(accuracy_list)*100, 2)
#     else:
#         overall_accuracy = 0
#         min_accuracy = 0
#     kpis_af = {'result1':
#                     {'numerator': num_of_SI_with_same_num_of_dets_af,
#                      'denominator': num_of_same_SI_in_veh_and_sim_af,
#                      'value': round((num_of_SI_with_same_num_of_dets_af / num_of_same_SI_in_veh_and_sim_af) * 100,2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
#                 'result2':
#                     {'numerator': num_of_SI_with_matching_rv_af,
#                      'denominator': num_of_SI_with_same_num_of_dets_af,
#                      'value': round((num_of_SI_with_matching_rv_af / num_of_SI_with_same_num_of_dets_af) * 100, 2) if (num_of_SI_with_same_num_of_dets_af != 0) else None},
#                 'result3':
#                     {'numerator': num_of_SI_with_matching_rv_af,
#                      'denominator': num_of_same_SI_in_veh_and_sim_af,
#                      'value': round((num_of_SI_with_matching_rv_af / num_of_same_SI_in_veh_and_sim_af) * 100, 2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
#                 'result4':
#                     {'numerator': num_of_SI_with_matching_rvtp_af,
#                      'denominator': num_of_SI_with_same_num_of_dets_af,
#                      'value': round(( num_of_SI_with_matching_rvtp_af / num_of_SI_with_same_num_of_dets_af) * 100, 2) if (num_of_SI_with_same_num_of_dets_af != 0) else None},
#                 'result5':
#                     {'numerator': num_of_SI_with_matching_rvtp_af,
#                      'denominator': num_of_same_SI_in_veh_and_sim_af,
#                      'value': round((num_of_SI_with_matching_rvtp_af / num_of_same_SI_in_veh_and_sim_af) * 100, 2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
#                 'result6':
#                     {'numerator': num_of_dets_with_matching_rv_af,
#                      'denominator': num_of_dets_in_veh_af,
#                      'value': round((num_of_dets_with_matching_rv_af / num_of_dets_in_veh_af) * 100, 2) if (num_of_dets_in_veh_af != 0) else None},
#                 'result7':
#                     {'numerator': num_of_dets_with_matching_rvtp_af,
#                      'denominator': num_of_dets_in_veh_af,
#                      'value': round((num_of_dets_with_matching_rvtp_af / num_of_dets_in_veh_af) * 100, 2) if (num_of_dets_in_veh_af != 0) else None},
#                 }
#     custom_print(f"Number of SI in (vehicle, simulation): ({num_of_SI_in_veh_af}, {num_of_SI_in_sim_af})")
#     custom_print(f"Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_af}")
#     custom_print(f"% of SI with same number of AF detections: "
#           f"{kpis_af['result1']['numerator']}/{kpis_af['result1']['denominator']} --> {kpis_af['result1']['value']}%" )
#     custom_print(f"% of SI with 100% matching det params(r,v): "
#           f"{kpis_af['result2']['numerator']}/{kpis_af['result2']['denominator']} --> {kpis_af['result2']['value']}%, "
#           f"{kpis_af['result3']['numerator']}/{kpis_af['result3']['denominator']} --> {kpis_af['result3']['value']}%")
#     custom_print(f"% of SI with 100% matching det params(r,v,t,p): "
#           f"{kpis_af['result4']['numerator']}/{kpis_af['result4']['denominator']} --> {kpis_af['result4']['value']}%, "
#           f"{kpis_af['result5']['numerator']}/{kpis_af['result5']['denominator']} --> {kpis_af['result5']['value']}%")
#     custom_print(f"Number of detections in (vehicle, simulation): ({num_of_dets_in_veh_af}, {num_of_dets_in_sim_af})")
#     custom_print(f"Accuracy(r,v): "
#           f"{kpis_af['result6']['numerator']}/{kpis_af['result6']['denominator']} --> {kpis_af['result6']['value']}%")
#     custom_print(f"Accuracy(r,v,t,p): "
#           f"{kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']} --> {kpis_af['result7']['value']}%")
#     #################################
#     # HTML Content
#     #################################
#     html_content += f"""
#     <b>KPI:</b> Accuracy: ({kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']}) --> <b>{kpis_af['result7']['value']}%</b>
#     <details>
#         <summary><i>Details</i></summary>
#         <b>RDD Streams</b><br>
#         Number of SI in (vehicle, simulation) : {num_of_SI_in_veh_rdd}, {num_of_SI_in_sim_rdd}<br>
#         Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_rdd}<br>
#         % of SI with same number of RDD1 detections: 
#         ({kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']}) --> {kpis_rdd['result1']['value']}%<br>
#         % of SI with 100% matching (rindx, dindx) pair:
#         ({kpis_rdd['result2']['numerator']}/{kpis_rdd['result2']['denominator']}) --> {kpis_rdd['result2']['value']}%,
#         ({kpis_rdd['result3']['numerator']}/{kpis_rdd['result3']['denominator']}) --> {kpis_rdd['result3']['value']}%,
#         <br>
#         % of SI with 100% matching (range, rangerate) pair:
#         ({kpis_rdd['result4']['numerator']}/{kpis_rdd['result4']['denominator']}) --> {kpis_rdd['result4']['value']}%,
#         ({kpis_rdd['result5']['numerator']}/{kpis_rdd['result5']['denominator']}) --> {kpis_rdd['result5']['value']}%,
#         <br>
#         <b>Detection Streams</b><br>
#         Number of SI in (vehicle, simulation) : {num_of_SI_in_veh_af}, {num_of_SI_in_sim_af}<br>
#         Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_af}<br>
#         % of SI with same number of AF detections: 
#         ({kpis_af['result1']['numerator']}/{kpis_af['result1']['denominator']}) --> {kpis_af['result1']['value']}%
#         <br>
#         % of SI with 100% matching det params(r,v):
#         ({kpis_af['result2']['numerator']}/{kpis_af['result2']['denominator']}) --> {kpis_af['result2']['value']}%,
#         ({kpis_af['result3']['numerator']}/{kpis_af['result3']['denominator']}) --> {kpis_af['result3']['value']}%,
#         <br>
#         % of SI with 100% matching det params(r,v,t,p):
#         ({kpis_af['result4']['numerator']}/{kpis_af['result4']['denominator']}) --> {kpis_af['result4']['value']}%,
#         ({kpis_af['result5']['numerator']}/{kpis_af['result5']['denominator']}) --> {kpis_af['result5']['value']}%,
#         <br>
#         Number of detections in (vehicle, simulation): {num_of_dets_in_veh_af}, {num_of_dets_in_sim_af}<br>
#         Accuracy(r,v):
#         ({kpis_af['result6']['numerator']}/{kpis_af['result6']['denominator']}) --> {kpis_af['result6']['value']}%,
#         Accuracy(r,v,t,p):
#         ({kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']}) --> {kpis_af['result7']['value']}%,
#         <br>
#     """
#     #################################
#     return True
# def func_bar(diffs):
#     # Count the frequency of each unique value
#     counts = Counter(diffs)
#     # Total number of observations
#     total_observations = len(diffs)
#     # Calculate percentages
#     percentages = {key: (value / total_observations) * 100 for key, value in counts.items()}
#     sorted_dict = dict(sorted(percentages.items()))
#     # Return Bar trace
#     return go.Bar(x=list(sorted_dict.keys()), y=list(sorted_dict.values()))
# def func_scatter(x, y):
#     # Return Scatter trace
#     return go.Scatter(x=x, y=y, mode='markers')
# def func_line(x, y):
#     # Return Scatter trace
#     return go.Scatter(x=x, y=y, mode='lines')
# def plot_stats():
#     #################################
#     # Global variables
#     #################################
#     global html_content
#     global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list
#     global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list, saturated_list
#     #################################
#     rng_idx = 0
#     vel_idx = 1
#     theta_idx = 2
#     phi_idx = 3
#     single_target_idx = 4
#     superres_target_idx = 5
#     bistatic_idx = 6
#     error_idx = 7
#     ran_diffs = np.array([ele[error_idx] for ele in ran_diff_list]).round(3)
#     vel_diffs = np.array([ele[error_idx] for ele in vel_diff_list]).round(3)
#     theta_diffs = np.array([ele[error_idx] for ele in theta_diff_list]).round(3)
#     phi_diffs = np.array([ele[error_idx] for ele in phi_diff_list]).round(3)
#     #################################
#     # Create subplots for error bar plots:
#     #################################
#     fig_bar = sp.make_subplots(rows=2, cols=2, horizontal_spacing=0.04, vertical_spacing=0.2)
#     # Manually add traces for bar plots
#     fig_bar.add_trace(func_bar(ran_diffs), row=1, col=1)
#     fig_bar.update_yaxes(title_text="Percentage(%)", row=1, col=1)
#     fig_bar.update_xaxes(title_text="Range error", row=1, col=1)
#     fig_bar.add_trace(func_bar(vel_diffs), row=1, col=2)
#     fig_bar.update_xaxes(title_text="Range Rate error", row=1, col=2)
#     fig_bar.add_trace(func_bar(theta_diffs), row=2, col=1)
#     fig_bar.update_yaxes(title_text="Percentage(%)", row=2, col=1)
#     fig_bar.update_xaxes(title_text="Azimuth error", row=2, col=1)
#     fig_bar.add_trace(func_bar(phi_diffs), row=2, col=2)
#     fig_bar.update_xaxes(title_text="Elevation error", row=2, col=2)
#     # Update layout for bar plot
#     fig_bar.update_layout(height=700, width=1250, title_text="% of unmatched detections v/s error", showlegend=False)
#     fig_bar.update_traces(marker_color='red')
#     fig_bar.update_xaxes(zeroline=False, showgrid=False, type='category')
#     fig_bar.update_yaxes(zeroline=False, showgrid=False)
#     #fig_bar.show()
#     bar_plot_html = pio.to_html(fig_bar, full_html=False, include_plotlyjs='cdn')
#     #################################
#     #################################
#     # Create subplots for error v/s different detection properties plots:
#     #################################
#     fig_scatter = sp.make_subplots(rows=4, cols=7, horizontal_spacing=0.03, vertical_spacing=0.04)
#     # Manually add traces for scatter plots
#     # Range error against detection properties
#     row_num = 1
#     rang = [ele[rng_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(rang, ran_diffs), row=row_num, col=1)
#     fig_scatter.update_yaxes(title_text="Range error", row=row_num, col=1)
#     fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
#     vel = [ele[vel_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(vel, ran_diffs), row=row_num, col=2)
#     fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
#     theta = [ele[theta_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(theta, ran_diffs), row=row_num, col=3)
#     fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
#     phi = [ele[phi_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(phi, ran_diffs), row=row_num, col=4)
#     fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
#     single_target = [ele[single_target_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(single_target, ran_diffs), row=row_num, col=5)
#     fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
#     superres_target = [ele[superres_target_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(superres_target, ran_diffs), row=row_num, col=6)
#     fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
#     bistatic = [ele[bistatic_idx] for ele in ran_diff_list]
#     fig_scatter.add_trace(func_scatter(bistatic, ran_diffs), row=row_num, col=7)
#     fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)
#     # Range rate error against detection properties
#     row_num = 2
#     rang = [ele[rng_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(rang, vel_diffs), row=row_num, col=1)
#     fig_scatter.update_yaxes(title_text="Range Rate error", row=row_num, col=1)
#     fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
#     vel = [ele[vel_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(vel, vel_diffs), row=row_num, col=2)
#     fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
#     theta = [ele[theta_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(theta, vel_diffs), row=row_num, col=3)
#     fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
#     phi = [ele[phi_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(phi, vel_diffs), row=row_num, col=4)
#     fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
#     single_target = [ele[single_target_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(single_target, vel_diffs), row=row_num, col=5)
#     fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
#     superres_target = [ele[superres_target_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(superres_target, vel_diffs), row=row_num, col=6)
#     fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
#     bistatic = [ele[bistatic_idx] for ele in vel_diff_list]
#     fig_scatter.add_trace(func_scatter(bistatic, vel_diffs), row=row_num, col=7)
#     fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)
#     # Azimuth error against detection properties
#     row_num = 3
#     rang = [ele[rng_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(rang, theta_diffs), row=row_num, col=1)
#     fig_scatter.update_yaxes(title_text="Azimuth error", row=row_num, col=1)
#     fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
#     vel = [ele[vel_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(vel, theta_diffs), row=row_num, col=2)
#     fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
#     theta = [ele[theta_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(theta, theta_diffs), row=row_num, col=3)
#     fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
#     phi = [ele[phi_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(phi, theta_diffs), row=row_num, col=4)
#     fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
#     single_target = [ele[single_target_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(single_target, theta_diffs), row=row_num, col=5)
#     fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
#     superres_target = [ele[superres_target_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(superres_target, theta_diffs), row=row_num, col=6)
#     fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
#     bistatic = [ele[bistatic_idx] for ele in theta_diff_list]
#     fig_scatter.add_trace(func_scatter(bistatic, theta_diffs), row=row_num, col=7)
#     fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)
#     # Elevation error against detection properties
#     row_num = 4
#     rang = [ele[rng_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(rang, phi_diffs), row=row_num, col=1)
#     fig_scatter.update_yaxes(title_text="Elevation error", row=row_num, col=1)
#     fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
#     vel = [ele[vel_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(vel, phi_diffs), row=row_num, col=2)
#     fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
#     theta = [ele[theta_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(theta, phi_diffs), row=row_num, col=3)
#     fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
#     phi = [ele[phi_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(phi, phi_diffs), row=row_num, col=4)
#     fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
#     single_target = [ele[single_target_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(single_target, phi_diffs), row=row_num, col=5)
#     fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
#     superres_target = [ele[superres_target_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(superres_target, phi_diffs), row=row_num, col=6)
#     fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
#     bistatic = [ele[bistatic_idx] for ele in phi_diff_list]
#     fig_scatter.add_trace(func_scatter(bistatic, phi_diffs), row=row_num, col=7)
#     fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)
#     # Update layout for scatter plot
#     fig_scatter.update_layout(height=1400, width=1250,
#                               title_text="Error v/s detection Range, Range Rate, Azimuth, Elevation", showlegend=False)
#     fig_scatter.update_traces(marker_color='red', marker={'size': 2})
#     fig_scatter.update_xaxes(zeroline=False, showgrid=False)
#     fig_scatter.update_yaxes(zeroline=False, showgrid=False)
#     #fig_scatter.show()
#     scatter_plot_html = pio.to_html(fig_scatter, full_html=False, include_plotlyjs='cdn')
#     #################################
#     #################################
#     # Create subplots for different parameters v/s scanindex:
#     #################################
#     fig_line = sp.make_subplots(rows=4, cols=1, horizontal_spacing=0.04, vertical_spacing=0.1)
#     # Manually add traces for line plots
#     row_num = 1
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=accuracy_list, mode="lines", name="Accuracy", line=dict(color="red")), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="Accuracy", row=row_num, col=1)
#     fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)
#     row_num = 2
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=num_af_det_veh_list, mode="lines", name="Veh", line=dict(color="red")), row=row_num, col=1)
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=num_af_det_sim_list, mode="lines", name="Sim", line=dict(color="blue")), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="Num of dets", row=row_num, col=1)
#     fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)
#     row_num = 3
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=max_range_veh_list, mode="lines", name="Veh", line=dict(color="red")), row=row_num, col=1)
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=max_range_sim_list, mode="lines", name="Sim", line=dict(color="blue")), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="Max range", row=row_num, col=1)
#     fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)
#     row_num = 4
#     fig_line.add_trace(go.Scatter(x=scan_index_list, y=saturated_list, mode="lines", name="Saturation flag", line=dict(color="red")), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="Saturation flag", row=row_num, col=1)
#     fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)
#     # Update layout for plot
#     fig_line.update_layout(height=1200, width=1250, title_text="Params v/s scanindex", showlegend=True)
#     fig_line.update_xaxes(zeroline=False, showgrid=False, type='category')
#     fig_line.update_yaxes(zeroline=False, showgrid=False)
#     #fig_line.show()
#     params_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs='cdn')
#     #################################
#     #################################
#     # HTML Content
#     #################################
#     html_content += f"""
#         <b><u>Plots:</u></b>
#         <details>
#             <summary><i>Plot A</i></summary>
#             {bar_plot_html}
#         </details>
#         <details>
#             <summary><i>Plot B</i></summary>
#             {scatter_plot_html}
#         </details>
#         <details>
#             <summary><i>Plot C</i></summary>
#             {params_plot_html}
#         </details>
#     </details>
#     <hr>
#     """
# def plo_data_across_logs(data_across_log):
#     #################################
#     # Global variables
#     #################################
#     global html_content
#     global cdc_data_available
#     #################################
#     #################################
#     # Create sub-plot for different data across logs:
#     #################################
#     fig_line = sp.make_subplots(rows=3, cols=1, horizontal_spacing=0.04, vertical_spacing=0.1)
#     sensor_color_dict = {"FC": "red",
#                          "FL": "green",
#                          "FR": "blue",
#                          "RL": "violet",
#                          "RR": "black"}
#     row_num = 1
#     for sensor in data_across_log.keys():
#         min_acc = [data['min_accuracy'] for data in data_across_log[sensor]]
#         overall_acc = [data['overall_accuracy'] for data in data_across_log[sensor]]
#         color = sensor_color_dict[sensor]
#         fig_line.add_trace(go.Scatter(x=list(range(1, len(min_acc) + 1)), y=min_acc, mode="lines+markers", opacity=.3, name=sensor+"_min_acc", line=dict(color=color, dash='dash')), row=row_num, col=1)
#         fig_line.add_trace(go.Scatter(x=list(range(1, len(overall_acc)+1)), y=overall_acc, mode="lines+markers", name=sensor+"_overall_acc", line=dict(color=color)), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="Accuracy", row=row_num, col=1)
#     if cdc_data_available:
#         row_num += 1
#         for sensor in data_across_log.keys():
#             cdc_satur_perc = [data['perc_of_scans_with_cdc_saturation'] for data in data_across_log[sensor]]
#             color = sensor_color_dict[sensor]
#             fig_line.add_trace(go.Scatter(x=list(range(1, len(cdc_satur_perc) + 1)), y=cdc_satur_perc, mode="lines+markers", name=sensor, line=dict(color=color)), row=row_num, col=1)
#         fig_line.update_yaxes(title_text="% Of Scans With CDC Saturation", row=row_num, col=1)
#     row_num += 1
#     for sensor in data_across_log.keys():
#         rng_satur_perc = [data['perc_of_scans_with_range_saturation'] for data in data_across_log[sensor]]
#         color = sensor_color_dict[sensor]
#         fig_line.add_trace(go.Scatter(x=list(range(1, len(rng_satur_perc) + 1)), y=rng_satur_perc, mode="lines+markers", name=sensor, line=dict(color=color)), row=row_num, col=1)
#     fig_line.update_yaxes(title_text="% Of Scans With Range Saturation", row=row_num, col=1)
#     fig_line.update_layout(height=900, width=1250, title_text="Data across logs", showlegend=True)
#     fig_line.update_xaxes(zeroline=False, showgrid=False, type='category')
#     fig_line.update_yaxes(zeroline=False, showgrid=False)
#     #fig_line.show()
#     data_across_logs_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs='cdn')
#     #################################
#     #################################
#     # HTML Content
#     #################################
#     html_content += f"""
#             {data_across_logs_plot_html}
#     <hr width="100%" size="2" color="blue" noshade>
#     """
#     #################################
# def find_file_pairs(base_path):
#     # Pattern to identify version strings
#     resim_version_pattern = re.compile(r'_r\d{8}_')
#     # Create dictionary for resim files {base_name: resim_file}
#     input_output_file_dict = {}
#     for filename in os.listdir(base_path):
#         # go through only resim files
#         if resim_version_pattern.search(filename):
#             # Create base name by removing version string
#             base_name = resim_version_pattern.sub('_', filename)
#             input_output_file_dict[base_name] = filename
#     return input_output_file_dict
# def find_data_files(base_path):
#     data_files = {
#         'input': [],
#         'output': []
#     }
#     # Use only those files which have a corresponding resim file
#     file_pair_dict = find_file_pairs(base_path)
#     input_files = list(file_pair_dict.keys())
#     files_of_interest = [s for s in input_files if DET_FILE_SUFFIX in s or RDD_FILE_SUFFIX in s]
#     unique_input_files = set(s.replace(DET_FILE_SUFFIX, "").replace(RDD_FILE_SUFFIX, "").strip() for s in files_of_interest)
#     for filename in sorted(unique_input_files):
#         det_filename = filename + DET_FILE_SUFFIX
#         rdd_filename = filename + RDD_FILE_SUFFIX
#         cdc_filename = filename + CDC_FILE_SUFFIX
#         vse_filename = filename + VSE_FILE_SUFFIX
#         input_det_file_path = os.path.join(base_path, det_filename)
#         input_rdd_file_path = os.path.join(base_path, rdd_filename)
#         input_cdc_file_path = os.path.join(base_path, cdc_filename)
#         input_vse_file_path = os.path.join(base_path, vse_filename)
#         if(det_filename in file_pair_dict.keys() and
#            rdd_filename in file_pair_dict.keys()): # not checking explicitly for CDC or VSE file as it may not be available always
#             output_det_file_path = os.path.join(base_path, file_pair_dict[det_filename])
#             output_rdd_file_path = os.path.join(base_path, file_pair_dict[rdd_filename])
#             output_cdc_file_path = ""
#             if cdc_filename in file_pair_dict.keys():
#                 output_cdc_file_path = os.path.join(base_path, file_pair_dict[cdc_filename])
#             output_vse_file_path = ""
#             if vse_filename in file_pair_dict.keys():
#                 output_vse_file_path = os.path.join(base_path, file_pair_dict[vse_filename])
#             # Do not change the order in which the file paths are added,
#             # these files paths are accessed later using indexing
#             data_files['input'].append((input_det_file_path,
#                                         input_rdd_file_path,
#                                         input_cdc_file_path,
#                                         input_vse_file_path))
#             data_files['output'].append((output_det_file_path,
#                                          output_rdd_file_path,
#                                          output_cdc_file_path,
#                                          output_vse_file_path))
#     return data_files
# def process_logs(data_files):
#     start_time = time.time()
#     #################################
#     # Global variables
#     #################################
#     global num_of_SI_in_veh_af, num_of_SI_in_sim_af, num_of_same_SI_in_veh_and_sim_af
#     global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list, snr_diff_list, rcs_diff_list
#     global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list, saturated_list
#     global html_content, html_header, min_accuracy, overall_accuracy, total_real_data, total_sim_data
#     global cdc_data_available, perc_of_scans_with_cdc_saturation, perc_of_scans_with_range_saturation
#     global MAX_NUM_OF_AF_DETS, RANGE_SATURATION_THRESHOLD
#     global num_of_SI_available_in_veh, num_of_SI_available_for_kpi
#     global dist_travelled_by_veh, dist_travelled_by_sim
#     global output_folder
#     #################################
#     data_across_log = {'FC':[], 'FL':[], 'FR':[], 'RL':[], 'RR':[]}
#     num_of_logs = len(data_files['input'])
#     for log_idx in range(num_of_logs): # log_idx should start from 0 itself as it is used to index objects
#         #################################
#         # Reset the below variables for each log
#         #################################
#         num_of_SI_in_veh_af = 0
#         num_of_SI_in_sim_af = 0
#         num_of_same_SI_in_veh_and_sim_af = 0
#         ran_diff_list = []
#         vel_diff_list = []
#         theta_diff_list = []
#         phi_diff_list = []
#         snr_diff_list = []
#         rcs_diff_list = []
#         scan_index_list = []
#         accuracy_list = []
#         num_af_det_veh_list = []
#         num_af_det_sim_list = []
#         max_range_veh_list = []
#         max_range_sim_list = []
#         saturated_list = []
#         min_accuracy = 0
#         overall_accuracy = 0
#         total_real_data = 0
#         total_sim_data = 0
#         cdc_data_available = False
#         perc_of_scans_with_cdc_saturation = 0
#         perc_of_scans_with_range_saturation = 0
#         #################################
#         # Each log should have atleast 4 csv files. CDC files are optional
#         input_det_file = data_files['input'][log_idx][0]
#         input_rdd_file = data_files['input'][log_idx][1]
#         input_cdc_file = data_files['input'][log_idx][2]
#         input_vse_file = data_files['input'][log_idx][3]
#         output_det_file = data_files['output'][log_idx][0]
#         output_rdd_file = data_files['output'][log_idx][1]
#         output_cdc_file = data_files['output'][log_idx][2]
#         output_vse_file = data_files['output'][log_idx][3]
#         det_log_name = os.path.basename(input_det_file)
#         det_log_name_wo_ext = det_log_name.replace(DET_FILE_SUFFIX, "")
#         custom_print(f"\n[INFO] Processing log: {log_idx+1}/{num_of_logs} : {det_log_name_wo_ext}")
#         #################################
#         # check if the csv files exists.
#         # If any of the csv files have issues, then simply continue on to next log
#         #################################
#         if not check_file(input_det_file):
#             continue
#         if not check_file(input_rdd_file):
#             continue
#         if not check_file(output_det_file):
#             continue
#         if not check_file(output_rdd_file):
#             continue
#         #################################
#         html_content += f"""
#                 <b>Log:</b> {det_log_name_wo_ext};
#                 """
#         # Few variables are different for front and corner radars, 
#         # hence, it has to be updated for every log based on the log name
#         if '_FC_UDP_' in det_log_name:
#             MAX_NUM_OF_AF_DETS = MAX_NUM_OF_AF_DETS_FRONT_RADAR
#             RANGE_SATURATION_THRESHOLD = RANGE_SATURATION_THRESHOLD_FRONT_RADAR
#         else:
#             MAX_NUM_OF_AF_DETS = MAX_NUM_OF_AF_DETS_CORNER_RADAR
#             RANGE_SATURATION_THRESHOLD = RANGE_SATURATION_THRESHOLD_CORNER_RADAR
#         #################################
#         # Main function for each log
#         #################################
#         log_start_time = time.time()    
#         status = main(input_det_file, output_det_file,
#                       input_rdd_file, output_rdd_file,
#                       input_cdc_file, output_cdc_file,
#                       input_vse_file, output_vse_file)        
#         log_end_time = time.time()
#         log_execution_time = log_end_time - log_start_time
#         custom_print(f"[INFO] Log {det_log_name_wo_ext} processed in {log_execution_time:.2f} seconds")
#         #################################
#         # Save the data and plot data only if the log was successfully processed
#         if status:
#             #################################
#             # Save required data of each log
#             #################################
#             data_dict = {
#                 "det_log_name_wo_ext": det_log_name_wo_ext,
#                 "min_accuracy": min_accuracy, 
#                 "overall_accuracy": overall_accuracy,
#                 "perc_of_scans_with_cdc_saturation": perc_of_scans_with_cdc_saturation,
#                 "perc_of_scans_with_range_saturation": perc_of_scans_with_range_saturation,
#                 "num_of_SI_available_in_veh": num_of_SI_available_in_veh,
#                 "num_of_SI_available_for_kpi": num_of_SI_available_for_kpi,
#                 "dist_travelled_by_veh": dist_travelled_by_veh,
#                 "dist_travelled_by_sim": dist_travelled_by_sim,                
#                 "log_idx": log_idx}
#             found_sensor = False
#             for sensor in ['FC', 'FL', 'FR', 'RL', 'RR']:
#                 if ('_' + sensor + '_UDP_') in det_log_name:
#                     data_across_log[sensor].append(data_dict)
#                     found_sensor = True
#                     break
#             if found_sensor == False:
#                 custom_print(f"[WARNING] Unsupported sensor position in {det_log_name}")
#             #################################
#             #################################
#             # Plot statistics of each log
#             #################################
#             plot_stats()
#             #################################
#         else:
#             html_content += f"""
#                 <b>NA</b>
#                 <hr>
#                 """
#         #################################
#         # Create HTML files for every MAX_LOGS_IN_ONE_REPORT logs
#         #################################
#         if(((log_idx+1) % MAX_LOGS_IN_ONE_REPORT == 0) or ((log_idx+1) == num_of_logs)):
#             #################################
#             # Table for yield data      
#             #################################
#             for sensor in data_across_log:
#                 sensor_data_list = data_across_log[sensor]
#                 sensor_num_of_SI_available_in_veh = sum([(sensor_data_dict['num_of_SI_available_in_veh']) for sensor_data_dict in sensor_data_list])
#                 sensor_num_of_SI_available_for_kpi = sum([(sensor_data_dict['num_of_SI_available_for_kpi']) for sensor_data_dict in sensor_data_list])
#                 sensor_dist_travelled_by_veh = sum([(sensor_data_dict['dist_travelled_by_veh']) for sensor_data_dict in sensor_data_list])
#                 sensor_dist_travelled_by_sim = sum([(sensor_data_dict['dist_travelled_by_sim']) for sensor_data_dict in sensor_data_list])
                 
#                 kpi_yield = 0
#                 if sensor_num_of_SI_available_in_veh != 0:
#                     kpi_yield = round((sensor_num_of_SI_available_for_kpi/sensor_num_of_SI_available_in_veh)*100, 2)
#                 sensor_dist_travelled_by_veh_km = round((sensor_dist_travelled_by_veh * M_2_KM), 2)
#                 sensor_dist_travelled_by_sim_km = round((sensor_dist_travelled_by_sim * M_2_KM), 2)
#                 mileage_yield = 0
#                 if sensor_dist_travelled_by_veh != 0:
#                     mileage_yield = round((sensor_dist_travelled_by_sim/sensor_dist_travelled_by_veh)*100, 2)
#                 html_content += f"""
#                     <table>
#                         <tr>
#                             <td><strong style="font-size: 1em;">KPI Yield {sensor}</strong></td>
#                             <td style="text-align: right; width: 100px; font-family: 'Courier New', monospace;"><strong style="font-size: 1em;">: {kpi_yield:.2f}%</strong></td>
#                             <td><small>(Scans in veh = {sensor_num_of_SI_available_in_veh}, Scans in sim = {sensor_num_of_SI_available_for_kpi})</small></td>
#                         </tr>
#                         <tr>
#                             <td><strong style="font-size: 1em;">Mileage Yield {sensor}</strong></td>
#                             <td style="text-align: right; width: 100px; font-family: 'Courier New', monospace;"><strong style="font-size: 1em;">: {mileage_yield:.2f}%</strong></td>
#                             <td><small>(Veh mileage = {sensor_dist_travelled_by_veh_km}km, Sim mileage = {sensor_dist_travelled_by_sim_km}km)</small></td>
#                         </tr>
#                     </table>          
#                 """
                
#             html_content += f"""
#                 <hr width="100%" size="2" color="blue" noshade> 
#             """
#             #################################
            
#             #################################
#             # Table for failed logs   
#             #################################   
#             log_name_list = []
#             overall_accuracy_list = []
#             perc_of_scans_with_cdc_saturation_list = []
#             perc_of_scans_with_range_saturation_list = []
#             report_file_num_list = []
#             for sensor in data_across_log:
#                 for data in data_across_log[sensor]:
#                     log_name_list.append(data['det_log_name_wo_ext'])
#                     overall_accuracy_list.append(data['overall_accuracy'])
#                     perc_of_scans_with_cdc_saturation_list.append(data['perc_of_scans_with_cdc_saturation'])
#                     perc_of_scans_with_range_saturation_list.append(data['perc_of_scans_with_range_saturation'])
#                     report_file_num_list.append(((data['log_idx']//MAX_LOGS_IN_ONE_REPORT)+1)*MAX_LOGS_IN_ONE_REPORT)
#             html_content += f"""                
#                 <div style="margin: 20px 0;">
#                     <label for="threshold" style="font-weight: bold; margin-right: 2px;">
#                         Enter overall accuracy threshold value:
#                     </label>
#                     <input type="number" id="threshold" value="{DEFAULT_ACC_THRESHOLD}" 
#                     style="margin: 20px; padding: 4px;" oninput="updateTable()">
#                 </div>
                
#                 <div id="logs_based_on_acc"></div>
#                 <div id="logs_based_on_saturation"></div>
                
#                 <script>
#                     const fileNames = {log_name_list};
#                     const accuracies = {overall_accuracy_list};
#                     const perc_of_scans_with_cdc_saturations = {perc_of_scans_with_cdc_saturation_list};
#                     const perc_of_scans_with_range_saturations = {perc_of_scans_with_range_saturation_list};
#                     const report_file_nums = {report_file_num_list};
#                     function updateTable() {{
#                         const threshold = parseFloat(document.getElementById('threshold').value);
#                         if (isNaN(threshold)) {{
#                             document.getElementById('logs_based_on_acc').innerHTML = "<em>Please enter a valid threshold.</em>";
#                             return;
#                         }}
#                         document.getElementById('logs_based_on_saturation').innerHTML ="Hello";
                        
#                         // Table 1: Filter based on accuracy
#                         const filtered_based_on_acc = fileNames.map((name, i) => ({{
#                             name, 
#                             accuracy: accuracies[i],
#                             perc_of_scans_with_cdc_saturation: perc_of_scans_with_cdc_saturations[i],
#                             perc_of_scans_with_range_saturation: perc_of_scans_with_range_saturations[i],
#                             report_file_num: report_file_nums[i],
#                             isBelow: accuracies[i] < threshold
#                         }})).filter(item => item.isBelow);
                        
#                         const count_based_on_acc = filtered_based_on_acc.length;
#                         const tableHTML_based_on_acc = `
#                             <h3 style="color:red;">Logs with overall accuracy &lt; ${{threshold}}% (No. of logs: ${{count_based_on_acc}})</h3>
#                             <table style="width:80%; text-align: left; margin-top:20px; border-collapse:collapse;">
#                                 <tr style="background:#f0f0f0">
#                                     <th style="padding:10px">File Name</th>
#                                     <th style="padding:10px">Accuracy</th>
#                                     <th style="padding:10px">% of Scans with CDC Saturation</th>
#                                     <th style="padding:10px">% of Scans with Range Saturation</th>
#                                     <th style="padding:10px">Report File Number</th>
#                                 </tr>
#                                 ${{filtered_based_on_acc.map(item => `
#                                     <tr>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.name}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.accuracy}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.perc_of_scans_with_cdc_saturation}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.perc_of_scans_with_range_saturation}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.report_file_num}}</td>
#                                     </tr>
#                                 `).join('')}}
#                             </table>
#                         `;
#                         document.getElementById('logs_based_on_acc').innerHTML = tableHTML_based_on_acc;
                        
#                         // Table 2: Filter based on saturation
#                         const filtered_based_on_saturation = fileNames.map((name, i) => ({{
#                             name, 
#                             accuracy: accuracies[i],
#                             perc_of_scans_with_cdc_saturation: perc_of_scans_with_cdc_saturations[i],
#                             perc_of_scans_with_range_saturation: perc_of_scans_with_range_saturations[i],
#                             report_file_num: report_file_nums[i],
#                             isSaturated: (perc_of_scans_with_cdc_saturations[i] != 0) || (perc_of_scans_with_range_saturations[i] != 0)
#                         }})).filter(item => item.isSaturated);
                        
#                         const count_based_on_saturation = filtered_based_on_saturation.length;
#                         const tableHTML_based_on_saturation = `
#                             <h3 style="color:red;">Logs with either CDC or Range saturation (No. of logs: ${{count_based_on_saturation}})</h3>
#                             <table style="width:80%; text-align: left; margin-top:20px; border-collapse:collapse;">
#                                 <tr style="background:#f0f0f0">
#                                     <th style="padding:10px">File Name</th>
#                                     <th style="padding:10px">Accuracy</th>
#                                     <th style="padding:10px">% of Scans with CDC Saturation</th>
#                                     <th style="padding:10px">% of Scans with Range Saturation</th>
#                                     <th style="padding:10px">Report File Number</th>
#                                 </tr>
#                                 ${{filtered_based_on_saturation.map(item => `
#                                     <tr>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.name}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.accuracy}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.perc_of_scans_with_cdc_saturation}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.perc_of_scans_with_range_saturation}}</td>
#                                         <td style="padding:10px; border-bottom:1px solid #ddd">${{item.report_file_num}}</td>
#                                     </tr>
#                                 `).join('')}}
#                             </table>
#                         `;
#                         document.getElementById('logs_based_on_saturation').innerHTML = tableHTML_based_on_saturation;
#                     }}
#                     // Call updateTable on page load with default threshold
#                     window.onload = updateTable;
#                 </script>
#                 <hr width="100%" size="2" color="blue" noshade>
#             """
#             #################################
#             plo_data_across_logs(data_across_log)
            
#             html_content += "</body></html>" # end of html content for a set of logs
#             # Write HTML content to file
#             output_html = output_folder + f"/detection_kpi_report_{log_idx+1:04d}.html"
#             with open(output_html, "w") as f:
#                 f.write(html_content)
#                 f.close()
                
#             # start of html content for next set of logs
#             html_content = html_header
#         #################################
#     end_time = time.time()
#     execution_time = end_time - start_time
#     custom_print(f"[INFO] Script execution completed in {execution_time:.2f} seconds ({execution_time/60:.2f} minutes)")
# def custom_print(text):
#     #################################
#     # Global variables
#     #################################
#     global log_file
#     #################################
#     """Print text to both terminal and file."""
#     print(text)  # Print to terminal
#     print(text, file=log_file)  # Print to file
#     log_file.flush()  # Ensure it's written immediately
	
# #####################################################################
# # START
# #####################################################################
# proper_command_string = "Proper usage e.g.: python detection_matching_kpi_script.py log_path.txt meta_data.txt C:\\Gitlab\\gen7v1_resim_kpi_scripts"
# num_of_argv = len(sys.argv)  
# if num_of_argv < 3:
#     print("[COMMAND ERROR]: Not all arguments provided. " 
#           + proper_command_string)
#     sys.exit(1)
# elif num_of_argv == 3:
#     print(
#         "[COMMAND WARNING]: Output path not provided. Output will be generated in the path provided in log_path.txt file. "
#         + proper_command_string)
# else:
#     pass
# log_path_file = sys.argv[1]
# meta_data_file = sys.argv[2]
# # Check if log_path and meta_data files exists
# if not os.path.exists(log_path_file):
#     print(f"[ERROR]: File {log_path_file} does not exist.")
#     sys.exit(1)
# if not os.path.exists(meta_data_file):
#     print(f"[ERROR]: File {meta_data_file} does not exist.")
#     sys.exit(1)
    
# input_folder = ""
# with open(log_path_file, 'r') as f:
#     input_folder = f.readline().strip()
#     if not input_folder:
#         print(f"[ERROR]: Input folder path is not povided in log_path.txt.")
#         sys.exit(1)        
# # If input log_path file exists and if output folder path is not 
# # provided by user, then generate the output in the input
# # log_path itself
# output_folder = ""
# if(num_of_argv > 3):
#     output_folder = sys.argv[3]
# else:
#     output_folder = input_folder    
    
# log_file = open(output_folder+"/"+PRINT_LOG_FILE_NAME, 'w')
# #################################
# # Variables
# #################################
# num_of_SI_in_veh_af = 0
# num_of_SI_in_sim_af = 0
# num_of_same_SI_in_veh_and_sim_af = 0
# num_of_SI_available_in_veh = 0
# num_of_SI_available_for_kpi = 0
# dist_travelled_by_veh = 0
# dist_travelled_by_sim = 0
# ran_diff_list = []
# vel_diff_list = []
# theta_diff_list = []
# phi_diff_list = []
# snr_diff_list = []
# rcs_diff_list = []
# scan_index_list = []
# accuracy_list = []
# num_af_det_veh_list = []
# num_af_det_sim_list = []
# max_range_veh_list = []
# max_range_sim_list = []
# saturated_list = []
# min_accuracy = 0
# overall_accuracy = 0
# total_real_data = 0
# total_sim_data = 0
# cdc_data_available = False
# perc_of_scans_with_cdc_saturation = 0
# perc_of_scans_with_range_saturation = 0
# #################################
# #################################
# # Read meta data
# #################################
# # Initialize an empty dictionary
# metadata_dict = {}
# # Open and read the file
# with open(meta_data_file, "r") as file:
#     for line in file:
#         # Split each line by the first whitespace to get key and value
#         key, value = line.strip().split(maxsplit=1)
#         # Add to dictionary
#         metadata_dict[key] = value
# # Update variables to be used in other functions
# RESIM_MODE = metadata_dict['Mode'].upper()
# MAX_CDC_RECORDS = int(metadata_dict['Max_CDC_Records'])
# RANGE_SATURATION_THRESHOLD_FRONT_RADAR = int(metadata_dict['Range_Saturation_Thresh_Front_Radar'])
# RANGE_SATURATION_THRESHOLD_CORNER_RADAR = int(metadata_dict['Range_Saturation_Thresh_Corner_Radar'])
# #################################
# #################################
# # HTML Content
# #################################
# html_header = f"""
# <html>
# <head>
#     <title>{FILE_TITLE}</title>
#     <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
# </head>
# <body>
#     <h1>{FILE_TITLE} {FILE_VERSION}</h1>
#     <small>Unique key: {UNIQUE_KEY}</small>
#     <ul>
#         <li><b>SiL_Engine:</b> {metadata_dict['SiL_Engine']}</li>
#         <li><b>SW:</b> {metadata_dict['SW']}</li>
#         <li><b>RSP_SiL:</b> {metadata_dict['RSP_SiL']}</li>
#         <li><b>Tracker:</b> {metadata_dict['Tracker']}</li>
#         <li><b>Mode:</b> {metadata_dict['Mode']}</li>
#     </ul> 
#     <details>
#         <summary><b><i>Glossary</i></b></summary>
#         <ol>
#             <li><b>Definition of match:</b>
#                 A detection is said to match a re-simulated detection if it has the same RDD range and doppler index(within a scan index), 
#                 and the difference(error) in the range, range rate, azimuth and elevation are within the thresholds mentioned below
#                 <ul>
#                     <li>Range : {round(RAN_THRESHOLD, 5)} m</li>
#                     <li>Range rate : {round(VEL_THRESHOLD, 5)} m/s</li>
#                     <li>Azimuth : {round(THETA_THRESHOLD, 5)} radians</li>
#                     <li>Elevation : {round(PHI_THRESHOLD, 5)} radians</li>
#                 </ul> 
#             <li><b>Accuracy:</b> (Number of matching detections / total number of detections) * 100
#             <li><b>Saturation:</b> A SI is said to be saturated if the maximum range among all the vehicle detections is >= 135m and re-simulated detections is < 135m
#             <li><b>Plot A:</b> Plot of % of unmatched detections against the corresponding error. Unmatched detections are those 
#             detections having same RDD index in both vehicle and re-simulated data but falling outside at-least one of the above 
#             mentioned thresholds.
#             <li><b>Plot B:</b> Plot of different errors against the corresponding detection's range, range rate, azimuth and elevation
#             <li><b>Plot C:</b> Plot of different parameters across scan indices
#         </ol>
#         <b>Note:</b> The plots are interactive
#         <br>
#         <b>Note:</b> Blank plots indicate no data or zero error
#     </details>    
#     <hr width="100%" size="2" color="blue" noshade>
# """
# html_content = html_header
# #################################
# if input_folder:
#     data_files = find_data_files(input_folder)
#     # TODO ANANTHESH: Do we need to check for input logs availability here itself?
#     if data_files['input'] and data_files['output']:
#         process_logs(data_files)
#     else:
#         print(f"Error: No logs to process - either the input and/or the output csvs are missing.")
# #####################################################################
# # END
# #####################################################################
