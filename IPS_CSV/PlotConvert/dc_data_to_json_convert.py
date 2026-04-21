import gc
import os
import plotly.graph_objects as go
from IPS.DashManager.report_dash import ReportDash
from IPS.DataPrep.plot_dataprep import PlotDataPreparation
from IPS.DataStore.idatastore import IDataStore
from IPS.EventMan.ievent_mediator import IEventMediator
from IPS.Sig_Prep.isig_observer import ISigObserver
from IPS.Sig_Prep.sig_filter import SignalFilter


class DCDataToJSONConvert(ISigObserver):

    def __init__(self, event_mediator: IEventMediator,
                 radar_datastore: IDataStore,
                 plot_datastore: IDataStore,
                 jsonpath_datastore: IDataStore,
                 plot_data_prep: PlotDataPreparation,
                 sig_filter: SignalFilter):
        self._data_event_mediator_obj = event_mediator
        self._radar_datastore_obj = radar_datastore
        self._plot_datastore_obj = plot_datastore
        self._jsonpath_datastore_obj = jsonpath_datastore
        self._plot_data_prep_obj = plot_data_prep
        self._sig_filter_obj = sig_filter

    def final_poi_sensor_datapath(self, data):
        pass

    def consume_event(self, sensor_info, event):
        """
        This function Consumes an event JDS_UPDATED
        event carry info (sensor#stream#signal)
        index 0 : sensor
        index 1 : stream
        index 2 : signal
        """

        if event == "JDS_UPDATED":
            sensor_info = sensor_info.split('#')
            if sensor_info[1] != 'scan_index':
                self.generate_json_plot(sensor_info[0], sensor_info[1], sensor_info[2])

    def generate_json_plot(self, sensor, stream, signal):

        self.generate_json_scatter_overlay(sensor, stream, signal, "plot_unit")

    def generate_histogram(self, sensor, stream, signal):

        data1 = self._plot_datastore_obj.get_data(signal, "in")
        data2 = self._plot_datastore_obj.get_data(signal, "out")

        if data1 is not None:
            data1 = self._plot_datastore_obj.get_data(signal, "in").tolist()
        if data2 is not None:
            data2 = self._plot_datastore_obj.get_data(signal, "out").tolist()

        if data1 is not None or data2 is not None:
            # Create histogram traces with transparency
            trace1 = go.Histogram(
                x=data1,
                opacity=0.5,
                name='input',
                marker_color='blue'
            )

            trace2 = go.Histogram(
                x=data2,
                opacity=0.5,
                name='output',
                marker_color='red'
            )

            # Combine traces
            fig = go.Figure(data=[trace1, trace2])

            # Set overlay mode for transparency
            fig.update_layout(
                barmode='overlay',
                title='Histogram ',
                xaxis_title='Value',
                yaxis_title='Count'
            )

            # Ensure the report directory exists
            sensor_folder = os.path.join(ReportDash.report_directory + "/JSON", sensor)
            os.makedirs(sensor_folder, exist_ok=True)

            # Save the figure as a JSON file
            json_path = sensor + "_" + stream + "_" + str("histogram") + "_" + signal + ".json"

            file_path = os.path.join(sensor_folder, json_path)

            PlotDataPreparation.list_json_path.append(json_path)
            self._jsonpath_datastore_obj.update_data(sensor, file_path, "Histogram")

            with open(file_path, "w", encoding="utf-8") as f:
                f.write(fig.to_json())

    def generate_json_scatter_overlay(self, sensor, stream, sig, plot_unit):

        # print("generate_json_scatter_overlay")
        if sig != "scan_index":
            input_data = self._plot_datastore_obj.get_data(sig, "in")

            if input_data is not None:
                input_data = self._plot_datastore_obj.get_data(sig, "in").tolist()
                input_scan_index = self._plot_datastore_obj.get_data(sig, "in_scan_index", sensor).tolist()

            if input_data is not None:
                fig = go.Figure()

                fig.add_trace(
                    go.Scattergl(
                        x=input_scan_index,
                        y=input_data,
                        mode='markers',
                        marker=dict(size=3, opacity=0.5, color="blue"),
                        name="input"
                    )
                )

                fig.update_layout(
                    # title=str(sensor) + "_" + str(stream) + "_" + "scatter" + "_" + str(sig),
                    xaxis_title="Scan Index (ms)",
                    yaxis_title=str(plot_unit)
                )
                # print("input_scan_index",input_scan_index)
                # print("input_data", input_data)

                gc.collect()

                # Ensure the report directory exists
                sensor_folder = os.path.abspath(os.path.join(ReportDash.report_directory, "JSON", sensor))
                os.makedirs(sensor_folder, exist_ok=True)

                #stream = stream.split('/')[-2]
                stream = stream.split('/')[-2] if '/' in stream else stream
                json_path = f"{sensor}_{stream}_scatter_{sig}.json"
                file_path = os.path.join(sensor_folder, json_path)
                # print("json_path", json_path)
                # print("file_path", file_path)
                PlotDataPreparation.list_json_path.append(json_path)
                self._jsonpath_datastore_obj.update_data(sensor, file_path, "scatter")

                # Write JSON with UTF-8 encoding and Linux-friendly line endings
                with open(file_path, "w", encoding="utf-8", newline="\n") as f:
                    f.write(fig.to_json())
