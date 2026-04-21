from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.DataCollect.radar_hdf_datacollect import RadarHDFDataCollect
import numpy as np


# RadarDataPreparation

class CSVRadarDataPreparation:
    input_scan_index_arr = None
    output_scan_index_arr = None

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore, ):
        self._data_event_mediator = event_mediator
        self._radar_datastore = radar_datastore
        self._plot_datastore = plot_datastore

    def clear_data(self):

        CSVRadarDataPreparation.input_scan_index_arr = None
        CSVRadarDataPreparation.output_scan_index_arr = None

    def check_type(data):
        if isinstance(data, np.ndarray):
            return "NumPy array"
        elif isinstance(data, list):
            return "Python list"
        else:
            return "Other type"

    def consume_event(self, sensor, event):
        """
        Once HDF data collection is completed, an event RDS_UPDATED is notified.
        the event RDS_UPDATED is consumed here. Here we call flatten ( conversion of 2d to 1d)
        scan index and flatten ( conversion of 2d to 1d) radar data and store the flattened
        in plot data store (PDS).

        an event PDS_UPDATED is notified
        """

        if event == "RDS_UPDATED":

            self.flatten_scan_index("RL", "scan_index")
            signal_list = ['ran', 'vel', 'theta', 'phi']
            for sig in signal_list:
                self.flatten_radar_data(sensor, sig)

            self._data_event_mediator.notify_event(sensor, "PDS_UPDATED")  # PDS--> Plot Data store

    def flatten_scan_index(self, sensor, sig):
        """
           This function get scan index from radar data store and convert 2D scan index
           to 1D scan index using ravel() and update flatted scan index to plot data store
        """
        # print("flatten_scan_index", sig)
        input_scan_index_arr = self._radar_datastore.get_data(sensor, sig, "in")
        input_scan_index_arr = input_scan_index_arr.ravel()
        self._plot_datastore.update_data(sig, input_scan_index_arr, "in")

        output_scan_index_arr = self._radar_datastore.get_data(sensor, sig, "out")
        output_scan_index_arr = output_scan_index_arr.ravel()
        self._plot_datastore.update_data(sig, output_scan_index_arr, "out")

    def flatten_radar_data(self, sensor, sig):
        """
        This function gets radar data(2D) of signal from radar data store and takes only two decimal
        places and getting the dimension of signal and update signal dimension to plot data store.
        updated signal dimension is used as scaling factor for scan index duplication ( required for plotting)

        then convert 2D radar data to 1D data and update to plot data store.

        signals like phi and theta are converted from radian to degree and updated to plot data store.
        From plot data store we can retrieve to plot scatter plots

        """
        # print("flatten_radar_data", sig)
        sig_data = self._radar_datastore.get_data(sensor, sig, "in")
        rounded_sig_data = (sig_data * 100).astype(int) / 100.0
        input_scan_index_scaling_factor = rounded_sig_data.shape[1] if rounded_sig_data.ndim == 2 else "Not 2D array"

        if input_scan_index_scaling_factor != "Not 2D array":
            self._plot_datastore.update_data(str(sig) + "Dim", input_scan_index_scaling_factor, "in")
            scan_index_in = self._plot_datastore.get_data('scan_index', "in")
            scaled_scan_index_array_in = np.repeat(scan_index_in, input_scan_index_scaling_factor)
            self._plot_datastore.update_data(sig,
                                             scaled_scan_index_array_in,
                                             "in", None, sensor)

            # update_data(self, signal, signal_numpy_data, data_source=None, mismatch_scan_index=None, sensor=None):
        rounded_sig_data = rounded_sig_data.ravel()  # ravel(convert 2d to 1d data)

        if str(sig) == "theta" or str(sig) == "phi":
            rounded_sig_data = rounded_sig_data.astype(np.float32)
            np.degrees(rounded_sig_data, out=rounded_sig_data)
        '''
        if str(sig) == "veh_speed":
            rounded_sig_data *= 3.6
        '''
        self._plot_datastore.update_data(sig, rounded_sig_data, "in")

        sig_data = self._radar_datastore.get_data(sensor, sig, "out")
        rounded_sig_data = (sig_data * 100).astype(int) / 100.0
        output_scan_index_scaling_factor = rounded_sig_data.shape[1] if rounded_sig_data.ndim == 2 else "Not 2D array"

        if output_scan_index_scaling_factor != "Not 2D array":
            self._plot_datastore.update_data(sig + "Dim", output_scan_index_scaling_factor, "out")

            scan_index_out = self._plot_datastore.get_data('scan_index', "out")
            scaled_scan_index_array_out = np.repeat(scan_index_out, output_scan_index_scaling_factor)
            self._plot_datastore.update_data(sig,
                                             scaled_scan_index_array_out,
                                             "out", None, sensor)
        rounded_sig_data = rounded_sig_data.ravel()

        if str(sig) == "theta" or str(sig) == "phi":  # converting rad to degree in place
            rounded_sig_data = rounded_sig_data.astype(np.float32)
            np.degrees(rounded_sig_data, out=rounded_sig_data)
        '''
        if str(sig) == "veh_speed":
            rounded_sig_data *= 3.6
        '''
        self._plot_datastore.update_data(sig, rounded_sig_data, "out")
