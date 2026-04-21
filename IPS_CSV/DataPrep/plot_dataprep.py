from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.Sig_Prep.sig_filter import SignalFilter
from IPS.DashManager.report_dash import ReportDash
import numpy as np
from collections import defaultdict


class PlotDataPreparation:
    output_scan_index_scaled = np.array([])
    input_scan_index_scaled = np.array([])
    list_json_path = []
    mismatch_input_value = np.array([])
    mismatch_output_value = np.array([])
    mismatch_input_scan_index = np.array([])
    mismatch_output_scan_index = np.array([])

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 jsonpath_datastore: IDataStore,
                 sig_filter: SignalFilter):
        self._data_event_mediator_obj = event_mediator
        self._plot_datastore_obj = plot_datastore
        self._radar_datastore_obj = radar_datastore
        self._jsonpath_datastore_obj = jsonpath_datastore
        self._sig_filter_obj = sig_filter

    def clear_data(self):

        PlotDataPreparation.output_scan_index_scaled = np.array([])
        PlotDataPreparation.input_scan_index_scaled = np.array([])
        PlotDataPreparation.list_json_path.clear()
        PlotDataPreparation.mismatch_input_value = np.array([])
        PlotDataPreparation.mismatch_output_value = np.array([])
        PlotDataPreparation.mismatch_input_scan_index = np.array([])
        PlotDataPreparation.mismatch_output_scan_index = np.array([])

    def group_by_index(self, data, scan_index):
        grouped = defaultdict(list)
        for val, idx in zip(data, scan_index):
            grouped[idx].append(val)
        return grouped

    def consume_event(self, sensor, event):

        if event == "PDS_UPDATED":
            streams = self._sig_filter_obj.get_streams()
            for stream in streams:
                signal_list = self._sig_filter_obj.get_sig_in_stream(stream)
                # signal_list = RadarHDFDataCollect.sensor_to_sig[sensor]

                for sig in signal_list:
                    if sig != 'scan_index':
                        # self.compute_scaled_scan_index(sensor, sig)
                        plot_types = self._sig_filter_obj.get_sig_plot_type(stream, sig)
                        if "SC_MM" in plot_types:
                            # self.compute_match_mismatch_missing_additional_value_updated(sig, sensor)
                            # self.compare_vehicle_resim(sig, sensor)
                            self.compare_vehicle_resim_datapoints(sig, sensor)

                    self._data_event_mediator_obj.notify_event(sensor + "#" + str(stream) + "#" + sig,
                                                               "JDS_UPDATED")  # JDS--> JSON Data store

    def compute_scaled_scan_index(self, sensor, sig):

        input_scan_index = self._plot_datastore_obj.get_data(sensor, "scan_index", "in_scan_index")
        input_scan_index_duplicate_factor = self._plot_datastore_obj.get_data(str(sig) + "Dim", "in")
        PlotDataPreparation.input_scan_index_scaled = np.repeat(input_scan_index,
                                                                input_scan_index_duplicate_factor)

        self._plot_datastore_obj.update_data(sig, PlotDataPreparation.input_scan_index_scaled, "mismatch_in",
                                             None,
                                             sensor)

        output_scan_index = self._plot_datastore_obj.get_data(sensor, "scan_index", "out_scan_index")
        output_scan_index_duplicate_factor = self._plot_datastore_obj.get_data(str(sig) + "Dim", "out")
        PlotDataPreparation.output_scan_index_scaled = np.repeat(output_scan_index,
                                                                 output_scan_index_duplicate_factor)

        self._plot_datastore_obj.update_data(sig, PlotDataPreparation.output_scan_index_scaled, "mismatch_in",
                                             None,
                                             sensor)

    def compute_match_mismatch_missing_additional_value_updated(self, sig, sensor):
        # print("compute_match_mismatch_missing_additional_value_updated")

        scanindex_vehicle = self._radar_datastore_obj.get_data(sensor, "scan_index", "in").flatten().astype(int)
        scanindex_simulated = self._radar_datastore_obj.get_data(sensor, "scan_index", "out").flatten().astype(int)
        vehicle_range = self._radar_datastore_obj.get_data(sensor, sig, "in")
        simulated_range = self._radar_datastore_obj.get_data(sensor, sig, "out")

        # ✅ Remove all zeros from each row
        vehicle_range = np.array([
            np.array([float(x) for x in row if float(x) != 0], dtype=float)
            for row in vehicle_range
        ], dtype=object)

        simulated_range = np.array([
            np.array([float(x) for x in row if float(x) != 0], dtype=float)
            for row in simulated_range
        ], dtype=object)

        sim_index_map = {idx: i for i, idx in enumerate(scanindex_simulated)}
        veh_index_map = {idx: i for i, idx in enumerate(scanindex_vehicle)}

        common_scanindices = set(scanindex_vehicle) & set(scanindex_simulated)
        missing_scanindices = set(scanindex_vehicle) - set(scanindex_simulated)
        additional_scanindices = set(scanindex_simulated) - set(scanindex_vehicle)

        mismatch_scanindex = []
        mismatch_scanindex_file1 = []
        mismatch_scanindex_file2 = []
        mismatch_indices = []
        mismatch_values = []
        mismatch_values_file1 = []
        mismatch_values_file2 = []

        missing_scanindex = []
        missing_data = []

        additional_scanindex = []
        additional_data = []

        match_count = 0
        mismatch_count = 0

        for scan_idx in common_scanindices:
            v_idx = veh_index_map[scan_idx]
            s_idx = sim_index_map[scan_idx]

            v_row = vehicle_range[v_idx]
            s_row = simulated_range[s_idx]

            min_len = min(len(v_row), len(s_row))

            for i in range(min_len):
                v_val = v_row[i]
                s_val = s_row[i]
                if not np.isclose(v_val, s_val, atol=1e-2):
                    # mismatch_scanindex.append(int(scan_idx))

                    mismatch_scanindex_file1.append(int(scan_idx))
                    mismatch_scanindex_file2.append(int(scan_idx))

                    mismatch_indices.append((int(v_idx), i))

                    mismatch_values_file1.append(float(v_val))
                    mismatch_values_file2.append(float(s_val))

                    mismatch_values.append((
                        int(scan_idx), float(v_val),
                        int(scan_idx), float(s_val)
                    ))
                    mismatch_count += 1
                else:
                    match_count += 1

        for scan_idx in missing_scanindices:
            idx = veh_index_map[scan_idx]
            if idx < len(vehicle_range):
                missing_scanindex.extend([int(scan_idx)] * len(vehicle_range[idx]))
                missing_data.extend(vehicle_range[idx].tolist())
            else:
                print(f"Warning: idx {idx} out of bounds for vehicle_range with size {len(vehicle_range)}")

        for scan_idx in additional_scanindices:
            idx = sim_index_map[scan_idx]
            additional_scanindex.extend([int(scan_idx)] * len(simulated_range[idx]))  # ✔ wrap in list
            additional_data.extend(simulated_range[idx].tolist())

            # Total mismatches = value + missing + additional
        total_mismatch_count = mismatch_count + len(missing_scanindex) + len(additional_scanindex)
        total = match_count + total_mismatch_count

        match_percent = round((match_count / total) * 100, 2) if total else 0.0
        mismatch_percent = round((total_mismatch_count / total) * 100, 2) if total else 0.0

        if sig == "ran" or sig == "vel" or sig == "phi" or sig == "theta" or sig == "snr" or sig == "rcs":

            # ReportDash.signal_stats[sensor][sig]["match"] = match_percent
            # ReportDash.signal_stats[sensor][sig]["mismatch"] = mismatch_percent

            ReportDash.update_signal_stats(sensor, sig, match_percent, mismatch_percent)
            ReportDash.signal_name_set.add(sig)

            if mismatch_percent != 0:
                self._plot_datastore_obj.mismatch_signal_list[sensor] = sig

                self._plot_datastore_obj.update_data(sig, mismatch_values_file1, "mismatch_in",
                                                     None,
                                                     sensor)
                self._plot_datastore_obj.update_data(sig, None, "mismatch_scan_index_in",
                                                     mismatch_scanindex_file1,
                                                     sensor)

                self._plot_datastore_obj.update_data(sig, mismatch_values_file2, "mismatch_out",
                                                     None,
                                                     sensor)

                self._plot_datastore_obj.update_data(sig, None, "mismatch_scan_index_out",
                                                     mismatch_scanindex_file2,
                                                     sensor)

            if len(missing_scanindex) != 0 or len(additional_scanindex) != 0:
                self._plot_datastore_obj.additional_missing_signal_list[sensor] = sig
                self._plot_datastore_obj.update_data(sig, missing_scanindex, "missing_scan_index",
                                                     None,
                                                     sensor)

                self._plot_datastore_obj.update_data(sig, missing_data, "missing_value",
                                                     None,
                                                     sensor)

                with open("missing_scanindex.txt", "w") as f: f.write(
                    "\n".join(str(item) for item in missing_scanindex))
                with open("missing_data.txt", "w") as f: f.write("\n".join(str(item) for item in missing_data))

                self._plot_datastore_obj.update_data(sig, additional_scanindex, "additional_scan_index",
                                                     None,
                                                     sensor)
                self._plot_datastore_obj.update_data(sig, additional_data, "additional_value",
                                                     None,
                                                     sensor)
                with open("additional_scanindex.txt", "w") as f: f.write(
                    "\n".join(str(item) for item in additional_scanindex))
                with open("additional_data.txt", "w") as f: f.write("\n".join(str(item) for item in additional_data))

    def compare_vehicle_resim(self, sig, sensor):
        # Create dictionaries for quick lookup
        vehicle_data = self._plot_datastore_obj.get_data(sig, "in")
        resim_data = self._plot_datastore_obj.get_data(sig, "out")
        if vehicle_data is not None and resim_data is not None:
            vehicle_scan_index = self._plot_datastore_obj.get_data(sig, "in_scan_index", sensor)
            resim_scan_index = self._plot_datastore_obj.get_data(sig, "out_scan_index", sensor)
            # print("length of vehicle_scan_index", len(vehicle_scan_index))
            # print("length of resim_scan_index", len(resim_scan_index))

            # print("length of vehicle_data", len(vehicle_data))
            # print("length of resim_data", len(resim_data))

            vehicle_grouped = self.group_by_index(vehicle_data, vehicle_scan_index)
            resim_grouped = self.group_by_index(resim_data, resim_scan_index)

            # Initialize result containers
            matched_value_in_vehicle = []
            matched_scanindex_vehicle = []
            matched_value_in_resim = []
            matched_scanindex_resim = []

            mismatch_value_in_vehicle = []
            mismatch_scanindex_vehicle = []
            mismatch_value_in_resim = []
            mismatch_scanindex_resim = []

            missing_value = []
            missing_scanindex = []
            additional_value = []
            additional_scanindex = []

            # Compare values by scan index
            all_indices = set(vehicle_grouped.keys()).union(set(resim_grouped.keys()))
            for idx in sorted(all_indices):
                veh_values = vehicle_grouped.get(idx, [])
                res_values = resim_grouped.get(idx, [])
                min_len = min(len(veh_values), len(res_values))

                # Compare common positions
                for i in range(min_len):
                    if veh_values[i] == res_values[i]:
                        matched_value_in_vehicle.append(veh_values[i])
                        matched_scanindex_vehicle.append(idx)
                        matched_value_in_resim.append(res_values[i])
                        matched_scanindex_resim.append(idx)
                    else:
                        mismatch_value_in_vehicle.append(veh_values[i])
                        mismatch_scanindex_vehicle.append(idx)
                        mismatch_value_in_resim.append(res_values[i])
                        mismatch_scanindex_resim.append(idx)

                # Handle missing and additional
                if len(veh_values) > len(res_values):
                    for i in range(min_len, len(veh_values)):
                        missing_value.append(veh_values[i])
                        missing_scanindex.append(idx)
                elif len(res_values) > len(veh_values):
                    for i in range(min_len, len(res_values)):
                        additional_value.append(res_values[i])
                        additional_scanindex.append(idx)

            # Calculate percentages
            total_comparisons = len(matched_value_in_vehicle) + len(mismatch_value_in_vehicle) + len(
                missing_value) + len(
                additional_value)
            match_percentage = (len(matched_value_in_vehicle) / total_comparisons) * 100 if total_comparisons else 0
            mismatch_percentage = 100 - match_percentage
            '''
            print("_____________________")
            print("sensor", sensor)
            print("signal", sig)
            print("match_percentage", match_percentage)
            print("mismatch_percentage", mismatch_percentage)
            print("_____________________")
            '''
            if sig == "ran" or sig == "vel" or sig == "phi" or sig == "theta" or sig == "snr" or sig == "rcs":

                ReportDash.signal_stats[sensor][sig]["match"] = match_percentage
                ReportDash.signal_stats[sensor][sig]["mismatch"] = mismatch_percentage

                if mismatch_percentage != 0:
                    self._plot_datastore_obj.mismatch_signal_list[sensor] = sig

                    self._plot_datastore_obj.update_data(sig, mismatch_value_in_vehicle, "mismatch_in",
                                                         None, sensor)
                    self._plot_datastore_obj.update_data(sig, None, "mismatch_scan_index_in",
                                                         mismatch_scanindex_vehicle, sensor)

                    self._plot_datastore_obj.update_data(sig, mismatch_value_in_resim, "mismatch_out",
                                                         None,
                                                         sensor)

                    self._plot_datastore_obj.update_data(sig, None, "mismatch_scan_index_out",
                                                         mismatch_scanindex_resim,
                                                         sensor)

                if len(missing_scanindex) != 0 or len(additional_scanindex) != 0:
                    self._plot_datastore_obj.additional_missing_signal_list[sensor] = sig
                    self._plot_datastore_obj.update_data(sig, missing_scanindex, "missing_scan_index",
                                                         None,
                                                         sensor)

                    self._plot_datastore_obj.update_data(sig, missing_value, "missing_value",
                                                         None,
                                                         sensor)
                    '''
                    with open("missing_scanindex.txt", "w") as f: f.write(
                        "\n".join(str(item) for item in missing_scanindex))
                    with open("missing_data.txt", "w") as f: f.write("\n".join(str(item) for item in missing_data))
                    '''
                    self._plot_datastore_obj.update_data(sig, additional_scanindex, "additional_scan_index",
                                                         None,
                                                         sensor)
                    self._plot_datastore_obj.update_data(sig, additional_value, "additional_value",
                                                         None,
                                                         sensor)
                    '''
                    with open("additional_scanindex.txt", "w") as f: f.write(
                        "\n".join(str(item) for item in additional_scanindex))
                    with open("additional_data.txt", "w") as f: f.write("\n".join(str(item) for item in additional_data))
                    '''

    # Pad arrays to equal shape
    def pad_array(self, arr, target_rows, target_cols):
        padded = np.full((target_rows, target_cols), np.nan)
        padded[:arr.shape[0], :arr.shape[1]] = arr
        return padded

    def compare_vehicle_resim_datapoints(self, sig, sensor):
        # Create dictionaries for quick lookup
        try:
            vehicle_data = self._radar_datastore_obj.get_data(sensor, sig, "in")
        except KeyError as e:
            print(f"KeyError: {e} not found in radar datastore.")
            vehicle_data = None  # or handle it another way

        try:
            resim_data = self._radar_datastore_obj.get_data(sensor, sig, "out")
        except KeyError as e:
            print(f"KeyError: {e} not found in radar datastore.")
            resim_data = None  # or handle it another way

        if vehicle_data is not None and resim_data is not None:

            input_scan_index = self._radar_datastore_obj.get_data(sensor, "scan_index", "in")
            output_scan_index = self._radar_datastore_obj.get_data(sensor, "scan_index", "out")
            tolerance_dict = {
                "ran": 0.1,
                "vel": 0.015,
                "phi": 0.00873,
                "theta": 0.00873,
                "snr": 0,
                "rcs": 0

            }

            # Find common scan indices and their positions
            common_indices = np.intersect1d(input_scan_index, output_scan_index)
            # np.isin(input_scan_index, common_indices) returns a boolean array indicating
            # which elements of input_scan_index are
            # present in common_indices. np.nonzero(...) returns the indices of True values in that boolean array.
            # [0] extracts the first element of the tuple returned by np.nonzero,
            # which is the array of matching indices
            input_positions = np.nonzero(np.isin(input_scan_index, common_indices))[0]
            output_positions = np.nonzero(np.isin(output_scan_index, common_indices))[0]

            # Extract rows corresponding to common scan indices
            array1_common = vehicle_data[input_positions]
            array2_common = resim_data[output_positions]

            target_rows = max(array1_common.shape[0], array2_common.shape[0])
            target_cols = max(array1_common.shape[1], array2_common.shape[1])

            array1_padded = self.pad_array(array1_common, target_rows, target_cols)
            array2_padded = self.pad_array(array2_common, target_rows, target_cols)

            # Compare with tolerance
            mask = np.isclose(array1_padded, array2_padded, atol=tolerance_dict[sig], equal_nan=False)
            # performing an element-wise comparison between two NumPy arrays (array1_padded and array2_padded)
            # to check if their values are close within a specified absolute tolerance

            # Extract matching and missing values
            matching_values = np.where(mask, array1_padded, np.nan)
            missing_values = np.where(mask, np.nan, array1_padded)
            # using the mask from np.isclose(...) to separate matching and non-matching values from array1_padded
            # matching_values: Keeps values from array1_padded where the mask is True (i.e., values are close to array2_padded),
            # and replaces others with NaN. missing_values: Keeps values from array1_padded where the mask is False (i.e.,
            # values are not close), and replaces matching ones with NaN.

            # Clean columns with all NaNs
            matched_clean_2d = matching_values[:, ~np.all(np.isnan(matching_values), axis=0)]
            missing_clean_2d = missing_values[:, ~np.all(np.isnan(missing_values), axis=0)]

            # Calculate percentages
            total_elements = np.count_nonzero(~np.isnan(array1_padded))
            matched_elements = np.count_nonzero(~np.isnan(matched_clean_2d))
            missing_elements = np.count_nonzero(~np.isnan(missing_clean_2d))

            match_percentage = (matched_elements / total_elements) * 100
            miss_percentage = (missing_elements / total_elements) * 100

            # Compute missing and additional scan indices
            missing_scan_index = np.setdiff1d(input_scan_index, output_scan_index)
            additional_scan_index = np.setdiff1d(output_scan_index, input_scan_index)

            input_scan_index = np.squeeze(input_scan_index)
            if input_scan_index.ndim != 1:
                raise ValueError("input_scan_index must be a 1D array")

            mask = np.isin(input_scan_index, missing_scan_index)
            if mask.shape[0] != vehicle_data.shape[0]:
                raise ValueError("Mask and vehicle_data row count mismatch")

            missing_values_from_array1 = vehicle_data[mask]

            # missing_values_from_array1 = vehicle_data[np.isin(input_scan_index, missing_scan_index)]

            # additional_values_from_array2 = resim_data[np.isin(output_scan_index, additional_scan_index)]
            output_scan_index = np.squeeze(output_scan_index)
            if output_scan_index.ndim != 1:
                raise ValueError("output_scan_index must be a 1D array")

            mask = np.isin(output_scan_index, additional_scan_index)
            if mask.shape[0] != resim_data.shape[0]:
                raise ValueError("Mask and vehicle_data row count mismatch")

            mask = np.isin(output_scan_index, additional_scan_index)
            additional_values_from_array2 = resim_data[mask]

            # Prepare duplicated scan indices and missing data for plotting
            missing_scan_indices_for_plot = []
            missing_data_for_plot = []
            for idx, row in zip(input_scan_index[input_positions], missing_values):
                for val in row:
                    if not np.isnan(val):
                        missing_scan_indices_for_plot.append(idx)
                        missing_data_for_plot.append(val)

            # Duplicate scan indices based on number of values in each row (completely missing)
            duplicated_missing_indices = []
            missing_data_values = []
            for idx, row in zip(missing_scan_index, missing_values_from_array1):
                duplicated_missing_indices.extend([idx] * len(row))
                missing_data_values.extend(row.tolist())

            # Duplicate additional scan indices based on number of values in each row
            duplicated_additional_indices = []
            additional_data_values = []
            for idx, row in zip(additional_scan_index, additional_values_from_array2):
                duplicated_additional_indices.extend([idx] * len(row))
                additional_data_values.extend(row.tolist())

            if sig == "ran" or sig == "vel" or sig == "phi" or sig == "theta" or sig == "snr" or sig == "rcs":

                ReportDash.signal_stats[sensor][sig]["match"] = match_percentage
                ReportDash.signal_stats[sensor][sig]["mismatch"] = miss_percentage

                if miss_percentage != 0:
                    self._plot_datastore_obj.mismatch_signal_list[sensor] = sig

                    self._plot_datastore_obj.update_data(sig, missing_data_values, "mismatch_in",
                                                         None, sensor)
                    self._plot_datastore_obj.update_data(sig, None, "mismatch_scan_index_in",
                                                         duplicated_missing_indices, sensor)

                if len(missing_data_for_plot) != 0 or len(additional_data_values) != 0:
                    self._plot_datastore_obj.additional_missing_signal_list[sensor] = sig
                    self._plot_datastore_obj.update_data(sig, missing_scan_indices_for_plot, "missing_scan_index",
                                                         None,
                                                         sensor)

                    self._plot_datastore_obj.update_data(sig, missing_data_for_plot, "missing_value",
                                                         None,
                                                         sensor)
                    '''
                    with open("missing_scanindex.txt", "w") as f: f.write(
                        "\n".join(str(item) for item in missing_scanindex))
                    with open("missing_data.txt", "w") as f: f.write("\n".join(str(item) for item in missing_data))
                    '''
                    self._plot_datastore_obj.update_data(sig, duplicated_additional_indices, "additional_scan_index",
                                                         None,
                                                         sensor)
                    self._plot_datastore_obj.update_data(sig, additional_data_values, "additional_value",
                                                         None,
                                                         sensor)
                    '''
                    with open("additional_scanindex.txt", "w") as f: f.write(
                        "\n".join(str(item) for item in additional_scanindex))
                    with open("additional_data.txt", "w") as f: f.write("\n".join(str(item) for item in additional_data))
                    '''
