import numpy as np
import pandas as pd

from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification.BasicBinaryClassifier import BCType
from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.utilities.data_validity import check_if_data_set_exists_and_is_not_empty


class DrawerAspeResults(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        super().__init__(name, drawers_controller, data_model)

        self.data_model = self.join_pe_data(self.data_model)

    def create_drawers(self):
        drawers = []

        if check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "extracted_estimated_objects"):
            drawers.append(DrawersFactory.create_objects_drawer(self,
                                                                self.data_model.extracted.extracted_estimated_objects,
                                                                "Estimated"))

        if check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "extracted_reference_objects"):
            drawers.append(DrawersFactory.create_objects_drawer(self,
                                                                self.data_model.extracted.extracted_reference_objects,
                                                                "Reference"))

        if check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "extracted_reference_host"):
            drawers.append(DrawersFactory.create_host_drawer(self,
                                                             self.data_model.extracted.extracted_reference_host,
                                                             "Host"))

        if check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "extracted_radar_sensors"):
            drawers.append(DrawersFactory.create_sensors_drawer(self,
                                                                self.data_model.extracted.extracted_radar_sensors,
                                                                "Sensors"))

        dets_are_available = hasattr(self.data_model.extracted, "extracted_radar_detections") and \
                             self.data_model.extracted.extracted_radar_detections is not None
        if dets_are_available:
            calc_dets_azimuth_vcs(self.data_model.extracted)
            drawers.append(
                DrawersFactory.create_detection_drawer(self, self.data_model.extracted.extracted_radar_detections,
                                                       "Detections",
                                                       "range_rate", "azimuth_vcs"))

        associated_pairs_data = self.prepare_associations_data()
        drawers.append(DrawersFactory.create_est_to_ref_association_drawer(self, associated_pairs_data, "Associations"))

        not_relevant_objs = self.prepare_relevancy_data()
        drawers.append(DrawersFactory.create_not_relevant_objs_drawer(self, not_relevant_objs, "NotRelObjects"))

        bin_class_data = self.prepare_bc_data()
        drawers.append(DrawersFactory.create_binary_classification_drawer(self, bin_class_data, "BinaryClassification"))
        return drawers

    def prepare_associations_data(self):
        pe_out = self.data_model.extracted
        ref_ds = self.data_model.extracted.extracted_reference_objects  # just for shorter lines :)
        est_ds = self.data_model.extracted.extracted_estimated_objects

        pairs = pe_out.pe_results_obj_pairs.signals
        pairs_ref_idx = pairs["index_ref"]
        pairs_est_idx = pairs["index_est"]

        center_signals = ["center_x", "center_y"]
        center_ref = ref_ds.signals.loc[pairs_ref_idx, center_signals].rename(
            columns={s: f"{s}_ref" for s in center_signals}).reset_index(drop=True)
        center_est = est_ds.signals.loc[pairs_est_idx, center_signals].rename(
            columns={s: f"{s}_est" for s in center_signals}).reset_index(drop=True)

        return pd.concat([pairs, center_ref, center_est], axis=1)

    def prepare_relevancy_data(self):
        pe_out = self.data_model.extracted
        ref_ds = self.data_model.extracted.extracted_reference_objects  # just for shorter lines :)
        est_ds = self.data_model.extracted.extracted_estimated_objects
        rel_signals = ["scan_index", "center_x", "center_y", "bounding_box_dimensions_x", "bounding_box_dimensions_y",
                       "bounding_box_orientation"]
        not_rel_ref_mask = ~pe_out.pe_results_obj_ref.signals.relevancy_flag.to_numpy().astype(bool)
        not_rel_est_mask = ~pe_out.pe_results_obj_est.signals.relevancy_flag.to_numpy().astype(bool)

        not_rel_ref = ref_ds.signals.loc[not_rel_ref_mask, rel_signals]
        not_rel_est = est_ds.signals.loc[not_rel_est_mask, rel_signals]
        return pd.concat([not_rel_est, not_rel_ref]).reset_index(drop=True)

    def prepare_bc_data(self):
        ref_ds = self.data_model.extracted.extracted_reference_objects  # just for shorter lines :)
        est_ds = self.data_model.extracted.extracted_estimated_objects
        pe_out = self.data_model.extracted

        center_signals = ["center_x", "center_y"]
        est_bc = pd.concat([est_ds.signals.loc[:, center_signals],
                            pe_out.pe_results_obj_est.signals.loc[:, ["scan_index", "binary_classification"]]], axis=1)
        ref_bc = pd.concat([ref_ds.signals.loc[:, center_signals],
                            pe_out.pe_results_obj_ref.signals.loc[:, ["scan_index", "binary_classification"]]], axis=1)
        bin_class_data = pd.concat([ref_bc, est_bc]).reset_index(drop=True)
        bin_class_data = bin_class_data.loc[bin_class_data.binary_classification != BCType.NotAssigned, :]
        return bin_class_data

    @staticmethod
    def join_pe_data(data_model):
        if check_if_data_set_exists_and_is_not_empty(data_model.extracted, "extracted_estimated_objects") and \
                check_if_data_set_exists_and_is_not_empty(data_model.extracted, "pe_results_obj_est"):
            # Concatenating extracted data and pe_results if possible
            data_model.extracted.extracted_estimated_objects.signals = pd.concat(
                [data_model.extracted.extracted_estimated_objects.signals,
                 data_model.extracted.pe_results_obj_est.signals], axis=1)
            duplicated_columns = data_model.extracted.extracted_estimated_objects.signals.columns.duplicated()
            # Deleting duplicate columns
            data_model.extracted.extracted_estimated_objects.signals = \
                data_model.extracted.extracted_estimated_objects.signals.loc[:, ~duplicated_columns]
        if check_if_data_set_exists_and_is_not_empty(data_model.extracted, "extracted_reference_objects") and \
                check_if_data_set_exists_and_is_not_empty(data_model.extracted, "pe_results_obj_ref"):
            # Concatenating extracted data and pe_results if possible
            data_model.extracted.extracted_reference_objects.signals = pd.concat(
                [data_model.extracted.extracted_reference_objects.signals,
                 data_model.extracted.pe_results_obj_ref.signals], axis=1)
            duplicated_columns = data_model.extracted.extracted_reference_objects.signals.columns.duplicated()
            # Deleting duplicate columns
            data_model.extracted.extracted_reference_objects.signals = \
                data_model.extracted.extracted_reference_objects.signals.loc[:, ~duplicated_columns]
        return data_model


def calc_dets_azimuth_vcs(extracted_data):
    dets = extracted_data.extracted_radar_detections.signals
    if extracted_data.extracted_radar_sensors is not None:
        sensors = extracted_data.extracted_radar_sensors.per_sensor
        dets_w_sensors = dets.join(sensors.set_index("sensor_id"), on="sensor_id", rsuffix="_sensor")
        dets["azimuth_vcs"] = dets_w_sensors.azimuth + dets_w_sensors.boresight_az_angle
    else:
        dets["azimuth_vcs"] = np.arctan2(dets["position_y"], dets["position_x"])
