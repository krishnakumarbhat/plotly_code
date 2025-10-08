import numpy as np

from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.utilities.data_validity import check_if_data_set_exists_and_is_not_empty


class F360XtrkDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        calc_dets_azimuth_vcs(data_model.extracted)
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):
        drawers = []

        detection_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "detections")
        objects_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "internal_objects")
        clusters_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "clusters")
        sensors_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "sensors")
        host_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "host")

        if host_exists:
            drawers.append(DrawersFactory.create_host_drawer(self, self.data_model.extracted.host, "Host"))

        if sensors_exists and host_exists:
            drawers.append(DrawersFactory.create_sensors_drawer(self, self.data_model.extracted.sensors, "Sensors"))

        if objects_exists and detection_exists:
            det_to_objs = get_det_associated_to_objs(self.data_model)
            drawers.append(DrawersFactory.create_det_to_obj_association_drawer(self, det_to_objs, "DetToObjAssoc"))

        if clusters_exists and detection_exists:
            det_to_clusters = get_det_associated_to_clusters(self.data_model)
            drawers.append(
                DrawersFactory.create_det_to_clusters_association_drawer(
                    self,
                    det_to_clusters,
                    "DetToClustersAssoc"))

        if objects_exists:
            drawers.append(
                DrawersFactory.create_xtrk_objects_drawer(
                    self,
                    self.data_model.extracted.internal_objects,
                    "AllObjects"))

        if detection_exists:
            drawers.append(
                DrawersFactory.create_detection_drawer(
                    self,
                    self.data_model.extracted.detections,
                    "Detections",
                    "range_rate_comp",
                    "azimuth_vcs"))

        if clusters_exists:
            drawers.append(
                DrawersFactory.create_cluster_drawer(
                    self,
                    self.data_model.extracted.clusters,
                    "Clusters",
                    "range_rate",
                    "azimuth_vcs_aligned"))

        if check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "static_environment"):
            drawers.append(
                DrawersFactory.create_stat_env_drawer(
                    self,
                    self.data_model.extracted.static_environment,
                    "StatEnv"))
        return drawers

    def append_legend_part(self, legend_df, legend_dict):
        if self.is_enabled:
            log_name = self.data_model.name
            legend_dict["log_name"] = log_name
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df

    def switch_to_reduced(self):
        self._get_drawer_by_name("DetToObjAssoc").switch_to_reduced()

    def switch_to_all_data(self):
        self._get_drawer_by_name("DetToObjAssoc").switch_to_all_assoc()


def get_det_associated_to_objs(data_model):
    det_signals = \
        data_model.extracted.detections.signals.set_index(
            ["scan_index", "assigned_obj_id"]).loc[:, ["position_x", "position_y"]]
    obj_signals = \
        data_model.extracted.internal_objects.signals.set_index(
            ["scan_index", "id"]).loc[:, ["position_x", "position_y", "reduced_id"]]

    det_signals.index.names = [None, None]
    obj_signals.index.names = [None, None]

    return obj_signals.join(det_signals, how="inner", lsuffix="_obj", rsuffix="_det")


def get_det_associated_to_clusters(data_model):
    det_signals = \
        data_model.extracted.detections.signals.set_index(
            ["scan_index", "cluster_id"]).loc[:, ["position_x", "position_y"]]
    cluster_signals = \
        data_model.extracted.clusters.signals.set_index(
            ["scan_index", "tracker_id"]).loc[:, ["position_x", "position_y"]]

    det_signals.index.names = [None, None]
    cluster_signals.index.names = [None, None]

    return cluster_signals.join(det_signals, how="inner", lsuffix="_cluster", rsuffix="_det")


def calc_dets_azimuth_vcs(extracted_data):
    if extracted_data.detections is not None:
        dets = extracted_data.detections.signals
        if extracted_data.sensors is not None:
            sensors = extracted_data.sensors.per_sensor
            dets_w_sensors = dets.join(sensors.set_index("sensor_id"), on="sensor_id", rsuffix="_sensor")
            dets["azimuth_vcs"] = dets_w_sensors.azimuth + dets_w_sensors.boresight_az_angle
        else:
            dets["azimuth_vcs"] = np.arctan2(dets["position_y"], dets["position_x"])
