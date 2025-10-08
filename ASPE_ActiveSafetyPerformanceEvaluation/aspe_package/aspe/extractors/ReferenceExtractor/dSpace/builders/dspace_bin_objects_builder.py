from warnings import warn

import numpy as np
import pandas as pd

from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Interfaces.IBuilder import IBuilder
from aspe.extractors.ReferenceExtractor.dSpace.datasets.dspace_objects import (
    DspaceObjects,
)


class DspaceBinObjectsBuilder(IBuilder):
    """Builder class for dSpace objects parsed from bin file.

    The implementation assumes that the data is parsed from a bin file output for the REAR_LEFT sensor. All sensors
    output their own bin file but only the rear left sensor has actual data. This is pretty weird and it might change in
    the future.

    :param parsed_data: data parsed from a container.
    :param f_extract_raw_signals: flag indicating if raw signals should be extracted. No translation is applied to raw
    signals. If yes extracted raw signals are placed in separate DataFrame within the extracted data. Defaults to False
    :param allowed_length_relative_deviation: since object classification is not provided by the logged data it is
    inferred using the length of the object. This parameter controls how much the length is allowed to deviate from it's
    expected value for each class. If `l` is the length of the object and `L` is the expected length, then the relative
    deviation is:
        rel_dev = |L - l| / L
    An object is only classified if its `rel_dev` is below `allowed_length_relative_deviation` for a given class.
    Defaults to 0.2
    """

    signal_mapper = {
        # dSpace bin signature from REAR_LEFT: ASPE signature
        "OSIGTInput_RL_vcs_long_posn": "center_x",
        "OSIGTInput_RL_vcs_lat_posn": "center_y",
        "OSIGTInput_RL_vcs_long_vel": "velocity_otg_x",
        "OSIGTInput_RL_vcs_lat_vel": "velocity_otg_y",
        "OSIGTInput_RL_vcs_long_vel_rel": "velocity_rel_x",
        "OSIGTInput_RL_vcs_lat_vel_rel": "velocity_rel_y",
        "OSIGTInput_RL_vcs_long_accel": "acceleration_otg_x",
        "OSIGTInput_RL_vcs_lat_accel": "acceleration_otg_y",
        "OSIGTInput_RL_length": "bounding_box_dimensions_x",
        "OSIGTInput_RL_width": "bounding_box_dimensions_y",
        "OSIGTInput_RL_heading": "bounding_box_orientation",
        "OSIGTInput_RL_speed": "speed",
    }

    def __init__(
        self,
        parsed_data,
        f_extract_raw_signals: bool = False,
        allowed_length_relative_deviation: float = 0.2,
    ):
        super().__init__(parsed_data)
        self.data_set: DspaceObjects = DspaceObjects()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._allowed_length_relative_deviation = (
            allowed_length_relative_deviation
        )

    def build(self) -> DspaceObjects:
        """Extracts objects from parsed data.

        :return: an instance of DspaceObjects with extracted signals.
        """
        self._create_lifespan_mask()
        if self._f_extract_raw_signals:
            self._extract_raw_signals()

        self._extract_mappable_signals()
        self._extract_non_mappable_signals()

        self.data_set.signals.reset_index(drop=True, inplace=True)

        return self.data_set

    def _create_lifespan_mask(self):
        """Creates a mask for object extraction.

        Masking assumptions:
        - Objects have length larger than 0
        - The unique ID of each object is the same as the column number. Consequentely, a column can never hold more
        than one object. Based on existing logs this assumption holds, however, existing logs are very simple scenarios
        with low number of objects. Verify this assumption in more complex scenarios with many objects.
        """
        # Assume that objects have length larger than 0
        rows, columns = np.nonzero(self._parsed_data["parsed_data"]["OSIGTInput_RL_length"] > 0)
        # Assume that each column can only hold one object
        unique_id = columns

        self._lifespan_mask = pd.DataFrame({"row": rows, "column": columns, "unique_id": unique_id})

    def _extract_raw_signals(self):
        rows = self._lifespan_mask["row"]
        columns = self._lifespan_mask["column"]
        unique_id = self._lifespan_mask["unique_id"]

        self.data_set.raw_signals = pd.DataFrame()
        self.data_set.raw_signals["unique_id"] = unique_id
        self.data_set.raw_signals["row"] = rows
        self.data_set.raw_signals["column"] = columns

        for signal_signature, array in self._parsed_data[
            "parsed_data"
        ].items():
            if len(array.shape) == 1:
                self.data_set.raw_signals[signal_signature] = array[rows]
            elif len(array.shape) == 2:
                self.data_set.raw_signals[signal_signature] = array[
                    rows, columns
                ]

    def _extract_mappable_signals(self):
        rows = self._lifespan_mask["row"]
        columns = self._lifespan_mask["column"]
        unique_id = self._lifespan_mask["unique_id"]

        self.data_set.signals["unique_id"] = unique_id
        self.data_set.signals["row"] = rows
        self.data_set.signals["column"] = columns

        for dspace_signature, aspe_signature in self.signal_mapper.items():
            if dspace_signature not in self._parsed_data["parsed_data"].keys():
                warn(
                    f"Object list extraction warning! There is no signal {dspace_signature} inside parsed signals "
                    f"DataFrame. Signal {aspe_signature} will be not filled."
                )
                continue
            if aspe_signature not in self.data_set.signals.columns:
                warn(
                    f"Object list extraction warning! There is no signal {aspe_signature} inside extracted signals "
                    f"DataFrame. Signal {aspe_signature} will be not filled."
                )
                continue

            # Fill object dataset if both signals exist in their respective dataframes
            dspace_arr = self._parsed_data["parsed_data"][dspace_signature]
            self.data_set.signals[aspe_signature] = dspace_arr[rows, columns]

    def _extract_non_mappable_signals(self):
        self.data_set.coordinate_system = "VCS"
        self._extract_bbox_signals()
        self._extract_classification()
        self._extract_movement_status()
        self._work_around_for_missing_gt_yaw_rate()
        self._extract_scan_index()
        self._extract_timestamp()

    def _extract_bbox_signals(self):
        """
        Calculates reference point and offset ratios for groundtruth bounding
        box
        """
        # The reference point of groundtruth bounding box(VCS origin) is in
        # the center --> reference point = center position
        ref_x = np.array([0.5] * self.data_set.signals.shape[0])
        ref_y = np.array([0.5] * self.data_set.signals.shape[0])

        self.data_set.signals["bounding_box_refpoint_long_offset_ratio"] = ref_x
        self.data_set.signals["bounding_box_refpoint_lat_offset_ratio"] = ref_y

        self.data_set.signals["position_x"] = self.data_set.signals["center_x"]
        self.data_set.signals["position_y"] = self.data_set.signals["center_y"]

    def _extract_classification(self):
        # dSpace data doesn't provide object classification but we can make an educated guess based on the object length
        length_to_class_map = {
            # dSpace object length: ASPE class
            0.54: ObjectClass.PEDESTRIAN,
            2.00: ObjectClass.TWOWHEEL,  # Bicycle
            2.20: ObjectClass.TWOWHEEL,  # Motorcycle
            4.36: ObjectClass.CAR,
        }

        # Infer the object class based on the object length using the mapping. Anything that is not on the mapping is
        # classified as unknown
        objects_length = self.data_set.signals["bounding_box_dimensions_x"].to_numpy()
        objects_class = np.full(objects_length.shape, ObjectClass.UNKNOWN)
        for len_threshold, classification in length_to_class_map.items():
            objects_class = np.where(
                (np.abs(len_threshold - objects_length) / len_threshold)
                < self._allowed_length_relative_deviation,
                classification,
                objects_class,
            )

        self.data_set.signals["object_class"] = objects_class

    def _extract_movement_status(self):

        objects_speed = self.data_set.signals["speed"].to_numpy()

        movement_status = np.full(objects_speed.shape, MovementStatus.UNDEFINED)

        movement_status = np.where(
            objects_speed > 0.0,
            MovementStatus.MOVING,
            MovementStatus.STATIONARY,
        )

        self.data_set.signals["movement_status"] = movement_status

    def _work_around_for_missing_gt_yaw_rate(self):

        self.data_set.signals["yaw_rate"] = np.full(self.data_set.signals["yaw_rate"].to_numpy().shape, 0.0)

    def _extract_scan_index(self):
        scan_index_arr = self._parsed_data["parsed_data"]["scan_index"]
        rows = self._lifespan_mask["row"]
        self.data_set.signals["scan_index"] = scan_index_arr[rows]

    def _extract_timestamp(self):
        timestamp = self._parsed_data["parsed_data"]["log_timestamp"]
        timestamp_arr = timestamp * 1e-3
        rows = self._lifespan_mask["row"]
        self.data_set.signals["timestamp"] = timestamp_arr[rows]
