from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.icollectdata import IDataCollect
from IPS.Sig_Prep.isig_observer import ISigObserver
import IPS.Metadata.GEN7V2.poi as poi
import h5py
import numpy as np
import os
from collections import defaultdict


class RadarHDFDCDataCollect(IDataCollect, ISigObserver):

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 ):
        self._data_event_mediator_obj = event_mediator
        self._radar_datastore_obj = radar_datastore
        self._plot_datastore_obj = plot_datastore
        self._final_poi_sensor_dataset_path = defaultdict(list)
        self.dc_hdf_datapath = self.extract_datapaths(poi.poi_data_DC)
        self.tracker_datapath = None
        self.detection_datapath = None
        self._in_file = None
        self._out_file = None
        self.sensor_index = {
            "0": "FL",
            "1": "FR",
            "2": "RL",
            "3": "RR"
        }

    def clear_data(self):

        self._in_file = None
        self._out_file = None
        self._final_poi_sensor_dataset_path.clear()

    def collect_data(self, input_file=None, output_file=None):

        self._in_file = input_file
        self._out_file = output_file

        self.parse_and_collect_data(self._in_file, self._out_file)
        self._data_event_mediator_obj.notify_event(self, "DC_JSON_GEN_DONE")

    def parse_and_collect_data(self, infile, outfile):
        """

        """

        # Extract Tracker and Detection categories
        self.tracker_datapath = [path for path in self.dc_hdf_datapath if
                                 path.startswith("Tracker_Information") or path.startswith("OSI_Ground_Truth")]
        self.detection_datapath = [path for path in self.dc_hdf_datapath if
                                   path.startswith("Raw_Detection_Information")]

        self.parse_hdf_trackerdata(str(infile), "in", self.tracker_datapath)
        self._data_event_mediator_obj.notify_event("DCtracks", "DC_TRACK_RDS_UPDATED")

        self.parse_hdf_detectiondata(str(infile), "in", self.detection_datapath)
        self._data_event_mediator_obj.notify_event("DCtracks", "DC_DET_RDS_UPDATED")

    def consume_event(self, sensor, event):
        pass

    def final_poi_sensor_datapath(self, data):
        pass

    def parse_hdf_featurefunction_data(self, hdf_file, sourcetype, filtered_dataset_paths):
        """
        This function traverse to the data path in HDF file and read the dataset
        and store the signal value (2D) in radar data store.
        Before storing it does the following operations
        a. removes duplicate rows in 2D data set
        b. removes zeros from all rows of 2D data set

        """

        with h5py.File(str(hdf_file), 'r') as f:
            for dataset_path in filtered_dataset_paths:
                if str(dataset_path) in f:
                    # print(f"Dataset path {dataset_path} found in the file.")
                    sig_name = os.path.basename(dataset_path)
                    sensor = "DCTracks"
                    data = f[str(dataset_path)][()]  # Read entire dataset (default : NumPy array)

                    # Step 1: Remove zeros from each row
                    non_zero_rows = [row[row != 0.0] for row in data]

                    # Step 2: Find the maximum row length after filtering
                    max_row_len = max(len(row) for row in non_zero_rows)

                    # Step 3: Pad rows to make them same length (with 0 or np.nan)
                    filtered_data = np.array([
                        np.pad(row, (0, max_row_len - len(row)), constant_values=0)
                        for row in non_zero_rows
                    ])

                    self._radar_datastore_obj.update_data(sensor, sig_name, filtered_data, sourcetype)

                else:
                    print(f"Dataset {dataset_path} not in file {hdf_file}")

            data = f[str("Scan_Index/scan_index")][()]
            self._radar_datastore_obj.update_data("DCTracks", "scan_index", data, sourcetype)

    def parse_hdf_detectiondata(self, hdf_file, sourcetype, filtered_dataset_paths):
        """
        This function traverse to the data path in HDF file and read the dataset
        and store the signal value (2D) in radar data store.
        Before storing it does the following operations
        a. removes duplicate rows in 2D data set
        b. removes zeros from all rows of 2D data set

        """

        with h5py.File(str(hdf_file), 'r') as f:
            for dataset_path in filtered_dataset_paths:
                if str(dataset_path) in f:
                    # print(f"Dataset path {dataset_path} found in the file.")
                    for index in range(0, 4):
                        sig_name = os.path.basename(dataset_path)
                        sensor = self.sensor_index[str(index)]
                        data_3d = f[str(dataset_path)][()]  # Read entire dataset (default : NumPy array)

                        # Extract each sensor's data
                        data = data_3d[:, :, index]  # Front-Left

                        # Step 1: Remove zeros from each row
                        non_zero_rows = [row[row != 0.0] for row in data]

                        # Step 2: Find the maximum row length after filtering
                        max_row_len = max(len(row) for row in non_zero_rows)

                        # Step 3: Pad rows to make them same length (with 0 or np.nan)
                        filtered_data = np.array([
                            np.pad(row, (0, max_row_len - len(row)), constant_values=0)
                            for row in non_zero_rows
                        ])

                        self._radar_datastore_obj.update_data(sensor, sig_name, filtered_data, sourcetype)
                        data = f[str("Scan_Index/scan_index")][()]
                        self._radar_datastore_obj.update_data(sensor, "scan_index", data, sourcetype)

                else:
                    print(f"Dataset {dataset_path} not in file {hdf_file}")

    def parse_hdf_trackerdata(self, hdf_file, sourcetype, filtered_dataset_paths):
        """
        This function traverse to the data path in HDF file and read the dataset
        and store the signal value (2D) in radar data store.
        Before storing it does the following operations
        a. removes duplicate rows in 2D data set
        b. removes zeros from all rows of 2D data set

        """

        with h5py.File(str(hdf_file), 'r') as f:
            for dataset_path in filtered_dataset_paths:
                if str(dataset_path) in f:
                    # print(f"Dataset path {dataset_path} found in the file.")
                    sig_name = os.path.basename(dataset_path)
                    sensor = "DCTracks"
                    data = f[str(dataset_path)][()]  # Read entire dataset (default : NumPy array)

                    # Step 1: Remove zeros from each row
                    non_zero_rows = [row[row != 0.0] for row in data]

                    # Step 2: Find the maximum row length after filtering
                    max_row_len = max(len(row) for row in non_zero_rows)

                    # Step 3: Pad rows to make them same length (with 0 or np.nan)
                    filtered_data = np.array([
                        np.pad(row, (0, max_row_len - len(row)), constant_values=0)
                        for row in non_zero_rows
                    ])

                    self._radar_datastore_obj.update_data(sensor, sig_name, filtered_data, sourcetype)

                else:
                    print(f"Dataset {dataset_path} not in file {hdf_file}")

            data = f[str("01_Scan_Index/scan_index")][()]
            self._radar_datastore_obj.update_data("DCTracks", "scan_index", data, sourcetype)

    def get_streams(self):
        return poi.poi_data_DC.get("streams", [])

    def get_signals(self, stream):
        return [signal["name"] for signal in poi.poi_data_DC.get(stream, [])]

    def extract_datapaths(self, poi_data: dict) -> list:
        datapaths = []
        for stream in poi_data.get("streams", []):
            for signal in poi_data.get(stream, []):
                datapaths.append(f"{stream}/{signal['name']}")
        return datapaths

    def get_plot_types(self, signal_name: str) -> list:
        for stream in poi.poi_data_DC.get("streams", []):
            for signal in poi.poi_data_DC.get(stream, []):
                if signal["name"] == signal_name:
                    return signal["plots"]
        return []

    def get_unit(self, signal_name: str) -> str:
        for stream in poi.poi_data_DC.get("streams", []):
            for signal in poi.poi_data_DC.get(stream, []):
                if signal["name"] == signal_name:
                    return signal["unit"]
        return "Unit not found"
