import sys
import os
import numpy as np
import pandas as pd
import plotly.graph_objs as go
import plotly.subplots as sp
import plotly.io as pio
from collections import Counter
pd.options.display.width = 0

def main(veh_csv, sim_csv, veh_rdd_csv, sim_rdd_csv):
    #################################
    # Global variables
    #################################
    global num_of_SI_in_veh_af, num_of_SI_in_sim_af, num_of_same_SI_in_veh_and_sim_af
    global ran_threshold, vel_threshold, theta_threshold, phi_threshold
    global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list, snr_diff_list, rcs_diff_list
    global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list
    global max_number_of_data, max_number_of_rdd_data
    global max_num_of_si_to_process
    global html_content
    #################################

    print("Reading the CSVs...")
    #################################
    # Read Vehicle and Resim data
    # Not all scans available in vehicle DET csv will be available in sim DET csv
    # Not all scans available in vehicle DET csv will be available in vehicle RDD csv
    # Not all scans available in sim DET csv will be available in sim RDD csv
    #################################
    if (max_num_of_si_to_process != 0):
        nrows = max_num_of_si_to_process
    else:
        nrows = None
    det_cols_of_interest = ['scan_index', 'num_af_det']
    det_cols_of_interest = det_cols_of_interest + [item for i in range(max_number_of_data) for item in (f"rdd_idx_{i}", f"ran_{i}", f"vel_{i}", f"theta_{i}", f"phi_{i}", f"f_single_target_{i}", f"f_superres_target_{i}", f"f_bistatic_{i}")]
    veh_det_df = pd.read_csv(veh_csv, usecols = det_cols_of_interest, nrows=nrows, memory_map=True)
    sim_det_df = pd.read_csv(sim_csv, usecols = det_cols_of_interest, nrows=nrows, memory_map=True)
    #################################
    # keep only those scans which are available in both DET and RDD csvs
    # #################################
    sample_veh_rdd_df = pd.read_csv(veh_rdd_csv, usecols = ['scan_index'], nrows=nrows, memory_map=True)
    sample_sim_rdd_df = pd.read_csv(sim_rdd_csv, usecols = ['scan_index'], nrows=nrows, memory_map=True)
    veh_det_df = pd.merge(veh_det_df, sample_veh_rdd_df, on='scan_index', suffixes=('', '_rdd'))
    sim_det_df = pd.merge(sim_det_df, sample_sim_rdd_df, on='scan_index', suffixes=('', '_rdd'))
    #################################
    veh_det_df = veh_det_df[veh_det_df['num_af_det'] != 0]
    sim_det_df = sim_det_df[sim_det_df['num_af_det'] != 0]
    num_of_SI_in_veh_af = veh_det_df.shape[0]
    num_of_SI_in_sim_af = sim_det_df.shape[0]
    rdd_cols_of_interest = ['scan_index', 'rdd1_num_detect']
    rdd_cols_of_interest = rdd_cols_of_interest + [item for i in range(max_number_of_rdd_data) for item in (f"rdd1_rindx_{i}", f"rdd1_dindx_{i}", f"rdd2_range_{i}", f"rdd2_range_rate_{i}")]
    veh_rdd_df = pd.read_csv(veh_rdd_csv, usecols = rdd_cols_of_interest, nrows=nrows, memory_map=True)
    sim_rdd_df = pd.read_csv(sim_rdd_csv, usecols = rdd_cols_of_interest, nrows=nrows, memory_map=True)
    veh_rdd_df = veh_rdd_df[veh_rdd_df['rdd1_num_detect'] != 0]
    sim_rdd_df = sim_rdd_df[sim_rdd_df['rdd1_num_detect'] != 0]
    num_of_SI_in_veh_rdd = veh_rdd_df.shape[0]
    num_of_SI_in_sim_rdd = sim_rdd_df.shape[0]
    #################################

    #################################
    # RDD Stream matching
    #################################
    # Set constants
    SCALE_P21_TO_FLOAT = (4.768371582e-07)

    # Step 1: Merge the dataframes on 'scan_index'
    merged_df = pd.merge(veh_rdd_df, sim_rdd_df, on='scan_index', suffixes=('_veh', '_sim'))
    num_of_same_SI_in_veh_and_sim_rdd = merged_df.shape[0]
    num_of_SI_with_same_num_of_rdd1_dets = merged_df[merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']].shape[0]

    # Generate column names
    rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(max_number_of_rdd_data)]
    dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(max_number_of_rdd_data)]
    range_cols_veh = [f"rdd2_range_{i}_veh" for i in range(max_number_of_rdd_data)]
    range_rate_cols_veh = [f"rdd2_range_rate_{i}_veh" for i in range(max_number_of_rdd_data)]
    rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(max_number_of_rdd_data)]
    dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(max_number_of_rdd_data)]
    range_cols_sim = [f"rdd2_range_{i}_sim" for i in range(max_number_of_rdd_data)]
    range_rate_cols_sim = [f"rdd2_range_rate_{i}_sim" for i in range(max_number_of_rdd_data)]

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

    merged_df['matched_rindx_dindx_pairs'] = merged_df.apply(count_rindx_dindx_matches, axis=1)

    # Step 3: Compute matches within thresholds for rdd2_range and rdd2_range_rate
    def count_range_matches(row):
        """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
        num_detect_veh = int(row['rdd1_num_detect_veh'])
        num_detect_sim = int(row['rdd1_num_detect_sim'])
        #if(num_detect_veh < num_detect_sim):
        #    print(num_detect_veh, num_detect_sim)

        veh_pairs = list(zip(row[rindx_cols_veh[:num_detect_veh]], row[dindx_cols_veh[:num_detect_veh]]))
        sim_pairs = list(zip(row[rindx_cols_sim[:num_detect_sim]], row[dindx_cols_sim[:num_detect_sim]]))
        sim_data = dict(zip(sim_pairs, zip(row[range_cols_sim[:num_detect_sim]], row[range_rate_cols_sim[:num_detect_sim]])))

        match_count = 0
        for idx, veh_pair in enumerate(veh_pairs):
            if veh_pair in sim_data:
                sim_range, sim_range_rate = sim_data[veh_pair]
                sim_range = round(sim_range * SCALE_P21_TO_FLOAT, 3)
                sim_range_rate = round(sim_range_rate * SCALE_P21_TO_FLOAT, 3)
                veh_range = row[range_cols_veh[idx]]
                veh_range_rate = row[range_rate_cols_veh[idx]]
                veh_range = round(veh_range * SCALE_P21_TO_FLOAT, 3)
                veh_range_rate = round(veh_range_rate * SCALE_P21_TO_FLOAT, 3)
                if(veh_pair == (0, 0)):
                    print(f"Idx:{idx}")
                    print(veh_pair)
                    print(veh_range, veh_range_rate)
                    print(sim_range, sim_range_rate)
                if (
                        abs(sim_range - veh_range) <= ran_threshold
                        and abs(sim_range_rate - veh_range_rate) <= vel_threshold
                ):
                    match_count += 1
        return match_count

    merged_df['range_rangerate_matches'] = merged_df.apply(count_range_matches, axis=1)

    merged_df['same_num_of_RDD1_detections'] = merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']
    merged_df['matching_pct_rindx_dindx_pairs'] = merged_df['matched_rindx_dindx_pairs']/merged_df['rdd1_num_detect_veh']
    merged_df['matching_pct_range_rangerate_pairs'] = merged_df['range_rangerate_matches'] / merged_df['matched_rindx_dindx_pairs']

    # Step 5: Report results
    #report = merged_df[['scan_index', 'rdd1_num_detect_veh', 'rdd1_num_detect_sim', 'matched_rindx_dindx_pairs', 'range_rangerate_matches']]
    #print(report)

    num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair = merged_df[(merged_df['same_num_of_RDD1_detections'] == 1) & (merged_df['matching_pct_rindx_dindx_pairs'] == 1)].shape[0]
    num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair = merged_df[(merged_df['same_num_of_RDD1_detections'] == 1) & (merged_df['matching_pct_rindx_dindx_pairs'] == 1) & (merged_df['matching_pct_range_rangerate_pairs'] == 1)].shape[0]

    kpis_rdd = {'result1':
                {'numerator': num_of_SI_with_same_num_of_rdd1_dets,
                 'denominator': num_of_same_SI_in_veh_and_sim_rdd,
                 'value': round((num_of_SI_with_same_num_of_rdd1_dets/num_of_same_SI_in_veh_and_sim_rdd)*100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None},
            'result2':
                {'numerator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
                 'denominator': num_of_SI_with_same_num_of_rdd1_dets,
                 'value': round((num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair / num_of_SI_with_same_num_of_rdd1_dets) * 100, 2) if (num_of_SI_with_same_num_of_rdd1_dets != 0) else None},
            'result3':
                {'numerator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
                 'denominator': num_of_same_SI_in_veh_and_sim_rdd,
                 'value': round((num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair / num_of_same_SI_in_veh_and_sim_rdd) * 100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None},
            'result4':
                {'numerator': num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair,
                 'denominator': num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair,
                 'value': round((num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair / num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair) * 100, 2) if (num_of_SI_with_100_pct_matching_rdd1_rindx_dindx_pair != 0) else None},
            'result5':
                {'numerator': num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair,
                 'denominator': num_of_same_SI_in_veh_and_sim_rdd,
                 'value': round((num_of_SI_with_100_pct_matching_rdd1_rng_dop_pair / num_of_same_SI_in_veh_and_sim_rdd) * 100, 2) if (num_of_same_SI_in_veh_and_sim_rdd != 0) else None },
            }


    print(f"Number of SI in (vehicle, simulation): ({num_of_SI_in_veh_rdd}, {num_of_SI_in_sim_rdd})")
    print(f"Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_rdd}")
    print(f"% of SI with same number of RDD1 detections: "
          f"{kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']} --> {kpis_rdd['result1']['value']}%" )
    print(f"% of SI with 100% matching (rindx, dindx) pair: "
          f"{kpis_rdd['result2']['numerator']}/{kpis_rdd['result2']['denominator']} --> {kpis_rdd['result2']['value']}%, "
          f"{kpis_rdd['result3']['numerator']}/{kpis_rdd['result3']['denominator']} --> {kpis_rdd['result3']['value']}%")
    print(f"% of SI with 100% matching (range, rangerate) pair: "
          f"{kpis_rdd['result4']['numerator']}/{kpis_rdd['result4']['denominator']} --> {kpis_rdd['result4']['value']}%, "
          f"{kpis_rdd['result5']['numerator']}/{kpis_rdd['result5']['denominator']} --> {kpis_rdd['result5']['value']}%")

    #################################
    # AF Stream matching
    #################################
    print("Extracting rdd1_rindx and rdd1_dindx values from veh_rdd_df...")
    #################################
    # Extract and append rdd1_rindx and rdd1_dindx values from veh_rdd_df to veh_det_df
    # based on rdd_idx columns of veh_det_df
    #################################
    # Generate a dictionary for the new columns 'rdd1_rindx' and 'rdd1_dindx' with `None` values
    new_data = {f"rdd1_rindx_{i}": None for i in range(max_number_of_data)}
    new_data.update({f"rdd1_dindx_{i}": None for i in range(max_number_of_data)})
    # Create a new DataFrame with these columns
    new_columns_df = pd.DataFrame(new_data, index=veh_det_df.index)
    # Concatenate the new columns to the existing DataFrame
    veh_det_df = pd.concat([veh_det_df, new_columns_df], axis=1)

    # Iterate through each row of veh_det_df
    for idx, row in veh_det_df.iterrows():
        scan_index = row['scan_index']

        # Extract corresponding row in veh_rdd_df
        rdd_row = veh_rdd_df[veh_rdd_df['scan_index'] == scan_index]

        # If a matching row is found in veh_rdd_df
        if not rdd_row.empty:
            # Iterate over each rdd_idx column in veh_det_df
            for i in range(len([col for col in veh_det_df.columns if col.startswith('rdd_idx')])):
                rdd_idx = row[f'rdd_idx_{i}']

                # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
                veh_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                veh_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]

    # Display the updated veh_det_df
    #print(veh_det_df)
    #################################

    print("Extracting rdd1_rindx and rdd1_dindx values from sim_rdd_df...")
    #################################
    # Extract and append rdd1_rindx and rdd1_dindx values from sim_rdd_df to sim_det_df
    # based on rdd_idx columns of sim_det_df
    #################################
    new_columns_df = pd.DataFrame(new_data, index=sim_det_df.index)
    # Concatenate the new columns to the existing DataFrame
    sim_det_df = pd.concat([sim_det_df, new_columns_df], axis=1)

    # Iterate through each row of veh_det_df
    for idx, row in sim_det_df.iterrows():
        scan_index = row['scan_index']

        # Extract corresponding row in veh_rdd_df
        rdd_row = sim_rdd_df[sim_rdd_df['scan_index'] == scan_index]

        # If a matching row is found in veh_rdd_df
        if not rdd_row.empty:
            # Iterate over each rdd_idx column in veh_det_df
            for i in range(len([col for col in sim_det_df.columns if col.startswith('rdd_idx')])):
                rdd_idx = row[f'rdd_idx_{i}']

                # Assign the values from rdd1_rindx and rdd1_dindx based on rdd_idx
                sim_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                sim_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]

    # Display the updated veh_det_df
    #print(sim_det_df)
    #################################

    print("Merging the dataframes...")
    #################################
    # Merge Vehicle and Resim data
    #################################
    result_df = pd.merge(veh_det_df, sim_det_df, on='scan_index', how='inner', suffixes=('_veh', '_sim'))
    if (max_num_of_si_to_process != 0):
        result_df = result_df.iloc[:max_num_of_si_to_process]
    num_of_same_SI_in_veh_and_sim_af = result_df.shape[0]
    num_of_SI_with_same_num_of_dets_af = result_df[result_df['num_af_det_veh'] <= result_df['num_af_det_sim']].shape[0]
    base_columns = ['scan_index', 'num_af_det_veh', 'num_af_det_sim']
    repeated_columns = ['rdd_idx', 'rdd1_rindx', 'rdd1_dindx', 'ran', 'vel', 'theta', 'phi', 'f_single_target', 'f_superres_target', 'f_bistatic']
    selected_columns_real = [f'{col}_{i}_veh' for col in repeated_columns for i in range(max_number_of_data)]
    selected_columns_sim = [f'{col}_{i}_sim' for col in repeated_columns for i in range(max_number_of_data)]
    selected_columns = base_columns + selected_columns_real + selected_columns_sim
    final_df = result_df[selected_columns]
    #final_df['scan_index'] = final_df['scan_index'].astype(int)
    # print(final_df.head())
    #################################

    print("Matching...")
    rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(max_number_of_data)]
    dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(max_number_of_data)]
    ran_cols_veh = [f"ran_{i}_veh" for i in range(max_number_of_data)]
    vel_cols_veh = [f"vel_{i}_veh" for i in range(max_number_of_data)]
    theta_cols_veh = [f"theta_{i}_veh" for i in range(max_number_of_data)]
    phi_cols_veh = [f"phi_{i}_veh" for i in range(max_number_of_data)]
    single_cols_veh = [f"f_single_target_{i}_veh" for i in range(max_number_of_data)]
    superres_cols_veh = [f"f_superres_target_{i}_veh" for i in range(max_number_of_data)]
    bistatic_cols_veh = [f"f_bistatic_{i}_veh" for i in range(max_number_of_data)]
    rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(max_number_of_data)]
    dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(max_number_of_data)]
    ran_cols_sim = [f"ran_{i}_sim" for i in range(max_number_of_data)]
    vel_cols_sim = [f"vel_{i}_sim" for i in range(max_number_of_data)]
    theta_cols_sim = [f"theta_{i}_sim" for i in range(max_number_of_data)]
    phi_cols_sim = [f"phi_{i}_sim" for i in range(max_number_of_data)]

    scan_index_list = final_df['scan_index'].tolist()
    num_af_det_veh_list = final_df['num_af_det_veh'].tolist()
    num_af_det_sim_list = final_df['num_af_det_sim'].tolist()

    def find_max_range(row):
        max_range_veh = 0
        max_range_sim = 0

        max_range_veh = max(row[ran_cols_veh])
        max_range_sim = max(row[ran_cols_sim])

        return max_range_veh, max_range_sim

    final_df[['max_range_veh', 'max_range_sim']] = final_df.apply(find_max_range, axis=1, result_type="expand")
    max_range_veh_list = final_df['max_range_veh'].tolist()
    max_range_sim_list = final_df['max_range_sim'].tolist()

    def count_det_params_matches(row):
        """Count matches for rdd2_range and rdd2_range_rate within thresholds."""
        num_detect_veh = int(row['num_af_det_veh'])
        num_detect_sim = int(row['num_af_det_sim'])
        #if(num_detect_veh < num_detect_sim):
        #    print(num_detect_veh, num_detect_sim)

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

                    if(ran_diff_abs <= ran_threshold and vel_diff_abs <= vel_threshold):
                        subset_match_count += 1
                        if(theta_diff_abs <= theta_threshold and phi_diff_abs <= phi_threshold):
                            all_match_count += 1
                            matched_sim_indices.add(jdx)
                            break

                    if (ran_diff_abs > ran_threshold):
                        ran_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, ran_diff))
                    if (vel_diff_abs > vel_threshold):
                        vel_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, vel_diff))
                    if (theta_diff_abs > theta_threshold):
                        theta_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, theta_diff))
                    if (phi_diff_abs > phi_threshold):
                        phi_diff_list.append((veh_ran, veh_vel, veh_theta, veh_phi, veh_single_target, veh_superres_target, veh_bistatic, phi_diff))

        return subset_match_count, all_match_count

    final_df[['det_subset_params_match_count', 'det_all_params_match_count']] = final_df.apply(count_det_params_matches, axis=1, result_type="expand")

    final_df['same_num_of_AF_detections'] = final_df['num_af_det_veh'] == final_df['num_af_det_sim']
    final_df['matching_pct_det_all_params'] = final_df['det_all_params_match_count']/final_df['num_af_det_veh']
    final_df['matching_pct_det_subset_params'] = final_df['det_subset_params_match_count']/final_df['num_af_det_veh']
    accuracy_list = final_df['matching_pct_det_all_params'].tolist()

    #report = final_df[['scan_index', 'num_af_det_veh', 'num_af_det_sim', 'det_subset_params_match_count', 'det_all_params_match_count']]
    #print(report)

    num_of_dets_in_veh_af = sum(final_df['num_af_det_veh'])
    num_of_dets_in_sim_af = sum(final_df['num_af_det_sim'])
    num_of_dets_with_matching_rv_af = sum(final_df['det_subset_params_match_count'])
    num_of_dets_with_matching_rvtp_af = sum(final_df['det_all_params_match_count'])
    num_of_SI_with_matching_rv_af = final_df[(final_df['same_num_of_AF_detections'] == 1) & (final_df['matching_pct_det_subset_params'] == 1)].shape[0]
    num_of_SI_with_matching_rvtp_af = final_df[(final_df['same_num_of_AF_detections'] == 1) & (final_df['matching_pct_det_all_params'] == 1)].shape[0]

    kpis_af = {'result1':
                    {'numerator': num_of_SI_with_same_num_of_dets_af,
                     'denominator': num_of_same_SI_in_veh_and_sim_af,
                     'value': round((num_of_SI_with_same_num_of_dets_af / num_of_same_SI_in_veh_and_sim_af) * 100,2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
                'result2':
                    {'numerator': num_of_SI_with_matching_rv_af,
                     'denominator': num_of_SI_with_same_num_of_dets_af,
                     'value': round((num_of_SI_with_matching_rv_af / num_of_SI_with_same_num_of_dets_af) * 100, 2) if (num_of_SI_with_same_num_of_dets_af != 0) else None},
                'result3':
                    {'numerator': num_of_SI_with_matching_rv_af,
                     'denominator': num_of_same_SI_in_veh_and_sim_af,
                     'value': round((num_of_SI_with_matching_rv_af / num_of_same_SI_in_veh_and_sim_af) * 100, 2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
                'result4':
                    {'numerator': num_of_SI_with_matching_rvtp_af,
                     'denominator': num_of_SI_with_same_num_of_dets_af,
                     'value': round(( num_of_SI_with_matching_rvtp_af / num_of_SI_with_same_num_of_dets_af) * 100, 2) if (num_of_SI_with_same_num_of_dets_af != 0) else None},
                'result5':
                    {'numerator': num_of_SI_with_matching_rvtp_af,
                     'denominator': num_of_same_SI_in_veh_and_sim_af,
                     'value': round((num_of_SI_with_matching_rvtp_af / num_of_same_SI_in_veh_and_sim_af) * 100, 2) if (num_of_same_SI_in_veh_and_sim_af != 0) else None},
                'result6':
                    {'numerator': num_of_dets_with_matching_rv_af,
                     'denominator': num_of_dets_in_veh_af,
                     'value': round((num_of_dets_with_matching_rv_af / num_of_dets_in_veh_af) * 100, 2) if (num_of_dets_in_veh_af != 0) else None},
                'result7':
                    {'numerator': num_of_dets_with_matching_rvtp_af,
                     'denominator': num_of_dets_in_veh_af,
                     'value': round((num_of_dets_with_matching_rvtp_af / num_of_dets_in_veh_af) * 100, 2) if (num_of_dets_in_veh_af != 0) else None},
                }

    print(f"Number of SI in (vehicle, simulation): ({num_of_SI_in_veh_af}, {num_of_SI_in_sim_af})")
    print(f"Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_af}")
    print(f"% of SI with same number of AF detections: "
          f"{kpis_af['result1']['numerator']}/{kpis_af['result1']['denominator']} --> {kpis_af['result1']['value']}%" )
    print(f"% of SI with 100% matching det params(r,v): "
          f"{kpis_af['result2']['numerator']}/{kpis_af['result2']['denominator']} --> {kpis_af['result2']['value']}%, "
          f"{kpis_af['result3']['numerator']}/{kpis_af['result3']['denominator']} --> {kpis_af['result3']['value']}%")
    print(f"% of SI with 100% matching det params(r,v,t,p): "
          f"{kpis_af['result4']['numerator']}/{kpis_af['result4']['denominator']} --> {kpis_af['result4']['value']}%, "
          f"{kpis_af['result5']['numerator']}/{kpis_af['result5']['denominator']} --> {kpis_af['result5']['value']}%")
    print(f"Number of detections in (vehicle, simulation): ({num_of_dets_in_veh_af}, {num_of_dets_in_sim_af})")
    print(f"Accuracy(r,v): "
          f"{kpis_af['result6']['numerator']}/{kpis_af['result6']['denominator']} --> {kpis_af['result6']['value']}%")
    print(f"Accuracy(r,v,t,p): "
          f"{kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']} --> {kpis_af['result7']['value']}%")

    #################################
    # HTML Content
    #################################
    html_content += f"""
    <b>KPI:</b> Accuracy: ({kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']}) --> <b>{kpis_af['result7']['value']}%</b>
    <details>
        <summary><i>Details</i></summary>
        <b>RDD Streams</b><br>
        Number of SI in (vehicle, simulation) : {num_of_SI_in_veh_rdd}, {num_of_SI_in_sim_rdd}<br>
        Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_rdd}<br>
        % of SI with same number of RDD1 detections: 
        ({kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']}) --> {kpis_rdd['result1']['value']}%<br>
        % of SI with 100% matching (rindx, dindx) pair:
        ({kpis_rdd['result2']['numerator']}/{kpis_rdd['result2']['denominator']}) --> {kpis_rdd['result2']['value']}%,
        ({kpis_rdd['result3']['numerator']}/{kpis_rdd['result3']['denominator']}) --> {kpis_rdd['result3']['value']}%,
        <br>
        % of SI with 100% matching (range, rangerate) pair:
        ({kpis_rdd['result4']['numerator']}/{kpis_rdd['result4']['denominator']}) --> {kpis_rdd['result4']['value']}%,
        ({kpis_rdd['result5']['numerator']}/{kpis_rdd['result5']['denominator']}) --> {kpis_rdd['result5']['value']}%,
        <br>
        <b>Detection Streams</b><br>
        Number of SI in (vehicle, simulation) : {num_of_SI_in_veh_af}, {num_of_SI_in_sim_af}<br>
        Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_af}<br>
        % of SI with same number of AF detections: 
        ({kpis_af['result1']['numerator']}/{kpis_af['result1']['denominator']}) --> {kpis_af['result1']['value']}%
        <br>
        % of SI with 100% matching det params(r,v):
        ({kpis_af['result2']['numerator']}/{kpis_af['result2']['denominator']}) --> {kpis_af['result2']['value']}%,
        ({kpis_af['result3']['numerator']}/{kpis_af['result3']['denominator']}) --> {kpis_af['result3']['value']}%,
        <br>
        % of SI with 100% matching det params(r,v,t,p):
        ({kpis_af['result4']['numerator']}/{kpis_af['result4']['denominator']}) --> {kpis_af['result4']['value']}%,
        ({kpis_af['result5']['numerator']}/{kpis_af['result5']['denominator']}) --> {kpis_af['result5']['value']}%,
        <br>
        Number of detections in (vehicle, simulation): {num_of_dets_in_veh_af}, {num_of_dets_in_sim_af}<br>
        Accuracy(r,v):
        ({kpis_af['result6']['numerator']}/{kpis_af['result6']['denominator']}) --> {kpis_af['result6']['value']}%,
        Accuracy(r,v,t,p):
        ({kpis_af['result7']['numerator']}/{kpis_af['result7']['denominator']}) --> {kpis_af['result7']['value']}%,
        <br>
    """
    #################################

def func_bar(diffs):
    # Count the frequency of each unique value
    counts = Counter(diffs)
    #print(len(counts))
    #print(counts)
    # Total number of observations
    total_observations = len(diffs)
    # Calculate percentages
    percentages = {key: (value / total_observations) * 100 for key, value in counts.items()}
    sorted_dict = dict(sorted(percentages.items()))
    #print(list(percentages.keys()))
    # Return Bar trace
    return go.Bar(x=list(sorted_dict.keys()), y=list(sorted_dict.values()))

def func_scatter(x, y):
    # Return Scatter trace
    return go.Scatter(x=x, y=y, mode='markers')

def func_line(x, y):
    # Return Scatter trace
    return go.Scatter(x=x, y=y, mode='lines')

def plot_stats():
    #################################
    # Global variables
    #################################
    global html_content
    global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list
    global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list
    #################################

    rng_idx = 0
    vel_idx = 1
    theta_idx = 2
    phi_idx = 3
    single_target_idx = 4
    superres_target_idx = 5
    bistatic_idx = 6
    error_idx = 7
    ran_diffs = np.array([ele[error_idx] for ele in ran_diff_list]).round(3)
    vel_diffs = np.array([ele[error_idx] for ele in vel_diff_list]).round(3)
    theta_diffs = np.array([ele[error_idx] for ele in theta_diff_list]).round(3)
    phi_diffs = np.array([ele[error_idx] for ele in phi_diff_list]).round(3)

    #################################
    # Create subplots for error bar plots:
    #################################
    fig_bar = sp.make_subplots(rows=2, cols=2, horizontal_spacing=0.04, vertical_spacing=0.2)

    # Manually add traces for bar plots
    fig_bar.add_trace(func_bar(ran_diffs), row=1, col=1)
    fig_bar.update_yaxes(title_text="Percentage(%)", row=1, col=1)
    fig_bar.update_xaxes(title_text="Range error", row=1, col=1)
    fig_bar.add_trace(func_bar(vel_diffs), row=1, col=2)
    fig_bar.update_xaxes(title_text="Range Rate error", row=1, col=2)
    fig_bar.add_trace(func_bar(theta_diffs), row=2, col=1)
    fig_bar.update_yaxes(title_text="Percentage(%)", row=2, col=1)
    fig_bar.update_xaxes(title_text="Azimuth error", row=2, col=1)
    fig_bar.add_trace(func_bar(phi_diffs), row=2, col=2)
    fig_bar.update_xaxes(title_text="Elevation error", row=2, col=2)

    # Update layout for bar plot
    fig_bar.update_layout(height=700, width=1250, title_text="% of unmatched detections v/s error", showlegend=False)
    fig_bar.update_traces(marker_color='red')
    fig_bar.update_xaxes(zeroline=False, showgrid=False, type='category')
    fig_bar.update_yaxes(zeroline=False, showgrid=False)
    fig_bar.show()

    bar_plot_html = pio.to_html(fig_bar, full_html=False, include_plotlyjs='cdn')
    #################################

    #################################
    # Create subplots for error v/s different detection properties plots:
    #################################
    fig_scatter = sp.make_subplots(rows=4, cols=7, horizontal_spacing=0.03, vertical_spacing=0.04)

    # Manually add traces for scatter plots
    # Range error against detection properties
    row_num = 1
    rang = [ele[rng_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(rang, ran_diffs), row=row_num, col=1)
    fig_scatter.update_yaxes(title_text="Range error", row=row_num, col=1)
    fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
    vel = [ele[vel_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(vel, ran_diffs), row=row_num, col=2)
    fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
    theta = [ele[theta_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(theta, ran_diffs), row=row_num, col=3)
    fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
    phi = [ele[phi_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(phi, ran_diffs), row=row_num, col=4)
    fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
    single_target = [ele[single_target_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(single_target, ran_diffs), row=row_num, col=5)
    fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
    superres_target = [ele[superres_target_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(superres_target, ran_diffs), row=row_num, col=6)
    fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
    bistatic = [ele[bistatic_idx] for ele in ran_diff_list]
    fig_scatter.add_trace(func_scatter(bistatic, ran_diffs), row=row_num, col=7)
    fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)

    # Range rate error against detection properties
    row_num = 2
    rang = [ele[rng_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(rang, vel_diffs), row=row_num, col=1)
    fig_scatter.update_yaxes(title_text="Range Rate error", row=row_num, col=1)
    fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
    vel = [ele[vel_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(vel, vel_diffs), row=row_num, col=2)
    fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
    theta = [ele[theta_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(theta, vel_diffs), row=row_num, col=3)
    fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
    phi = [ele[phi_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(phi, vel_diffs), row=row_num, col=4)
    fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
    single_target = [ele[single_target_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(single_target, vel_diffs), row=row_num, col=5)
    fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
    superres_target = [ele[superres_target_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(superres_target, vel_diffs), row=row_num, col=6)
    fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
    bistatic = [ele[bistatic_idx] for ele in vel_diff_list]
    fig_scatter.add_trace(func_scatter(bistatic, vel_diffs), row=row_num, col=7)
    fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)

    # Azimuth error against detection properties
    row_num = 3
    rang = [ele[rng_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(rang, theta_diffs), row=row_num, col=1)
    fig_scatter.update_yaxes(title_text="Azimuth error", row=row_num, col=1)
    fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
    vel = [ele[vel_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(vel, theta_diffs), row=row_num, col=2)
    fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
    theta = [ele[theta_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(theta, theta_diffs), row=row_num, col=3)
    fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
    phi = [ele[phi_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(phi, theta_diffs), row=row_num, col=4)
    fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
    single_target = [ele[single_target_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(single_target, theta_diffs), row=row_num, col=5)
    fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
    superres_target = [ele[superres_target_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(superres_target, theta_diffs), row=row_num, col=6)
    fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
    bistatic = [ele[bistatic_idx] for ele in theta_diff_list]
    fig_scatter.add_trace(func_scatter(bistatic, theta_diffs), row=row_num, col=7)
    fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)

    # Elevation error against detection properties
    row_num = 4
    rang = [ele[rng_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(rang, phi_diffs), row=row_num, col=1)
    fig_scatter.update_yaxes(title_text="Elevation error", row=row_num, col=1)
    fig_scatter.update_xaxes(title_text="Range", row=row_num, col=1)
    vel = [ele[vel_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(vel, phi_diffs), row=row_num, col=2)
    fig_scatter.update_xaxes(title_text="Range Rate", row=row_num, col=2)
    theta = [ele[theta_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(theta, phi_diffs), row=row_num, col=3)
    fig_scatter.update_xaxes(title_text="Azimuth", row=row_num, col=3)
    phi = [ele[phi_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(phi, phi_diffs), row=row_num, col=4)
    fig_scatter.update_xaxes(title_text="Elevation", row=row_num, col=4)
    single_target = [ele[single_target_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(single_target, phi_diffs), row=row_num, col=5)
    fig_scatter.update_xaxes(title_text="Single Target", row=row_num, col=5)
    superres_target = [ele[superres_target_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(superres_target, phi_diffs), row=row_num, col=6)
    fig_scatter.update_xaxes(title_text="Superres Target", row=row_num, col=6)
    bistatic = [ele[bistatic_idx] for ele in phi_diff_list]
    fig_scatter.add_trace(func_scatter(bistatic, phi_diffs), row=row_num, col=7)
    fig_scatter.update_xaxes(title_text="Bistatic", row=row_num, col=7)

    # Update layout for scatter plot
    fig_scatter.update_layout(height=1400, width=1250,
                              title_text="Error v/s detection Range, Range Rate, Azimuth, Elevation", showlegend=False)
    fig_scatter.update_traces(marker_color='red', marker={'size': 2})
    fig_scatter.update_xaxes(zeroline=False, showgrid=False)
    fig_scatter.update_yaxes(zeroline=False, showgrid=False)
    fig_scatter.show()

    scatter_plot_html = pio.to_html(fig_scatter, full_html=False, include_plotlyjs='cdn')
    #################################

    #################################
    # Create subplots for accuracy v/s scanindex:
    #################################
    fig_line = sp.make_subplots(rows=1, cols=1, horizontal_spacing=0.04, vertical_spacing=0.2)

    # Manually add traces for line plots
    fig_line.add_trace(func_line(scan_index_list, accuracy_list), row=1, col=1)
    fig_line.update_yaxes(title_text="Accuracy", row=1, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=1, col=1)

    # Update layout for bar plot
    fig_line.update_layout(height=500, width=1250, title_text="Accuracy v/s scanindex", showlegend=False)
    fig_line.update_traces(marker_color='red')
    fig_line.update_xaxes(zeroline=False, showgrid=False, type='category')
    fig_line.update_yaxes(zeroline=False, showgrid=False)
    fig_line.show()

    acc_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs='cdn')
    #################################

    #################################
    # Create subplots for accuracy v/s scanindex:
    #################################
    fig_line = sp.make_subplots(rows=2, cols=1, horizontal_spacing=0.04, vertical_spacing=0.2)

    # Manually add traces for line plots
    row_num = 1
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=num_af_det_veh_list, mode="lines", name="Veh", line=dict(color="red")), row=row_num, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=num_af_det_sim_list, mode="lines", name="Sim", line=dict(color="blue")), row=row_num, col=1)
    fig_line.update_yaxes(title_text="Num of dets", row=row_num, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)

    row_num = 2
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=max_range_veh_list, mode="lines", name="Veh", line=dict(color="red")), row=row_num, col=1)
    fig_line.add_trace(go.Scatter(x=scan_index_list, y=max_range_sim_list, mode="lines", name="Sim", line=dict(color="blue")), row=row_num, col=1)
    fig_line.update_yaxes(title_text="Max range", row=row_num, col=1)
    fig_line.update_xaxes(title_text="Scan Index", row=row_num, col=1)

    # Update layout for plot
    fig_line.update_layout(height=800, width=1250, title_text="Params v/s scanindex", showlegend=True)
    fig_line.update_xaxes(zeroline=False, showgrid=False, type='category')
    fig_line.update_yaxes(zeroline=False, showgrid=False)
    fig_line.show()

    params_plot_html = pio.to_html(fig_line, full_html=False, include_plotlyjs='cdn')
    #################################

    #################################
    # HTML Content
    #################################
    html_content += f"""
        <b><u>Plots:</u></b>
        <details>
            <summary><i>Plot A</i></summary>
            {bar_plot_html}
        </details>
        <details>
            <summary><i>Plot B</i></summary>
            {scatter_plot_html}
        </details>
        <details>
            <summary><i>Plot C</i></summary>
            {acc_plot_html}
        </details>
        <details>
            <summary><i>Plot D</i></summary>
            {params_plot_html}
        </details>
    </details>
    <hr>
    </body>
    </html>
    """
    
det_file_suffix = '_UDP_GEN7_DET_CORE.csv'
rdd_file_suffix = '_UDP_GEN7_RDD_CORE.csv'
def find_data_files(base_path):
    data_files = {
        'input': [],
        'output': []
    }
    files = os.listdir(base_path)
    files_of_interest = [s for s in files if det_file_suffix in s or rdd_file_suffix in s]
    unique_files = set(s.replace(det_file_suffix, "").replace(rdd_file_suffix, "").strip() for s in files_of_interest)
    for filename in sorted(unique_files):
        det_file_path = os.path.join(base_path, filename + det_file_suffix)
        rdd_file_path = os.path.join(base_path, filename + rdd_file_suffix)
        if "_r0" in filename:
            data_files['output'].append((det_file_path, rdd_file_path))
        else:
            data_files['input'].append((det_file_path, rdd_file_path))
    return data_files

def process_logs(data_files):
    #################################
    # Global variables
    #################################
    global num_of_SI_in_veh_af, num_of_SI_in_sim_af, num_of_same_SI_in_veh_and_sim_af
    global ran_diff_list, vel_diff_list, theta_diff_list, phi_diff_list, snr_diff_list, rcs_diff_list
    global scan_index_list, accuracy_list, num_af_det_veh_list, num_af_det_sim_list, max_range_veh_list, max_range_sim_list
    global html_content, total_real_data, total_sim_data, match_count, fp_count, fn_count, kpi_list
    global output_folder
    #################################
    num_of_logs = len(data_files['input'])
    for i in range(num_of_logs):
        print(f"Processing log: {i+1}/{num_of_logs}")
        #################################
        # Reset the below variables
        #################################
        num_of_SI_in_veh_af = 0
        num_of_SI_in_sim_af = 0
        num_of_same_SI_in_veh_and_sim_af = 0
        ran_diff_list = []
        vel_diff_list = []
        theta_diff_list = []
        phi_diff_list = []
        snr_diff_list = []
        rcs_diff_list = []
        scan_index_list = []
        accuracy_list = []
        num_af_det_veh_list = []
        num_af_det_sim_list = []
        max_range_veh_list = []
        max_range_sim_list = []
        total_real_data = 0
        total_sim_data = 0
        match_count = 0
        fp_count = 0
        fn_count = 0
        kpi_list = []
        if data_files['input']:
            file_name = os.path.basename(data_files['input'][i][0])
            log_name_wo_ext = file_name.replace(det_file_suffix, "")
        html_content += f"""
                <b>Log:</b> {log_name_wo_ext}
                """

        input_det_file = data_files['input'][i][0]
        input_rdd_file = data_files['input'][i][1]
        output_det_file = data_files['output'][i][0]
        output_rdd_file = data_files['output'][i][1]
        main(input_det_file, output_det_file, input_rdd_file, output_rdd_file)

        plot_stats()

        max_logs_in_one_report = 20
        if(((i+1) % max_logs_in_one_report == 0) or (i == (num_of_logs-1))):
            html_content += "</table></body></html>"

            # Write HTML content to file
            output_html = output_folder + f"/detection_kpi_report_{i+1:04d}.html"
            with open(output_html, "w") as f:
                f.write(html_content)
                f.close()
            html_content = "<table><body><html>"


#####################################################################
# START
#####################################################################
if len(sys.argv) != 4:
    print("Usage: python detection_matching_kpi_script.py log_path.txt meta_data.txt C:\\Gitlab\\gen7v1_resim_kpi_scripts")
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
use_sample_data = 0
max_num_of_si_to_process = 0
#################################

#################################
# Constants
#################################
if use_sample_data == 1:
    max_number_of_data = 2
else:
    max_number_of_data = 464
max_number_of_rdd_data = 512
epsilon = 0.0000001
ran_threshold = 0.1 + epsilon
vel_threshold = 0.015 + epsilon
theta_threshold = 0.00873 + epsilon
phi_threshold= 0.00873 + epsilon
snr_threshold= 1 + epsilon
rcs_threshold= 1 + epsilon
number_of_scans_in_veh = 0
number_of_scans_in_sim = 0
number_of_scans_in_both_real_and_sim = 0
ran_diff_list = []
vel_diff_list = []
theta_diff_list = []
phi_diff_list = []
snr_diff_list = []
rcs_diff_list = []
scan_index_list = []
accuracy_list = []
num_af_det_veh_list = []
num_af_det_sim_list = []
max_range_veh_list = []
max_range_sim_list = []
total_real_data = 0
total_sim_data = 0
match_count = 0
fp_count = 0
fn_count = 0
kpi_list = []
#################################

#################################
# Read meta data
#################################
# Initialize an empty dictionary
metadata_dict = {}

# Open and read the file
# with open(meta_data_file, "r") as file:
#     for line in file:
#         # Split each line by the first whitespace to get key and value
#         key, value = line.strip().split(maxsplit=1)
#         # Add to dictionary
#         metadata_dict[key] = value
#print(metadata_dict)
#################################

#################################
# HTML Content
#################################
html_content = f"""
<html>
<head>
    <title>Detection KPIs and Plots</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        table {{ width: 75%; border-collapse: collapse; margin: 20px 0; }}
        th, td {{ border: 1px solid #dddddd; text-align: left; padding: 8px; }}
        th {{ background-color: #f2f2f2; }}
    </style>
</head>
<body>
    <h1>Detection KPIs and Plots</h1>
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
                A detection is said to match a re-simulated detection if it has the same RDD range and doppler index(within a scan index), 
                and the difference(error) in the range, range rate, azimuth and elevation are within the thresholds mentioned below
                <ul>
                    <li>Range : {round(ran_threshold, 5)} m</li>
                    <li>Range rate : {round(vel_threshold, 5)} m/s</li>
                    <li>Azimuth : {round(theta_threshold, 5)} radians</li>
                    <li>Elevation : {round(phi_threshold, 5)} radians</li>
                </ul> 
            <li><b>Accuracy:</b> (Number of matching detections / total number of detections) * 100
            <li><b>Plot A:</b> Plot of % of unmatched detections against the corresponding error. Unmatched detections are those 
            detections having same RDD index in both vehicle and re-simulated data but falling outside at-least one of the above 
            mentioned thresholds.
            <li><b>Plot B:</b> Plot of different errors against the corresponding detection's range, range rate, azimuth and elevation
            <li><b>Plot C:</b> Plot of accuracy across scan indices
            <li><b>Plot D:</b> Plot of different parameters across scan indices
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
#####################################################################
# END
#####################################################################