import pandas as pd
from collections import Counter

from InteractivePlot.c_business_layer.data_prep import DataPrep
# from KPI.detection_matching_kpi import KPI_Detector 

# Define a Protocol for Data Providers (Interface Segregation Principle)
class RadarDataProvider:
    def get_veh_det_df(self):
        raise NotImplementedError

    def get_sim_det_df(self):
        raise NotImplementedError

    def get_veh_rdd_df(self):
        raise NotImplementedError

    def get_sim_rdd_df(self):
        raise NotImplementedError

# DataContainer acts as a Data Provider (Dependency Inversion Principle)
class DataContainer(RadarDataProvider):
    def __init__(self, data_container_in, val_sig_map_in, data_container_out, val_sig_map_out, html_name,
                 veh_det_df, sim_det_df, veh_rdd_df, sim_rdd_df):
        self.data_container_in = data_container_in
        self.val_sig_map_in = val_sig_map_in
        self.sig_val_map_in = self.reverse_mapping(val_sig_map_in)  # Create reverse mapping here
        self.data_container_out = data_container_out
        self.val_sig_map_out = val_sig_map_out
        self.sig_val_map_out = self.reverse_mapping(val_sig_map_out)  # Create reverse mapping here
        self.html_name = html_name
        self.veh_det_df = veh_det_df
        self.sim_det_df = sim_det_df
        self.veh_rdd_df = veh_rdd_df
        self.sim_rdd_df = sim_rdd_df

        # Removed DataPrep instantiation:  Its purpose and usage are unclear.
        # If you need to *use* DataPrep, show how it is used.

    def reverse_mapping(self, mapping):
        return {v: k for k, v in mapping.items()}

    # Data Provider methods
    def get_veh_det_df(self):
        return self.veh_det_df

    def get_sim_det_df(self):
        return self.sim_det_df

    def get_veh_rdd_df(self):
        return self.veh_rdd_df

    def get_sim_rdd_df(self):
        return self.sim_rdd_df

