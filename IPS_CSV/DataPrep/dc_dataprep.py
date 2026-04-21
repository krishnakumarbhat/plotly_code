from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.DataStore.idatastore import IDataStore
from IPS.Sig_Prep.sig_filter import SignalFilter
from IPS.DataCollect.icollectdata import IDataCollect
import numpy as np


class DCPlotDataPreparation:

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 jsonpath_datastore: IDataStore,
                 sig_filter: SignalFilter,
                 dc_hdf_datacollector: IDataCollect):
        self._data_event_mediator_obj = event_mediator
        self._plot_datastore_obj = plot_datastore
        self._radar_datastore_obj = radar_datastore
        self._jsonpath_datastore_obj = jsonpath_datastore
        self._sig_filter_obj = sig_filter
        self._dc_hdf_datacollector = dc_hdf_datacollector

    def clear_data(self):
        pass

    def consume_event(self, sensor, event):

        if event == "DC_TRACK_RDS_UPDATED":

            dc_stream = self._dc_hdf_datacollector.get_streams()
            for stream in dc_stream:
                if stream == "Tracker_Information/OLP" or stream == "OSI_Ground_Truth/Object":
                    dc_tracker_signal_list = self._dc_hdf_datacollector.get_signals(str(stream))
                    for sig in dc_tracker_signal_list:
                        self.flatten_radar_data("DCTracks", sig)
                        self._data_event_mediator_obj.notify_event("DCTracks" + "#" + str(stream) + "#" + sig,
                                                                   "JDS_UPDATED")

        if event == "DC_DET_RDS_UPDATED":
            dc_stream = self._dc_hdf_datacollector.get_streams()
            for stream in dc_stream:

                if stream == "Raw_Detection_Information":
                    dc_detection_signal_list = self._dc_hdf_datacollector.get_signals(str(stream))
                    for sig in dc_detection_signal_list:
                        for sensor in ["FL", "FR", "RR", "RL"]:
                            self.flatten_radar_data(sensor, sig)
                            self._data_event_mediator_obj.notify_event(sensor + "#" + str(stream) + "#" + sig,
                                                                       "JDS_UPDATED")

    def flatten_radar_data(self, sensor, sig):
        """
        This function gets radar data(2D) of signal from radar data store and takes only two decimal
        places and getting the dimension of signal and update signal dimension to plot data store.
        updated signal dimension is used as scaling factor for scan index duplication ( required for plotting)

        then convert 2D radar data to 1D data and update to plot data store.

        signals like phi and theta are converted from radian to degree and updated to plot data store.
        From plot data store we can retrieve to plot scatter plots

        """

        sig_data = self._radar_datastore_obj.get_data(sensor, sig, "in")
        rounded_sig_data = (sig_data * 100).astype(int) / 100.0
        input_scan_index_scaling_factor = rounded_sig_data.shape[1] if rounded_sig_data.ndim == 2 else "Not 2D array"

        if input_scan_index_scaling_factor != "Not 2D array":
            self._plot_datastore_obj.update_data(str(sig) + "Dim", input_scan_index_scaling_factor, "in")
            scan_index_in = self._radar_datastore_obj.get_data(sensor, "scan_index", "in")

            scaled_scan_index_array_in = np.repeat(scan_index_in, input_scan_index_scaling_factor)
            self._plot_datastore_obj.update_data(sig,
                                                 scaled_scan_index_array_in,
                                                 "in_scan_index", None, sensor)
        rounded_sig_data = rounded_sig_data.ravel()  # ravel(convert 2d to 1d data)

        self._plot_datastore_obj.update_data(sig, rounded_sig_data, "in")
