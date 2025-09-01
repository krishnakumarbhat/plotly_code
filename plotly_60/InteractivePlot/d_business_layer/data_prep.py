import importlib
from InteractivePlot.c_data_storage.regex_storage import Gen7V1_v2, sequence_of_plot
from collections import OrderedDict
from KPI.a_business_layer.detection_matching_kpi import KpiDataModel
from InteractivePlot.e_presentation_layer.html_generator import HtmlGenerator
from InteractivePlot.c_data_storage.data_model_storage import DataModelStorage
import multiprocessing as mp
from functools import lru_cache
import logging
import time
import gc


class DataPrep:
    """
    Prepares data for visualization by generating plots and handling plot data.
    Acts as a business layer between data storage and presentation.

    This class is responsible for:
    1. Retrieving data from storage layers (input and output)
    2. Processing and transforming data for visualization
    3. Generating appropriate plot types based on signal configuration
    4. Implementing multiprocessing for efficient data processing
    5. Handling KPI calculations.
    6. Passing visualization data to the presentation layer
    """

    def __init__(
        self,
        input_data,
        output_data,
        html_name,
        sensor,
        stream_name,
        input_file_name,
        output_file_name,
        output_dir=None,
        generate_html=True,
    ):
        """
        Initializes DataPrep with necessary parameters.

        Parameters:
            input_data: Input data storage containing signal data
            output_data: Output data storage containing processed signal data
            html_name: Name for the HTML file to be generated
            output_dir: Directory to save HTML reports (defaults to "html")
            stream_name: Type of data stream being processed (e.g., 'Radar', 'DETECTION_STREAM')
            generate_html: Whether to generate HTML file or just return plot data
        """

        self.input_data = input_data
        self.output_data = output_data

        self.html_name = html_name
        self.output_dir = output_dir or "html"
        self.sensor = sensor
        self.stream_name = stream_name  # Temporary storage for plot data

        # Initialize KPI plots for detection streams
        self.kpi_plots = {}
        if self.stream_name == "RDD_STREAM":
            self.kpi_plots = KpiDataModel(
                self.input_data,
                self.output_data,
            ).plots
        if self.stream_name != "RDD_STREAM":
            a = time.time()
            self.plots_hash = self.generate_plots()
            b = time.time()
            logging.debug(
                f"Time taken by generate plot figures for {b - a} in {self.stream_name}"
            )

            # Pass plots to HTML generator if requested
            if generate_html:
                HtmlGenerator(
                    self.plots_hash,
                    self.kpi_plots,
                    self.html_name,
                    self.output_dir,
                    input_file_name,
                    output_file_name,
                    sensor,
                )
            self.plots_hash = None  # to clean up the memory

    # @profile
    @lru_cache(maxsize=1024)
    def _get_data_cached(self, signal_name, keys_tuple):
        """
        Cached version of data retrieval to avoid redundant processing.
        Uses lru_cache for performance optimization when the same data
        is requested multiple times.

        Parameters:
        - signal_name: Name of the signal to get data for
        - keys_tuple: Tuple of unique keys for caching (immutable for cache)

        Returns:
        - Tuple data_dict from DataModelStorage.get_data or ('no_data_in_hdf', {}) if not found
        """

        return DataModelStorage.get_data(
            self.input_data,
            self.output_data,
            signal_name,
        )

    # @profile
    def _process_signal_plot(self, args):
        """
        Process a single signal and generate its plots.
        This method is designed to be used with multiprocessing for parallel execution.

        Parameters:
        - args: Tuple containing (signal_name, signal_config, data_cal, unique_keys_tuple, data_dict)
          * signal_name: Name of the signal to process
          * signal_config: Configuration for this signal from Gen7V1_v2
          * data_cal: DataCalculations instance for special calculations
          * unique_keys_tuple: Tuple of unique keys for caching
          * data_dict: Dictionary mapping scan indices to input and output rows

        Returns:
        - Dictionary mapping plot_type to figure objects
        """
        signal_name, signal_config, data_cal, unique_keys_tuple, shared_data, lock = (
            args
        )
        plot_data_dict = {}

        # Get the complete signal name from the 'call' field if available
        complete_signal_name = signal_name
        if "call" in signal_config and len(signal_config["call"]) > 0:
            complete_signal_name = signal_config["call"][0]

        # Get data records for this signal using cached method
        data_dict = self._get_data_cached(signal_name, unique_keys_tuple)

        # Try aliases if no data found (fallback mechanism)
        if isinstance(data_dict, tuple) and len(data_dict) > 0:
            if data_dict[0] == "no_data_in_hdf" and "aliases" in signal_config:
                for alias in signal_config["aliases"]:
                    data_dict, alias_data_dict = self._get_data_cached(
                        alias, unique_keys_tuple
                    )
                    if data_dict != "no_data_in_hdf":
                        # If we got valid data from an alias, use its data_dict if available
                        if alias_data_dict:
                            data_dict = alias_data_dict
                        break

        if (
            data_dict
            and data_dict != "no_data_in_hdf"
            and not isinstance(data_dict, tuple)
        ):
            # Process each plot type for this signal
            for plot_type in signal_config.get("plot_types", []):
                func_name = plot_type
                # Check if the function exists in data_cal.py
                if data_cal and hasattr(data_cal, func_name):
                    # Call the function with complete signal name,data_dict
                    fig_id, figure = getattr(data_cal, func_name)(
                        complete_signal_name, data_dict, shared_data, lock
                    )
                    if figure:  # Only add if figure was created successfully
                        plot_key = f"{complete_signal_name}:{plot_type}"
                        plot_data_dict[plot_key] = figure
                else:
                    # Route function not found messages to logs file
                    logging.debug(f"Function '{func_name}' not found in data_cal.py")

        return plot_data_dict

    # @profile
    def generate_plots(self):
        """
        Generates HTML content for plots by handling data and delegating plot creation.

        This method:
        1. Identifies unique keys from both input and output data
        2. Prepares data dictionary for processing
        3. Processes signals in parallel with multiprocessing
        4. Organizes plots by stream type in a dictionary following the sequence

        Returns:
        - plots_hash: A dictionary containing plots organized by stream type
        """
        plots_hash = {}
        signal_plot_dict = {}  # {streamname: {plotname: signalname, ...}}

        # Get all unique keys from both data containers (deduplication)
        unique_keys = list(
            OrderedDict.fromkeys(
                list(self.input_data._data_container.keys())
                + list(self.output_data._data_container.keys())
            )
        )
        unique_keys_tuple = tuple(unique_keys)  # Convert to tuple for caching

        # Import the data calculation module for specialized calculations
        try:
            data_cal_module = importlib.import_module(
                "InteractivePlot.d_business_layer.data_cal"
            )
            data_cal = data_cal_module.DataCalculations()
            # Set the stream name for the data calculations (context)
            data_cal.set_stream_name(self.stream_name)
        except (ImportError, AttributeError) as e:
            print(
                f"Warning: Could not import DataCalculations class from data_cal module: {str(e)}"
            )
            data_cal = None

        # Prepare arguments for multiprocessing (one job per signal)
        mp_args = []
        lock = mp.Manager().Lock()
        shared_data = mp.Manager().dict()
        for signal_name, signal_config in Gen7V1_v2.items():
            mp_args.append(
                (
                    signal_name,
                    signal_config,
                    data_cal,
                    unique_keys_tuple,
                    shared_data,
                    lock,
                )
            )

            # Create mapping entries for signal plot dictionary
            if self.stream_name not in signal_plot_dict:
                signal_plot_dict[self.stream_name] = {}

            # Get complete signal name from config
            complete_signal_name = signal_name
            if "call" in signal_config and len(signal_config["call"]) > 0:
                complete_signal_name = signal_config["call"][0]

            # Add mapping for each plot type
            for plot_type in signal_config.get("plot_types", []):
                plot_key = f"{complete_signal_name}:{plot_type}"
                signal_plot_dict[self.stream_name][plot_key] = signal_name

        # Determine the optimal number of processes to use
        num_processors = mp.cpu_count() - 2

        signal_plot_data = {}

        a = time.time()
        if num_processors > 3:
            try:
                with mp.Pool(
                    processes=max(2, int(num_processors // 1.25)),
                    maxtasksperchild=150,
                    initializer=gc.disable,
                ) as pool:
                    results = pool.map(self._process_signal_plot, mp_args)

                # Combine all dictionaries from results
                for result_dict in results:
                    signal_plot_data.update(result_dict)
            except Exception as e:
                print(
                    f"Error in multiprocessing: {str(e)}. Falling back to sequential processing."
                )
                # Fall back to sequential processing if multiprocessing fails
                for args in mp_args:
                    result_dict = self._process_signal_plot(args)
                    signal_plot_data.update(result_dict)
        else:
            # Sequential processing for small datasets or when multiprocessing is not beneficial
            print(
                "Using sequential processing for plot generation as number of free available cores in pc is less"
            )
            for args in mp_args:
                result_dict = self._process_signal_plot(args)
                signal_plot_data.update(result_dict)

        self._process_post_multiprocessing_plots(
            data_cal, signal_plot_data, shared_data, lock
        )

        b = time.time()
        logging.debug(
            f"Time taken by generate plot figures for {b - a} in {self.stream_name}"
        )

        # Initialize the plots_hash
        plots_hash[self.stream_name] = []

        handled_plots = set()

        for category, plot_keys in sequence_of_plot.items():
            for plot_key in plot_keys:
                if plot_key in signal_plot_data:
                    # Use the category to determine the key
                    key = f"{self.stream_name}_{category}"
                    plots_hash.setdefault(key, []).append(signal_plot_data[plot_key])
                    handled_plots.add(plot_key)

        # Add any plots not in the sequence (to ensure we don't miss any)
        for plot_key, figure in signal_plot_data.items():
            if plot_key not in handled_plots:
                plots_hash.setdefault(self.stream_name, []).append(figure)

        return plots_hash

    def _process_post_multiprocessing_plots(
        self, data_cal, signal_plot_data, shared_data, lock
    ):
        """
        Process plots that need to run after all other plots to collect and aggregate data.
        These plots typically need shared data from multiple signals.

        Parameters:
        - data_cal: DataCalculations instance
        - signal_plot_data: Dictionary to update with new plots
        """
        if not data_cal:
            return

        # Generate the mismatch summary plot
        fig_id, fig = data_cal.bar_mismatch_plots_all(
            "DONE_ALL_SIGNALS", None, shared_data, lock
        )
        if fig:
            plot_key = "all_signal:bar_mismatch_plots_all"
            signal_plot_data[plot_key] = fig

        # Generate the FBI support signals summary plot
        fig_id, fig = data_cal.bar_plots_fbi_sup_sig(
            "DONE_FBI_SUP_SIG", None, shared_data, lock
        )
        if fig:
            plot_key = "fbi_sup_sig:bar_plots_fbi_sup_sig"
            signal_plot_data[plot_key] = fig
