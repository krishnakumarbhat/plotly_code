import numpy as np
import pandas as pd

from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.utilities.data_validity import check_if_data_set_exists_and_is_not_empty


class EnvDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        self.calc_dets_azimuth_vcs(data_model.extracted)
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):

        drawers = []

        occupancy_grid_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "occupancy_grid")

        if occupancy_grid_exists:
            drawers.append(DrawersFactory.create_occupancy_grid_drawer(self,
                                                                       self.data_model.extracted.occupancy_grid,
                                                                       "OccupancyGrid"))

        detection_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "detections")

        if detection_exists:
            drawers.append(
                DrawersFactory.create_detection_drawer(self, self.data_model.extracted.detections, "Detections",
                                                       "range_rate_comp", "azimuth_vcs"))

        objects_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "objects")

        if objects_exists:
            drawers.append(
                DrawersFactory.create_objects_drawer(self, self.data_model.extracted.objects, "ReducedObjects"))

        host_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "host")

        if host_exists:
            drawers.append(DrawersFactory.create_host_drawer(self, self.data_model.extracted.host, "Host"))

        sensors_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted, "sensors")

        if sensors_exists and host_exists:
            drawers.append(DrawersFactory.create_sensors_drawer(self, self.data_model.extracted.sensors, "Sensors"))

        stationary_geometric_exists = check_if_data_set_exists_and_is_not_empty(self.data_model.extracted,
                                                                                "stationary_geometries")

        if stationary_geometric_exists:
            drawers.append(
                DrawersFactory.create_stationary_geometric_drawer(self, self.data_model.extracted.stationary_geometries,
                                                                  "StationaryGeometries"))

        return drawers

    def append_legend_part(self, legend_df: pd.DataFrame, legend_dict: dict) -> pd.DataFrame:
        if self.is_enabled:
            log_name = self.data_model.name
            legend_dict["log_name"] = [log_name]
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df

    @staticmethod
    def calc_dets_azimuth_vcs(extracted_data):
        if extracted_data.detections is not None:
            dets = extracted_data.detections.signals
            if extracted_data.sensors is not None:
                sensors = extracted_data.sensors.per_sensor
                dets_w_sensors = dets.join(sensors.set_index("sensor_id"), on="sensor_id", rsuffix="_sensor")
                dets["azimuth_vcs"] = dets_w_sensors.azimuth + dets_w_sensors.boresight_az_angle
            else:
                dets["azimuth_vcs"] = np.arctan2(dets["position_y"], dets["position_x"])
