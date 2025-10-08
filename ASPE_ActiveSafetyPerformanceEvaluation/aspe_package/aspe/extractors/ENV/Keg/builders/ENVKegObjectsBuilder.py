import numpy as np
import pandas as pd

from aspe.extractors.ENV.DataSets.ENVObjects import ENVObjects
from aspe.extractors.ENV.Keg.builders.ENVKegBuilder import ENVKegBuilder
from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.utilities.objects_lifespan_status_based_creator import (
    ObjectLifespanStatusBasedCreator,
)
from aspe.utilities.SupportingFunctions import map_array_values


class ENVKegObjectsBuilder(ENVKegBuilder):
    signal_mapper = {
        # ENV bin signature: ASPE signature
        "distinct_id": "id",
        "vcs_long_posn": "center_x",
        "vcs_lat_posn": "center_y",
        "ref_long_posn": "position_x",
        "ref_lat_posn": "position_y",
        "ref_long_vel": "velocity_otg_x",
        "ref_lat_vel": "velocity_otg_y",
        "vcs_long_vel_rel": "velocity_rel_x",
        "vcs_lat_vel_rel": "velocity_rel_y",
        "ref_long_accel": "acceleration_otg_x",
        "ref_lat_accel": "acceleration_otg_y",
        "heading_rate": "yaw_rate",
        "length": "bounding_box_dimensions_x",
        "width": "bounding_box_dimensions_y",
        "heading": "bounding_box_orientation",
        "existence_probability": "existence_indicator",
        "variance_vel_lon_filt": "velocity_otg_variance_x",
        "variance_vel_lat_filt": "velocity_otg_variance_y",
        "variance_pos_lon_filt": "position_variance_x",
        "variance_pos_lat_filt": "position_variance_y",
        "variance_covariances_velocities_filtered": "velocity_otg_covariance",
        "variance_covariances_positions_filtered": "position_covariance",
        "speed": "speed",
        # signals which were taken from lifespan mask
        "unique_id": "unique_id",
    }

    def __init__(self, parsed_data, timestamps, f_extract_raw_signals=False):
        super().__init__(parsed_data)
        self.data_set: ENVObjects = ENVObjects()
        self._f_extract_raw_signals = f_extract_raw_signals
        self._timestamps = timestamps

    def build(self) -> ENVObjects:
        """Extracts objects from parsed data.

        :return: an instance of envObjects with extracted signals.
        """
        self._create_lifespan_mask()
        self._extract_raw_signals()
        self._extract_mappable_signals()
        self._extract_non_mappable_signals()

        if self._f_extract_raw_signals:
            self.data_set.raw_signals = self._raw_signals.copy()
            self.data_set.raw_signals.reset_index(drop=True, inplace=True)

        self.data_set.signals.reset_index(drop=True, inplace=True)

        return self.data_set

    def _create_lifespan_mask(self):
        status_array = self._parsed_data["OBJECTS"]["parsed_data"]["status"].copy()
        status_array_extended = np.vstack(
            [
                np.zeros(shape=(1, status_array.shape[1]), dtype=int),
                status_array,
            ],
        )
        status_diff = np.diff(status_array_extended, axis=0)
        obj_was_born_mask = status_diff == 2
        new_enum_for_new_obj_status = 1
        status_array[obj_was_born_mask] = new_enum_for_new_obj_status

        self._lifespan_mask = ObjectLifespanStatusBasedCreator.create_lifespan(
            status_array,
            obj_status_new_val=new_enum_for_new_obj_status,
            obj_status_invalid_val=0,
        )

    def _extract_raw_signals(self):
        rows = self._lifespan_mask["row"]
        columns = self._lifespan_mask["column"]
        unique_id = self._lifespan_mask["unique_obj_id"]

        self._raw_signals["unique_id"] = unique_id
        self._raw_signals["row"] = rows
        self._raw_signals["column"] = columns

        num_objects = self._parsed_data["OBJECTS"]["parsed_data"]["distinct_id"].shape[-1]

        for signal_signature, array in self._parsed_data["OBJECTS"]["parsed_data"].items():
            if len(array.shape) == 1:
                self._raw_signals[signal_signature] = array[rows]
            elif len(array.shape) == 2 and array.shape[-1] == num_objects:
                self._raw_signals[signal_signature] = array[rows, columns]

    def _extract_non_mappable_signals(self):
        self.data_set.coordinate_system = "VCS"
        self._extract_bbox_signals()
        self._extract_classification()
        self._extract_movement_status()
        self._extract_scan_index()
        self._extract_timestamp()

    def _extract_bbox_signals(self):
        ref_point_map = {
            # Ref. point: (long offset ratio, lat offset ratio)
            0: (1.0, 0.0),  # front left
            1: (1.0, 0.5),  # middle front
            2: (1.0, 1.0),  # front right
            3: (0.5, 1.0),  # side right
            4: (0.0, 1.0),  # rear right
            5: (0.0, 0.5),  # middle rear
            6: (0.0, 0.0),  # rear left
            7: (0.5, 0.0),  # middle left
        }
        # Convert reference point enum to longitude and latitude offsets
        ref_points = self._raw_signals["ref_position"].to_numpy().astype(int)
        ref_x = np.zeros(self._raw_signals.shape[0])
        ref_y = np.zeros(self._raw_signals.shape[0])
        for corner, (val_x, val_y) in ref_point_map.items():
            ref_x = np.where(ref_points == corner, val_x, ref_x)
            ref_y = np.where(ref_points == corner, val_y, ref_y)

        self.data_set.signals[
            "bounding_box_refpoint_long_offset_ratio"
        ] = ref_x
        self.data_set.signals["bounding_box_refpoint_lat_offset_ratio"] = ref_y

    def _extract_classification(self):
        # Convert classification
        class_map = {
            # env class: ASPE class
            0: ObjectClass.UNKNOWN,
            1: ObjectClass.PEDESTRIAN,
            2: ObjectClass.TWOWHEEL,
            3: ObjectClass.CAR,
            4: ObjectClass.TRUCK,
        }
        classification_raw = self._raw_signals["object_class"].to_numpy()
        self.data_set.signals["object_class"] = map_array_values(
            classification_raw,
            class_map,
            ObjectClass.UNDEFINED,
        )

        # Convert classification probability
        class_to_class_prob_signal_map = {
            # env class: env class probability signal name
            0: "probability_unknown",
            1: "probability_pedestrian",
            2: "probability_2wheel",
            3: "probability_car",
            4: "probability_truck",
        }
        for (
            env_class,
            class_prob_signal,
        ) in class_to_class_prob_signal_map.items():
            classification_prob = self._raw_signals[
                class_prob_signal
            ].to_numpy()
            self.data_set.signals["object_class_probability"] = np.where(
                classification_raw == env_class,
                classification_prob,
                self.data_set.signals["object_class_probability"],
            )

    def _extract_movement_status(self):
        movement_status_raw = self._raw_signals["f_stationary"].to_numpy()
        mapper = {
            1: MovementStatus.STATIONARY,
            0: MovementStatus.MOVING,
        }
        self.data_set.signals["movement_status"] = map_array_values(
            movement_status_raw,
            mapper,
            MovementStatus.UNDEFINED,
        )

    def _extract_scan_index(self):
        scan_index_arr = self._parsed_data["OBJECTS"]["parsed_data"]["scan_index"]
        rows = self._lifespan_mask["row"]
        self.data_set.signals["scan_index"] = scan_index_arr[rows]

    def _extract_timestamp(self):
        timestamp = self._parsed_data["OBJECTS"]["parsed_data"]["log_timestamp"]
        timestamp_arr = timestamp * 1e-3
        rows = self._lifespan_mask["row"]
        self.data_set.signals["timestamp"] = timestamp_arr[rows]
