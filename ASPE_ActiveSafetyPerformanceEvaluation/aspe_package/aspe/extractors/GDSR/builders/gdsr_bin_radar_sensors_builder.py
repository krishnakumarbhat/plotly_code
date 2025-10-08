from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.GDSR.builders.enums.gdsr_sensor import (
    GdsrMountingLocation,
    GdsrRadarSensorType,
)
from aspe.extractors.GDSR.datasets.gdsr_radar_sensors import GdsrRadarSensors
from aspe.extractors.Interfaces.IBuilder import IBuilder


class GdsrBinRadarSensorsBuilder(IBuilder):
    # Mapping of radar sensor signals between ASPE and GDSR. This is only concerned with signals that can be directly
    # mapped (no conversion needed)
    signal_mapper = {
        # ASPE signature: GDSR bin signature
        "unique_id": "id",
        "scan_index": "scan_index",
        "sensor_id": "id",
        "look_id": "look_id",
        "sensor_valid": "f_active",
        "velocity_otg_x": "long_vel",
        "velocity_otg_y": "lat_vel",
    }

    # Mapping of radar sensor properties between ASPE and GDSR. This is only concerned with signals that can be
    # directly mapped (no conversion needed)
    per_sensor_signal_mapper = {
        # ASPE signature: GDSR bin signature
        "sensor_id": "id",
        "polarity": "azimuth_polarity",
        "boresight_az_angle": "boresight_angle",
        "position_x": "long_posn",
        "position_y": "lat_posn",
    }

    # Mapping of radar sensor look configuration between ASPE and GDSR. This is only concerned with signals that can be
    # directly mapped (no conversion needed)
    per_look_signal_mapper = {
        # ASPE signature: GDSR bin signature
        "sensor_id": "id",
        "look_id": "look_id",
        "max_range": "max_range_current_look",
        "range_rate_interval_width": "v_un",
    }

    def __init__(self, parsed_data, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self.data_set = GdsrRadarSensors()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._raw_signals = pd.DataFrame()

    def build(self) -> GdsrRadarSensors:
        """Extracts radar sensors from parsed data.

        :return: an instance of GdsrRadarSensors with extracted signals.
        """
        self.data_set.coordinate_system = "VCS"
        self._extract_raw_signals()
        self._extract_radar_sensor_signals()
        self._extract_properties_per_sensor()
        self._extract_properties_per_look()

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
            self.data_set.raw_signals.reset_index(drop=True, inplace=True)

        self.data_set.signals.reset_index(drop=True, inplace=True)
        self.data_set.per_sensor.reset_index(drop=True, inplace=True)
        self.data_set.per_look.reset_index(drop=True, inplace=True)

        return self.data_set

    def _extract_raw_signals(self):
        """Extract raw GDSR radar sensor signals into a dataframe.

        Each row of the dataframe corresponds to one scan index and one sensor.
        """
        for signal_signature, array in self._parsed_data[
            "parsed_data"
        ].items():
            if array.ndim == 1:
                self._raw_signals[signal_signature] = array
            elif array.ndim == 2:
                self._raw_signals[signal_signature] = list(array)

    def _extract_radar_sensor_signals(self):
        self._extract_mappable_signals(self._raw_signals, self.signal_mapper, self.data_set.signals)
        self._extract_timestamp()
        self._extract_sensor_velocity_otg_variance()

    def _extract_properties_per_sensor(self):
        raw_sensors_df = self._raw_signals.drop_duplicates(subset="id", keep="first")
        self._extract_mappable_signals(
            raw_sensors_df,
            self.per_sensor_signal_mapper,
            self.data_set.per_sensor,
        )

        # Mounting location
        self.data_set.per_sensor["mount_location"] = raw_sensors_df[
            "mount_loc"
        ].map(lambda x: GdsrMountingLocation(x))

        # Sensor type is unknown because the log file doesn't contain such information
        self.data_set.per_sensor["sensor_type"].values[
            :
        ] = GdsrRadarSensorType.UNKNOWN

    def _extract_properties_per_look(self):
        raw_looks_df = self._raw_signals.drop_duplicates(subset=["id", "look_id"], keep="first")
        self._extract_mappable_signals(raw_looks_df, self.per_look_signal_mapper, self.data_set.per_look)
        self._extract_fov_azimuth(raw_looks_df)

    def _extract_mappable_signals(self, df_gdsr, signal_mapper, df_aspe):
        for aspe_signature, gdsr_signature in signal_mapper.items():
            # Check if the signals exist in their respective dataframes. If they do copy from gdsr to aspe; otherwise,
            # raise a warning and don't copy the data
            if gdsr_signature not in df_gdsr.columns:
                warn(
                    f"Object list extraction warning! There is no signal {gdsr_signature} inside raw signals "
                    f"DataFrame. Signal {aspe_signature} will be not filled."
                )
                continue
            if aspe_signature not in df_aspe.columns:
                warn(
                    f"Object list extraction warning! There is no signal {aspe_signature} inside extracted signals "
                    f"DataFrame. Signal {aspe_signature} will be not filled."
                )
                continue

            df_aspe[aspe_signature] = df_gdsr[gdsr_signature]

    def _extract_timestamp(self):
        # Use `log_timestamp` instead of `time_stamp` for consistency with tracker output extractor
        timestamp = self._raw_signals["log_timestamp"]
        timestamp_arr = timestamp * 1e-3
        self.data_set.signals["timestamp"] = timestamp_arr

    def _extract_sensor_velocity_otg_variance(self):
        # Workaround for extraction of sensor velocity variance
        # TO DO: Write this signal to binary output and map to ASPE signature
        self.data_set.signals["velocity_otg_variance_x"] = 0.0
        self.data_set.signals["velocity_otg_variance_y"] = 0.0
        self.data_set.signals["velocity_otg_covariance"] = 0.0

    def _extract_fov_azimuth(self, raw_looks_df):
        # Extract minimum and maximum field of view azimuth
        min_az_nx = raw_looks_df["fov_minaz_nx"].to_numpy()
        min_az_ny = raw_looks_df["fov_minaz_ny"].to_numpy()
        min_az_n = np.arctan2(min_az_ny, min_az_nx)

        max_az_nx = raw_looks_df["fov_maxaz_nx"].to_numpy()
        max_az_ny = raw_looks_df["fov_maxaz_ny"].to_numpy()
        # Sum Pi to the maximum azimuth angle because the normal vector, after being calculated, is reversed to point
        # the boresight
        max_az_n = np.arctan2(max_az_ny, max_az_nx) + np.pi

        fov_half_angles = 0.5 * (max_az_n - min_az_n)
        self.data_set.per_look["max_azimuth"] = fov_half_angles
        self.data_set.per_look["min_azimuth"] = -fov_half_angles
