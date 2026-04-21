from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.icollectdata import IDataCollect
from IPS.Sig_Prep.isig_observer import ISigObserver
from collections import defaultdict
import pandas as pd
import numpy as np


class CSVUDPDataCollect(IDataCollect, ISigObserver):

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 ):
        self._data_event_mediator_obj = event_mediator
        self._radar_datastore_obj = radar_datastore
        self._plot_datastore_obj = plot_datastore

        self._in_file = None
        self._out_file = None

    def clear_data(self):

        self._in_file = None
        self._out_file = None

    def collect_data(self, input_file=None, output_file=None):

        self._in_file = input_file
        self._out_file = output_file

        self.parse_and_collect_data(self._in_file, self._out_file)
        self._data_event_mediator_obj.notify_event(self, "JSON_GEN_DONE")  # JDS--> JSON Data store

    def parse_and_collect_data(self, infile, outfile):
        """
        This function calls csv parser function for input and output csv
        after parsing of  input & output csv, data sets are updated to radar data store.
        once in & out data set updated to radar data store, an event (RDS_UPDATES) is notified.
        radar data preparation module (radar_dataprep.py) will consume the event (RDS_UPDATES)
        parsing will be done sensor by sensor ( in & out)
        example : parse RL (in and out) --> Generate RL Json (Histogram,scatter......)
                  Generation of RL Json happens signal by signal
        """

        self.parse_csv_data(str(infile), "in")
        self.parse_csv_data(str(outfile), "out")

        self._data_event_mediator_obj.notify_event("RL", "RDS_UPDATED")  # RDS--> Radar Data store

    def final_poi_sensor_datapath(self, data):
        pass

    def parse_csv_data(self, csv_file, sourcetype):
        """
        This function traverse to the data path in HDF file and read the dataset
        and store the signal value (2D) in radar data store.
        Before storing it does the following operations
        a. removes duplicate rows in 2D data set
        b. removes zeros from all rows of 2D data set

        """

        signal_list = ['ran', 'vel', 'scan_index', 'theta', 'phi']

        for sig_name in signal_list:

            if sig_name == "scan_index":
                df = pd.read_csv(csv_file, engine='c', usecols=['scan_index'])
            else:
                columns_to_read = [f'{sig_name}_{i}' for i in range(0, 679)]
                df = pd.read_csv(csv_file, engine='c', usecols=columns_to_read)

            # Convert to 2D NumPy array
            if sig_name == "scan_index":
                data = df.to_numpy().ravel()
            else:
                data = df.to_numpy()

            # check data is 1D or 2D
            if data.ndim == 1:  # 1D
                unique_data, idx = np.unique(data, return_index=True)
                data = unique_data[np.argsort(idx)]

            elif data.ndim == 2:  # 2D
                unique_data, idx = np.unique(data, axis=0, return_index=True)
                data = unique_data[np.argsort(idx)]

            if sig_name != "scan_index":
                # Step 1: Remove zeros from each row
                non_zero_rows = [row[row != 0.0] for row in data]

                # Step 2: Find the maximum row length after filtering
                max_row_len = max(len(row) for row in non_zero_rows)

                # Step 3: Pad rows to make them same length (with 0 or np.nan)
                filtered_data = np.array([
                    np.pad(row, (0, max_row_len - len(row)), constant_values=0)
                    for row in non_zero_rows
                ])

                filtered_data = (np.asarray(filtered_data) * 100).astype(int) / 100.0
            if sig_name != "scan_index":
                self._radar_datastore_obj.update_data("RL", sig_name, filtered_data, sourcetype)
            else:
                self._radar_datastore_obj.update_data("RL", sig_name, data, sourcetype)

    def consume_event(self, sensor, event):
        pass
