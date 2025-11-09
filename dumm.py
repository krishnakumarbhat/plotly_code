#################################
# Packages
#################################
import sys
import os
import glob
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
from collections import Counter
import time
from datetime import datetime
# user defined
from constants import Constants
from meta_data import Metadata
from variables import Session_Vars, Log_Vars
from file_handling import check_file, find_det_related_data_files
from logger import logger
from config import config
# properties
pd.options.display.width = 0
#################################
#################################
# Define class objects
#################################
session_vars = Session_Vars()
log_vars = Log_Vars()
#################################
def process_one_log(veh_csv, sim_csv, veh_rdd_csv, sim_rdd_csv, veh_cdc_csv, sim_cdc_csv, veh_vse_csv, sim_vse_csv) -> bool:
    log_vars.reset()
    
    logger.custom_print("[INFO] Reading the CSVs...")    
    #################################
    # Read Vehicle and Resim data
    # Not all scans available in vehicle DET csv will be available in sim DET csv
    # Not all scans available in vehicle DET csv will be available in vehicle RDD csv
    # Not all scans available in sim DET csv will be available in sim RDD csv
    #################################
    if (config.MAX_NUM_OF_SI_TO_PROCESS != 0):
        nrows = config.MAX_NUM_OF_SI_TO_PROCESS
    else:
        nrows = None
    logger.custom_print("[INFO] Reading DET CSV...")
    #################################
    # Read DET csv
    #################################
    det_cols_of_interest = ['scan_index', 'num_af_det']
    det_cols_of_interest = det_cols_of_interest + [item for i in range(config.MAX_NUM_OF_AF_DETS) for item in (f"rdd_idx_{i}", f"ran_{i}", f"vel_{i}", f"theta_{i}", f"phi_{i}", f"f_single_target_{i}", f"f_superres_target_{i}", f"f_bistatic_{i}")]
    veh_det_df = pd.read_csv(veh_csv, usecols=det_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
    sim_det_df = pd.read_csv(sim_csv, usecols=det_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
    # Keep only non-zero scan indices
    veh_det_df = veh_det_df[veh_det_df['scan_index'] != 0]
    sim_det_df = sim_det_df[sim_det_df['scan_index'] != 0]
    # Keep only those scans which have non-zero AF dets
    veh_det_df = veh_det_df[veh_det_df['num_af_det'] != 0]
    sim_det_df = sim_det_df[sim_det_df['num_af_det'] != 0]
    # Drop duplicates if any
    veh_det_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)
    sim_det_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)    
    # Check if there is data after above filtering
    log_vars.num_of_SI_in_veh_af = veh_det_df.shape[0]
    log_vars.num_of_SI_in_sim_af = sim_det_df.shape[0]
    if(0 == log_vars.num_of_SI_in_veh_af):
        logger.custom_print("[WARNING] Filtered " + veh_csv + " is empty")
        return False
    if(0 == log_vars.num_of_SI_in_sim_af):
        logger.custom_print("[WARNING] Filtered " + sim_csv + " is empty")
        return False
    
    # Merge Vehicle and Resim DET data
    merged_det_df = pd.merge(veh_det_df, sim_det_df, on='scan_index', how='inner', suffixes=('_veh', '_sim'))
    if (config.MAX_NUM_OF_SI_TO_PROCESS != 0):
        merged_det_df = merged_det_df.iloc[:config.MAX_NUM_OF_SI_TO_PROCESS]
    log_vars.num_of_SI_in_veh_and_sim_af = merged_det_df.shape[0]
    log_vars.num_of_SI_with_same_num_of_dets_af = merged_det_df[merged_det_df['num_af_det_veh'] == merged_det_df['num_af_det_sim']].shape[0]
    
    # DET KPI    
    kpis_af = {'result1':
                    {'numerator': log_vars.num_of_SI_with_same_num_of_dets_af,
                     'denominator': log_vars.num_of_SI_in_veh_and_sim_af,
                     'value': round((log_vars.num_of_SI_with_same_num_of_dets_af / log_vars.num_of_SI_in_veh_and_sim_af) * 100,2) if (log_vars.num_of_SI_in_veh_and_sim_af != 0) else None},
              }
    # logger.custom_print(f"Number of SI in (vehicle, simulation): ({log_vars.num_of_SI_in_veh_af}, {log_vars.num_of_SI_in_sim_af})")
    # logger.custom_print(f"Number of same SI available in both vehicle and simulation: {log_vars.num_of_SI_in_veh_and_sim_af}")
    # logger.custom_print(f"% of SI with same number of AF detections: "
    #       f"{kpis_af['result1']['numerator']}/{kpis_af['result1']['denominator']} --> {kpis_af['result1']['value']}%")
    #################################
    
    #################################
    # Read CDC csv
    #################################
    veh_cdc_df = pd.DataFrame()
    if "CDC" in config.RESIM_MODE:
        logger.custom_print("[INFO] Reading CDC CSV...")
        session_vars.is_cdc_mode = True
        def read_all_related_cdc_csvs(base_filename):
            """
            Read all related CSV files into a single pandas DataFrame.
            Parameters:
            base_filename (str): The name of the first file (e.g., 'base_CDC.csv')
            Returns:
            pandas.DataFrame: Combined DataFrame from all related CSV files
            """
            # Extract the base pattern from the filename
            file_prefix = os.path.splitext(base_filename)[0]  # Remove extension
            # Create a pattern to match all related files
            pattern = f"{file_prefix}*.csv"
            # Get the directory of the base file
            directory = os.path.dirname(base_filename) or '.'
            # Find all matching files
            all_files = glob.glob(os.path.join(directory, pattern))
            #print(f"Found {len(all_files)} files matching pattern: {pattern}")
            # Read and combine all files
            df_list = []
            for file in sorted(all_files):
                #print(f"Reading {file}...")
                cdc_cols_of_interest = ['Scan_Index']
                df = pd.read_csv(file, usecols=cdc_cols_of_interest)  # type: ignore
                df_list.append(df)
            # Combine all dataframes
            if df_list:
                combined_df = pd.concat(df_list, ignore_index=True)
                #print(f"Combined DataFrame shape: {combined_df.shape}")
                return combined_df
            else:
                #print("No files found!")
                return pd.DataFrame()
        if check_file(veh_cdc_csv):
            veh_cdc_df = read_all_related_cdc_csvs(veh_cdc_csv)
            # Keep only non-zero scan indices
            veh_cdc_df = veh_cdc_df.rename(columns={'Scan_Index': 'scan_index'})
            veh_cdc_df = veh_cdc_df[veh_cdc_df['scan_index'] != 0]
            # Groupby scan_index
            veh_cdc_df = veh_cdc_df.groupby(['scan_index']).size().reset_index(name='num_cdc_records')  # type: ignore
            veh_cdc_df['cdc_saturated'] = veh_cdc_df['num_cdc_records'] == config.MAX_CDC_RECORDS
            # Check if there is data after above filtering
            log_vars.num_of_SI_in_veh_cdc = veh_cdc_df.shape[0]
            num_of_scans_with_cdc_saturation = int(veh_cdc_df['cdc_saturated'].sum())
            if log_vars.num_of_SI_in_veh_cdc != 0:
                log_vars.perc_of_scans_with_cdc_saturation = round((num_of_scans_with_cdc_saturation/log_vars.num_of_SI_in_veh_cdc)*100, 2)
            else:
                log_vars.perc_of_scans_with_cdc_saturation = 0
            log_vars.cdc_data_available = True
            logger.custom_print("[INFO] CDC data available...")
        else:
            log_vars.cdc_data_available = False
            logger.custom_print("[WARNING] CDC data not available...")
    #################################
    
    logger.custom_print("[INFO] Reading RDD CSV...")
    #################################
    # Read RDD csv
    #################################
    rdd_cols_of_interest = ['scan_index', 'rdd1_num_detect']
    rdd_cols_of_interest = rdd_cols_of_interest + [item for i in range(config.MAX_NUM_OF_RDD_DETS) for item in (f"rdd1_rindx_{i}", f"rdd1_dindx_{i}", f"rdd2_range_{i}", f"rdd2_range_rate_{i}")]
    veh_rdd_df = pd.read_csv(veh_rdd_csv, usecols=rdd_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
    sim_rdd_df = pd.read_csv(sim_rdd_csv, usecols=rdd_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
    # Keep only non-zero scan indices
    veh_rdd_df = veh_rdd_df[veh_rdd_df['scan_index'] != 0]
    sim_rdd_df = sim_rdd_df[sim_rdd_df['scan_index'] != 0]
    # Keep only those rows which have RDD1 dets
    veh_rdd_df = veh_rdd_df[veh_rdd_df['rdd1_num_detect'] != 0]
    sim_rdd_df = sim_rdd_df[sim_rdd_df['rdd1_num_detect'] != 0]
    # Drop duplicates if any
    veh_rdd_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)  # type: ignore
    sim_rdd_df.drop_duplicates(subset='scan_index', keep='first', inplace=True)  # type: ignore
    # Check if there is data after above filtering
    log_vars.num_of_SI_in_veh_rdd = veh_rdd_df.shape[0]
    log_vars.num_of_SI_in_sim_rdd = sim_rdd_df.shape[0]
    if(0 == log_vars.num_of_SI_in_veh_rdd):
        logger.custom_print("[WARNING] Filtered " + veh_rdd_csv + " is empty")
        return False
    if(0 == log_vars.num_of_SI_in_sim_rdd):
        logger.custom_print("[WARNING] Filtered " + sim_rdd_csv + " is empty")
        return False
    
    # Merge Vehicle and Resim RDD data
    merged_rdd_df = pd.merge(veh_rdd_df, sim_rdd_df, on='scan_index', suffixes=('_veh', '_sim'))
    if (config.MAX_NUM_OF_SI_TO_PROCESS != 0):
        merged_rdd_df = merged_rdd_df.iloc[:config.MAX_NUM_OF_SI_TO_PROCESS]
    log_vars.num_of_SI_in_veh_and_sim_rdd = merged_rdd_df.shape[0]
    log_vars.num_of_SI_with_same_num_of_dets_rdd = merged_rdd_df[merged_rdd_df['rdd1_num_detect_veh'] == merged_rdd_df['rdd1_num_detect_sim']].shape[0]
    
    # RDD KPI
    kpis_rdd = {'result1':
                {'numerator': log_vars.num_of_SI_with_same_num_of_dets_rdd,
                 'denominator': log_vars.num_of_SI_in_veh_and_sim_rdd,
                 'value': round((log_vars.num_of_SI_with_same_num_of_dets_rdd/log_vars.num_of_SI_in_veh_and_sim_rdd)*100, 2) if (log_vars.num_of_SI_in_veh_and_sim_rdd != 0) else None},
               }
    # logger.custom_print(f"Number of SI in (vehicle, simulation): ({log_vars.num_of_SI_in_veh_rdd}, {log_vars.num_of_SI_in_sim_rdd})")
    # logger.custom_print(f"Number of same SI available in both vehicle and simulation: {log_vars.num_of_SI_in_veh_and_sim_rdd}")
    # logger.custom_print(f"% of SI with same number of RDD1 detections: "
    #       f"{kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']} --> {kpis_rdd['result1']['value']}%" )
    #################################
    logger.custom_print("[INFO] Reading VSE CSV...")
    #################################
    # Read VSE csv
    #################################
    veh_vse_df = pd.DataFrame()
    sim_vse_df = pd.DataFrame()
    if check_file(veh_vse_csv) and check_file(sim_vse_csv):
        vse_cols_of_interest = ['scan_index', 'veh_speed']
        veh_vse_df = pd.read_csv(veh_vse_csv, usecols=vse_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
        sim_vse_df = pd.read_csv(sim_vse_csv, usecols=vse_cols_of_interest, nrows=nrows, memory_map=True)  # type: ignore
        # Keep only non-zero scan indices
        veh_vse_df = veh_vse_df[veh_vse_df['scan_index'] != 0]
        sim_vse_df = sim_vse_df[sim_vse_df['scan_index'] != 0]
        log_vars.vse_data_available = True
        logger.custom_print("[INFO] VSE data available...")
    else:
        log_vars.vse_data_available = False
        logger.custom_print("[WARNING] VSE data not available...")
    #################################    
    logger.custom_print("[INFO] RDD Detection Matching...")

    
    #################################
    # RDD Stream matching
    #################################    
    # Generate column names
    rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    range_cols_veh = [f"rdd2_range_{i}_veh" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    range_rate_cols_veh = [f"rdd2_range_rate_{i}_veh" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    range_cols_sim = [f"rdd2_range_{i}_sim" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    range_rate_cols_sim = [f"rdd2_range_rate_{i}_sim" for i in range(config.MAX_NUM_OF_RDD_DETS)]
    # Step 2: Compute matched (rdd1_rindx, rdd1_dindx) pairs
    def count_rindx_dindx_matches(row):
        """Count matching (rindx, dindx) pairs between veh and sim."""
        num_detect_veh = int(row['rdd1_num_detect_veh'])
        num_detect_sim = int(row['rdd1_num_detect_sim'])
        veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
        sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
        match_count = 0
        for veh_pair in veh_pairs:
            if veh_pair in sim_pairs:
                match_count += 1
                sim_pairs.remove(veh_pair)  # Avoid reuse
        return match_count
    merged_rdd_df['matched_rindx_dindx_pairs'] = merged_rdd_df.apply(count_rindx_dindx_matches, axis=1)
    # Step 3: Compute matches within thresholds for rdd2_range and rdd2_range_rate
    def count_range_matches(row):
        """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
        num_detect_veh = int(row['rdd1_num_detect_veh'])
        num_detect_sim = int(row['rdd1_num_detect_sim'])
        veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
        sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
        sim_data = dict(zip(sim_pairs, zip(row[range_cols_sim[:num_detect_sim]], row[range_rate_cols_sim[:num_detect_sim]])))
        match_count = 0
        for idx, veh_pair in enumerate(veh_pairs):
            if veh_pair in sim_data:
                sim_range, sim_range_rate = sim_data[veh_pair]
                sim_range = round(sim_range * Constants.SCALE_P21_TO_FLOAT, 3)
                sim_range_rate = round(sim_range_rate * Constants.SCALE_P21_TO_FLOAT, 3)
                veh_range = row[range_cols_veh[idx]]
                veh_range_rate = row[range_rate_cols_veh[idx]]
                veh_range = round(veh_range * Constants.SCALE_P21_TO_FLOAT, 3)
                veh_range_rate = round(veh_range_rate * Constants.SCALE_P21_TO_FLOAT, 3)
                if (abs(sim_range - veh_range) <= config.RAN_THRESHOLD
                    and abs(sim_range_rate - veh_range_rate) <= config.VEL_THRESHOLD):
                    match_count += 1
        return match_count
    merged_rdd_df['range_rangerate_matches'] = merged_rdd_df.apply(count_range_matches, axis=1)
    merged_rdd_df['matching_pct_rindx_dindx_pairs'] = merged_rdd_df['matched_rindx_dindx_pairs']/merged_rdd_df['rdd1_num_detect_veh']
    merged_rdd_df['matching_pct_range_rangerate_pairs'] = merged_rdd_df['range_rangerate_matches'] / merged_rdd_df['matched_rindx_dindx_pairs']
    num_of_SI_with_matching_rdd1_rindx_dindx_pair = merged_rdd_df[(merged_rdd_df['matching_pct_rindx_dindx_pairs'] >= 0.99)].shape[0]
    num_of_SI_with_matching_rdd1_rng_dop_pair = merged_rdd_df[(merged_rdd_df['matching_pct_rindx_dindx_pairs'] >= 0.99) & (merged_rdd_df['matching_pct_range_rangerate_pairs'] >= 0.99)].shape[0]
    kpis_merged_rdd = {'result1':
                {'numerator': num_of_SI_with_matching_rdd1_rindx_dindx_pair,
                 'denominator': log_vars.num_of_SI_in_veh_and_sim_rdd,
                 'value': round((num_of_SI_with_matching_rdd1_rindx_dindx_pair / log_vars.num_of_SI_in_veh_and_sim_rdd) * 100, 2) if (log_vars.num_of_SI_in_veh_and_sim_rdd != 0) else None},
            'result2':
                {'numerator': num_of_SI_with_matching_rdd1_rng_dop_pair,
                 'denominator': log_vars.num_of_SI_in_veh_and_sim_rdd,
                 'value': round((num_of_SI_with_matching_rdd1_rng_dop_pair / log_vars.num_of_SI_in_veh_and_sim_rdd) * 100, 2) if (log_vars.num_of_SI_in_veh_and_sim_rdd != 0) else None},
            }
    # logger.custom_print(f"% of SI with 99% matching (rindx, dindx) pair: "
    #       f"{kpis_merged_rdd['result1']['numerator']}/{kpis_merged_rdd['result1']['denominator']} --> {kpis_merged_rdd['result1']['value']}%")
    # logger.custom_print(f"% of SI with 99% matching (range, rangerate) pair: "
    #       f"{kpis_merged_rdd['result2']['numerator']}/{kpis_merged_rdd['result2']['denominator']} --> {kpis_merged_rdd['result2']['value']}%")
    #################################
     
    logger.custom_print("[INFO] Merging the dataframes...")
    #################################
    # Keep only those scans which are available in DET, RDD and CDC(if available) csvs
    # ###############################
    veh_merged_df = veh_det_df[veh_det_df['scan_index'].isin(veh_rdd_df['scan_index'])]
    if log_vars.cdc_data_available:
        veh_merged_df = veh_merged_df.merge(veh_cdc_df[['scan_index', 'cdc_saturated']], on='scan_index', how='inner')
    sim_merged_df = sim_det_df[sim_det_df['scan_index'].isin(sim_rdd_df['scan_index'])]
    # Check if there is data after above filtering
    log_vars.num_of_SI_in_veh_merged = veh_merged_df.shape[0]
    log_vars.num_of_SI_in_sim_merged = sim_merged_df.shape[0]
    if(0 == log_vars.num_of_SI_in_veh_merged):
        logger.custom_print("[WARNING] Filtered " + veh_csv + " is empty")
        return False
    if(0 == log_vars.num_of_SI_in_sim_merged):
        logger.custom_print("[WARNING] Filtered " + sim_csv + " is empty")
        return False
    #################################  
    
    #################################
    # Extract and append rdd1_rindx and rdd1_dindx values from rdd_df to det_df
    # based on rdd_idx columns of det_df
    #################################
    # Generate a dictionary for the new columns 'rdd1_rindx' and 'rdd1_dindx' with `None` values
    new_data = {f"rdd1_rindx_{i}": None for i in range(config.MAX_NUM_OF_AF_DETS)}
    new_data.update({f"rdd1_dindx_{i}": None for i in range(config.MAX_NUM_OF_AF_DETS)})
    
    # Create a new DataFrame with these columns
    new_columns_df = pd.DataFrame(new_data, index=veh_merged_df.index)
    # Concatenate the new columns to the existing DataFrame
    veh_merged_df = pd.concat([veh_merged_df, new_columns_df], axis=1)
    # Iterate through each row of veh_merged_df
    for idx, row in veh_merged_df.iterrows():
        scan_index = row['scan_index']
        # Extract corresponding row in veh_rdd_df
        rdd_row = veh_rdd_df[veh_rdd_df['scan_index'] == scan_index]
        # If a matching row is found in veh_rdd_df
        if not rdd_row.empty:
            # Iterate over each rdd_idx column in veh_merged_df
            for i in range(len([col for col in veh_merged_df.columns if col.startswith('rdd_idx')])):
                rdd_idx = int(row[f'rdd_idx_{i}'])
                # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
                veh_merged_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                veh_merged_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]
    # Display the updated veh_merged_df
    #logger.custom_print(veh_merged_df)
    # Extract and append rdd1_rindx and rdd1_dindx values from sim_rdd_df to sim_merged_df
    # based on rdd_idx columns of sim_merged_df
    new_columns_df = pd.DataFrame(new_data, index=sim_merged_df.index)
    # Concatenate the new columns to the existing DataFrame
    sim_merged_df = pd.concat([sim_merged_df, new_columns_df], axis=1)
    # Iterate through each row of sim_merged_df
    for idx, row in sim_merged_df.iterrows():
        scan_index = row['scan_index']
        # Extract corresponding row in veh_rdd_df
        rdd_row = sim_rdd_df[sim_rdd_df['scan_index'] == scan_index]
        # If a matching row is found in veh_rdd_df
        if not rdd_row.empty:
            # Iterate over each rdd_idx column in sim_merged_df
            for i in range(len([col for col in sim_merged_df.columns if col.startswith('rdd_idx')])):
                rdd_idx = row[f'rdd_idx_{i}']
                # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
                sim_merged_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                sim_merged_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]
    # Display the updated sim_merged_df
    #logger.custom_print(sim_merged_df)
    #################################
    
    #################################
    # Get the final data for KPI
    #################################    
    final_df = pd.merge(veh_merged_df, sim_merged_df, on='scan_index', how='inner', suffixes=('_veh', '_sim'))
    
    base_columns = ['scan_index', 'num_af_det_veh', 'num_af_det_sim']
    if log_vars.cdc_data_available:
        base_columns += ['cdc_saturated']
    repeated_columns = ['rdd_idx', 'rdd1_rindx', 'rdd1_dindx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 'f_superres_target', 'f_bistatic']
    selected_columns_real = [f'{col}_{i}_veh' for col in repeated_columns for i in range(config.MAX_NUM_OF_AF_DETS)]
    selected_columns_sim = [f'{col}_{i}_sim' for col in repeated_columns for i in range(config.MAX_NUM_OF_AF_DETS)]
    selected_columns = base_columns + selected_columns_real + selected_columns_sim
    final_df = final_df[selected_columns]
    # The below should be assigned after all filtering 
    log_vars.num_of_SI_in_veh_and_sim_merged = final_df.shape[0]
    #################################
        
        
    logger.custom_print("[INFO] AF Detection Matching...")
    #################################
    # DET Stream matching
    #################################
    rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    ran_cols_veh = [f"ran_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    vel_cols_veh = [f"vel_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    theta_cols_veh = [f"theta_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    phi_cols_veh = [f"phi_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    single_cols_veh = [f"f_single_target_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    superres_cols_veh = [f"f_superres_target_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    bistatic_cols_veh = [f"f_bistatic_{i}_veh" for i in range(config.MAX_NUM_OF_AF_DETS)]
    rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    ran_cols_sim = [f"ran_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    vel_cols_sim = [f"vel_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    theta_cols_sim = [f"theta_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    phi_cols_sim = [f"phi_{i}_sim" for i in range(config.MAX_NUM_OF_AF_DETS)]
    log_vars.scan_index_list = final_df['scan_index'].tolist()
    log_vars.num_af_det_veh_list = final_df['num_af_det_veh'].tolist()
    log_vars.num_af_det_sim_list = final_df['num_af_det_sim'].tolist()
    def find_max_range(row):
        max_range_veh = 0
        max_range_sim = 0
        range_saturated = False
        max_range_veh = max(row[ran_cols_veh])
        max_range_sim = max(row[ran_cols_sim])
        if max_range_veh >= config.RANGE_SATURATION_THRESHOLD and max_range_sim < config.RANGE_SATURATION_THRESHOLD:
            range_saturated = True
        return max_range_veh, max_range_sim, range_saturated
    final_df[['max_range_veh', 'max_range_sim', 'range_saturated']] = final_df.apply(find_max_range, axis=1, result_type="expand")
    log_vars.max_range_veh_list = final_df['max_range_veh'].tolist()
    log_vars.max_range_sim_list = final_df['max_range_sim'].tolist()
    log_vars.saturated_list = final_df['range_saturated'].tolist()
    if len(log_vars.saturated_list) != 0:
        log_vars.perc_of_scans_with_range_saturation = round((sum(log_vars.saturated_list) / len(log_vars.saturated_list)) * 100, 2)
    else:
        log_vars.perc_of_scans_with_range_saturation = 0
    def count_det_params_matches(row):
        """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
        num_detect_veh = int(row['num_af_det_veh'])
        num_detect_sim = int(row['num_af_det_sim'])
        veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
        sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
        sim_data = dict(zip(sim_pairs, zip(row[ran_cols_sim[:num_detect_sim]],
                                           row[vel_cols_sim[:num_detect_sim]],
                                           row[theta_cols_sim[:num_detect_sim]],
                                           row[phi_cols_sim[:num_detect_sim]])))
        subset_match_count = 0
        all_match_count = 0
        matched_sim_indices = set()
        for idx, veh_pair in enumerate(veh_pairs):
            for jdx, sim_pair in enumerate(sim_pairs):
                if jdx in matched_sim_indices:
                    continue
                if veh_pair == sim_pair:
                    sim_ran = row[ran_cols_sim[jdx]]
                    sim_vel = row[vel_cols_sim[jdx]]
                    sim_theta = row[theta_cols_sim[jdx]]
                    sim_phi = row[phi_cols_sim[jdx]]
                    veh_ran = row[ran_cols_veh[idx]]
                    veh_vel = row[vel_cols_veh[idx]]
                    veh_theta = row[theta_cols_veh[idx]]
                    veh_phi = row[phi_cols_veh[idx]]
                    veh_single_target = row[single_cols_veh[idx]]
                    veh_superres_target = row[superres_cols_veh[idx]]
                    veh_bistatic = row[bistatic_cols_veh[idx]]
                    ran_diff = veh_ran - sim_ran
                    ran_diff_abs = abs(ran_diff)
                    vel_diff = veh_vel - sim_vel
                    vel_diff_abs = abs(vel_diff)
                    theta_diff = veh_theta - sim_theta
                    theta_diff_abs = abs(theta_diff)
                    phi_diff = veh_phi - sim_phi
                    phi_diff_abs = abs(phi_diff)
                    if(ran_diff_abs <= config.RAN_THRESHOLD and vel_diff_abs <= config.VEL_THRESHOLD):
                        subset_match_count += 1
                        if(theta_diff_abs <= config.THETA_THRESHOLD and phi_diff_abs <= config.PHI_THRESHOLD):
                            all_match_count += 1
                            matched_sim_indices.add(jdx)
                            break
                    if (ran_diff_abs > config.RAN_THRESHOLD):
                        log_vars.ran_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, ran_diff))
                    if (vel_diff_abs > config.VEL_THRESHOLD):
                        log_vars.vel_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, vel_diff))
                    if (theta_diff_abs > config.THETA_THRESHOLD):
                        log_vars.theta_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, theta_diff))
                    if (phi_diff_abs > config.PHI_THRESHOLD):
                        log_vars.phi_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, phi_diff))
        return subset_match_count, all_match_count
    final_df[['det_subset_params_match_count', 'det_all_params_match_count']] = final_df.apply(count_det_params_matches, axis=1, result_type="expand")
    final_df['same_num_of_AF_detections'] = final_df['num_af_det_veh'] == final_df['num_af_det_sim']
    final_df['matching_pct_det_all_params'] = final_df['det_all_params_match_count']/final_df['num_af_det_veh']
    final_df['matching_pct_det_subset_params'] = final_df['det_subset_params_match_count']/final_df['num_af_det_veh']
    log_vars.accuracy_list = final_df['matching_pct_det_all_params'].tolist()
    num_of_af_dets_in_veh = sum(final_df['num_af_det_veh'])
    num_of_af_dets_in_sim = sum(final_df['num_af_det_sim'])
    num_of_dets_with_matching_rv = sum(final_df['det_subset_params_match_count'])
    num_of_dets_with_matching_rvtp = sum(final_df['det_all_params_match_count'])
    num_of_SI_with_matching_rv = final_df[(final_df['matching_pct_det_subset_params'] >= 0.99)].shape[0]
    num_of_SI_with_matching_rvtp = final_df[(final_df['matching_pct_det_all_params'] >= 0.99)].shape[0]
    if num_of_af_dets_in_veh != 0:
        log_vars.overall_accuracy = round((num_of_dets_with_matching_rvtp / num_of_af_dets_in_veh) * 100, 2)
        log_vars.min_accuracy = round(min(log_vars.accuracy_list)*100, 2)
    else:
        log_vars.overall_accuracy = 0
        log_vars.min_accuracy = 0
        
    if log_vars.cdc_data_available:
        num_of_af_dets_in_veh_without_cdc_saturation = final_df.loc[~final_df['cdc_saturated'], 'num_af_det_veh'].sum()
        if num_of_af_dets_in_veh_without_cdc_saturation != 0:
            num_of_dets_with_matching_rvtp_without_cdc_saturation = final_df.loc[~final_df['cdc_saturated'], 'det_all_params_match_count'].sum()
            log_vars.overall_accuracy_excluding_cdc_saturation = round((num_of_dets_with_matching_rvtp_without_cdc_saturation / num_of_af_dets_in_veh_without_cdc_saturation) * 100, 2)
        else:
            log_vars.overall_accuracy_excluding_cdc_saturation = 0
    kpis_final = {'result1':
                    {'numerator': num_of_SI_with_matching_rv,
                     'denominator': log_vars.num_of_SI_in_veh_and_sim_merged,
                     'value': round((num_of_SI_with_matching_rv / log_vars.num_of_SI_in_veh_and_sim_merged) * 100, 2) if (log_vars.num_of_SI_in_veh_and_sim_merged != 0) else None},
                'result2':
                    {'numerator': num_of_SI_with_matching_rvtp,
                     'denominator': log_vars.num_of_SI_in_veh_and_sim_merged,
                     'value': round((num_of_SI_with_matching_rvtp / log_vars.num_of_SI_in_veh_and_sim_merged) * 100, 2) if (log_vars.num_of_SI_in_veh_and_sim_merged != 0) else None},
                'result3':
                    {'numerator': num_of_dets_with_matching_rv,
                     'denominator': num_of_af_dets_in_veh,
                     'value': round((num_of_dets_with_matching_rv / num_of_af_dets_in_veh) * 100, 2) if (num_of_af_dets_in_veh != 0) else None},
                'result4':
                    {'numerator': num_of_dets_with_matching_rvtp,
                     'denominator': num_of_af_dets_in_veh,
                     'value': round((num_of_dets_with_matching_rvtp / num_of_af_dets_in_veh) * 100, 2) if (num_of_af_dets_in_veh != 0) else None},
                }
  