class RadarDataAnalyzer:
    def __init__(self, data_provider, max_num_of_si_to_process=0,
                 ran_threshold=1.0, vel_threshold=1.0, theta_threshold=1.0, phi_threshold=1.0,
                 max_number_of_data=10, max_number_of_rdd_data=10):

        self.data_provider = data_provider
        self.max_num_of_si_to_process = max_num_of_si_to_process
        self.ran_threshold = ran_threshold
        self.vel_threshold = vel_threshold
        self.theta_threshold = theta_threshold
        self.phi_threshold = phi_threshold
        self.max_number_of_data = max_number_of_data
        self.max_number_of_rdd_data = max_number_of_rdd_data

        # Initialize global variables (as attributes)
        self.num_of_SI_in_veh_af = 0
        self.num_of_SI_in_sim_af = 0
        self.num_of_same_SI_in_veh_and_sim_af = 0
        self.ran_diff_list = []
        self.vel_diff_list = []
        self.theta_diff_list = []
        self.phi_diff_list = []
        self.snr_diff_list = []
        self.rcs_diff_list = []
        self.scan_index_list = []
        self.accuracy_list = []
        self.num_af_det_veh_list = []
        self.num_af_det_sim_list = []
        self.max_range_veh_list = []
        self.max_range_sim_list = []
        self.html_content = ""

    def read_data(self):
        """Gets the DataFrames from the DataProvider."""
        print("Getting the DataFrames from the DataProvider...")
        self.veh_det_df = self.data_provider.get_veh_det_df()
        self.sim_det_df = self.data_provider.get_sim_det_df()
        self.veh_rdd_df = self.data_provider.get_veh_rdd_df()
        self.sim_rdd_df = self.data_provider.get_sim_rdd_df()

        if (self.max_num_of_si_to_process != 0):
            nrows = self.max_num_of_si_to_process
            self.veh_det_df = self.veh_det_df.iloc[:nrows]
            self.sim_det_df = self.sim_det_df.iloc[:nrows]
            self.veh_rdd_df = self.veh_rdd_df.iloc[:nrows]
            self.sim_rdd_df = self.sim_rdd_df.iloc[:nrows]

        self.veh_det_df = self.veh_det_df[self.veh_det_df['num_af_det'] != 0]
        self.sim_det_df = self.sim_det_df[self.sim_det_df['num_af_det'] != 0]
        self.num_of_SI_in_veh_af = self.veh_det_df.shape[0]
        self.num_of_SI_in_sim_af = self.sim_det_df.shape[0]

        self.veh_rdd_df = self.veh_rdd_df[self.veh_rdd_df['rdd1_num_detect'] != 0]
        self.sim_rdd_df = self.sim_rdd_df[self.sim_rdd_df['rdd1_num_detect'] != 0]
        self.num_of_SI_in_veh_rdd = self.veh_rdd_df.shape[0]
        self.num_of_SI_in_sim_rdd = self.sim_rdd_df.shape[0]
        print(f"Number of SI in (vehicle, simulation): ({self.num_of_SI_in_veh_rdd}, {self.num_of_SI_in_sim_rdd})")

    def perform_rdd_stream_matching(self):
        """Performs RDD stream matching and calculates KPIs."""
        print("Performing RDD Stream matching...")
        SCALE_P21_TO_FLOAT = (4.768371582e-07)

        # Step 1: Merge the dataframes on 'scan_index'
        merged_df = pd.merge(self.veh_rdd_df, self.sim_rdd_df, on='scan_index', suffixes=('_veh', '_sim'))
        num_of_same_SI_in_veh_and_sim_rdd = merged_df.shape[0]
        num_of_SI_with_same_num_of_rdd1_dets = merged_df[merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']].shape[0]

        # Generate column names
        rindx_cols_veh = [f"rdd1_rindx_{i}_veh" for i in range(self.max_number_of_rdd_data)]
        dindx_cols_veh = [f"rdd1_dindx_{i}_veh" for i in range(self.max_number_of_rdd_data)]
        range_cols_veh = [f"rdd2_range_{i}_veh" for i in range(self.max_number_of_rdd_data)]
        range_rate_cols_veh = [f"rdd2_range_rate_{i}_veh" for i in range(self.max_number_of_rdd_data)]
        rindx_cols_sim = [f"rdd1_rindx_{i}_sim" for i in range(self.max_number_of_rdd_data)]
        dindx_cols_sim = [f"rdd1_dindx_{i}_sim" for i in range(self.max_number_of_rdd_data)]
        range_cols_sim = [f"rdd2_range_{i}_sim" for i in range(self.max_number_of_rdd_data)]
        range_rate_cols_sim = [f"rdd2_range_rate_{i}_sim" for i in range(self.max_number_of_rdd_data)]

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

        print(merged_df.apply(count_rindx_dindx_matches, axis=1))
        if merged_df.empty:
            print("The merged_df DataFrame is empty.")
        else:
            print("Applying count_rindx_dindx_matches...")
        merged_df['matched_rindx_dindx_pairs'] = merged_df.apply(count_rindx_dindx_matches, axis=1)

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
                    sim_range = round(sim_range * SCALE_P21_TO_FLOAT, 3)
                    sim_range_rate = round(sim_range_rate * SCALE_P21_TO_FLOAT, 3)
                    veh_range = row[range_cols_veh[idx]]
                    veh_range_rate = row[range_rate_cols_veh[idx]]
                    veh_range = round(veh_range * SCALE_P21_TO_FLOAT, 3)
                    veh_range_rate = round(veh_range_rate * SCALE_P21_TO_FLOAT, 3)
                    if (
                            abs(sim_range - veh_range) <= self.ran_threshold
                            and abs(sim_range_rate - veh_range_rate) <= self.vel_threshold
                    ):
                        match_count += 1
            return match_count

        merged_df['range_rangerate_matches'] = merged_df.apply(count_range_matches, axis=1)

        merged_df['same_num_of_RDD1_detections'] = merged_df['rdd1_num_detect_veh'] == merged_df['rdd1_num_detect_sim']
        merged_df['matching_pct_rindx_dindx_pairs'] = merged_df['matched_rindx_dindx_pairs']/merged_df['rdd1_num_detect_veh']
        merged_df['matching_pct_range_rangerate_pairs'] = merged_df['range_rangerate_matches'] / merged_df['matched_rindx_dindx_pairs']

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
        print(f"Number of same SI available in both vehicle and simulation: {num_of_same_SI_in_veh_and_sim_rdd}")
        print(f"% of SI with same number of RDD1 detections: "
              f"{kpis_rdd['result1']['numerator']}/{kpis_rdd['result1']['denominator']} --> {kpis_rdd['result1']['value']}%" )
        print(f"% of SI with 100% matching (rindx, dindx) pair: "
              f"{kpis_rdd['result2']['numerator']}/{kpis_rdd['result2']['denominator']} --> {kpis_rdd['result2']['value']}%, "
              f"{kpis_rdd['result3']['numerator']}/{kpis_rdd['result3']['denominator']} --> {kpis_rdd['result3']['value']}%")
        print(f"% of SI with 100% matching (range, rangerate) pair: "
              f"{kpis_rdd['result4']['numerator']}/{kpis_rdd['result4']['denominator']} --> {kpis_rdd['result4']['value']}%, "
              f"{kpis_rdd['result5']['numerator']}/{kpis_rdd['result5']['denominator']} --> {kpis_rdd['result5']['value']}%")

    def extract_rdd_indices(self):
        """
        Extracts rdd1_rindx and rdd1_dindx values from veh_rdd_df and sim_rdd_df
        and appends them to veh_det_df and sim_det_df, respectively.
        """
        print("Extracting rdd1_rindx and rdd1_dindx values from veh_rdd_df...")
        new_data = {f"rdd1_rindx_{i}": None for i in range(self.max_number_of_data)}
        new_data.update({f"rdd1_dindx_{i}": None for i in range(self.max_number_of_data)})
        new_columns_df = pd.DataFrame(new_data, index=self.veh_det_df.index)
        self.veh_det_df = pd.concat([self.veh_det_df, new_columns_df], axis=1)

        for idx, row in self.veh_det_df.iterrows():
            scan_index = row['scan_index']
            rdd_row = self.veh_rdd_df[self.veh_rdd_df['scan_index'] == scan_index]

            if not rdd_row.empty:
                for i in range(len([col for col in self.veh_det_df.columns if col.startswith('rdd_idx')])):
                    rdd_idx = row[f'rdd_idx_{i}']
                    self.veh_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                    self.veh_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]

        print("Extracting rdd1_rindx and rdd1_dindx values from sim_rdd_df...")
        new_columns_df = pd.DataFrame(new_data, index=self.sim_det_df.index)
        self.sim_det_df = pd.concat([self.sim_det_df, new_columns_df], axis=1)

        for idx, row in self.sim_det_df.iterrows():
            scan_index = row['scan_index']
            rdd_row = self.sim_rdd_df[self.sim_rdd_df['scan_index'] == scan_index]

            if not rdd_row.empty:
                for i in range(len([col for col in self.sim_det_df.columns if col.startswith('rdd_idx')])):
                    rdd_idx = row[f'rdd_idx_{i}']
                    self.sim_det_df.at[idx, f'rdd1_rindx_{i}'] = rdd_row[f'rdd1_rindx_{rdd_idx}'].values[0]
                    self.sim_det_df.at[idx, f'rdd1_dindx_{i}'] = rdd_row[f'rdd1_dindx_{rdd_idx}'].values[0]

    def perform_af_stream_matching(self):
        """Merges Vehicle and Resim data for AF stream matching."""
        print("Merging the dataframes...")
        result_df = pd.merge(self.veh_det_df, self.sim_det_df, on='scan_index', how='inner', suffixes=('_veh', '_sim'))
        if (self.max_num_of_si_to_process != 0):
            result_df = result_df.iloc[:self.max_num_of_si_to_process]
        self.num_of_same_SI_in_veh_and_sim_af = result_df.shape[0]
        num_of_SI_with_same_num_of_dets_af = result_df[result_df['num_af_det_veh'] <= result_df['num_af_det_sim']].shape[0]
        base_columns = ['scan_index', 'num_af_det_veh', 'num_af_det_sim']
        for i in range(self.max_number_of_data):
            base_columns.extend([f'rdd_idx_{i}_veh', f'ran_{i}_veh', f'vel_{i}_veh', f'theta_{i}_veh', f'phi_{i}_veh',
                                f'f_single_target_{i}_veh', f'f_superres_target_{i}_veh', f'f_bistatic_{i}_veh'])
            base_columns.extend([f'rdd_idx_{i}_sim', f'ran_{i}_sim', f'vel_{i}_sim', f'theta_{i}_sim', f'phi_{i}_sim',
                                f'f_single_target_{i}_sim', f'f_superres_target_{i}_sim', f'f_bistatic_{i}_sim'])

        common_columns = [col for col in base_columns if col in result_df.columns]
        result_df = result_df[common_columns]

        self.result_df = result_df

    def run_analysis(self):
        """
        Executes the full analysis pipeline.
        """
        self.read_data()
        self.perform_rdd_stream_matching()
        self.extract_rdd_indices()
        self.perform_af_stream_matching()

    def get_results(self):
        """
        Returns the result DataFrame.
        """
        return self.result_df

# veh_det_df = pd.read_csv('path/to/vehicle_det.csv')
# sim_det_df = pd.read_csv('path/to/simulation_det.csv')
# veh_rdd_df = pd.read_csv('path/to/vehicle_rdd.csv')
# sim_rdd_df = pd.read_csv('path/to/simulation_rdd.csv')

# Example of creating DataContainer and RadarDataAnalyzer:
# data_container = DataContainer(

#     html_name="my_report.html",
#     veh_det_df=veh_det_df,
#     sim_det_df=sim_det_df,
#     veh_rdd_df=veh_rdd_df,
#     sim_rdd_df=sim_rdd_df
# )

# analyzer = RadarDataAnalyzer(data_container)
# analyzer.run_analysis()
# results = analyzer.get_results()

# detector = KPI_Detector(results.to_dict('records'))
# kpi_results = detector.detect()
