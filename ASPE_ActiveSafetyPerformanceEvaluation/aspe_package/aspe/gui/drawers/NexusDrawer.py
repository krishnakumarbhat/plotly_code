from aspe.gui.drawers.abstract_drawers.DrawerExtractedData import DrawerExtractedData
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.utilities.data_validity import check_if_data_set_exists_and_is_not_empty


class NexusDrawer(DrawerExtractedData):
    def __init__(self, name, drawers_controller, data_model):
        super().__init__(name, drawers_controller, data_model)

    def create_drawers(self):
        drawers = []

        auto_gt_lidar_cuboids_exists = check_if_data_set_exists_and_is_not_empty(
            self.data_model.extracted,
            "auto_gt_lidar_cuboids")
        detection_exists = check_if_data_set_exists_and_is_not_empty(
            self.data_model.extracted,
            "detections")
        host_exists = check_if_data_set_exists_and_is_not_empty(
            self.data_model.extracted,
            "host")
        objects_exists = check_if_data_set_exists_and_is_not_empty(
            self.data_model.extracted,
            "objects")

        if auto_gt_lidar_cuboids_exists:
            drawers.append(DrawersFactory.create_objects_drawer(self,
                                                                self.data_model.extracted.auto_gt_lidar_cuboids,
                                                                "AutoGtCuboids"))

        if detection_exists:
            drawers.append(DrawersFactory.create_detection_drawer(self, self.data_model.extracted.detections,
                                                                  "Detections",
                                                                  "range_rate",
                                                                  "azimuth_vcs"))

        if host_exists:
            drawers.append(DrawersFactory.create_host_drawer(self,
                                                             self.data_model.extracted.host,
                                                             "Host"))

        if objects_exists:
            drawers.append(DrawersFactory.create_objects_drawer(self,
                                                                self.data_model.extracted.objects,
                                                                "Objects"))

        return drawers

    def append_legend_part(self, legend_df, legend_dict):
        if self.is_enabled:
            log_name = self.data_model.name
            legend_dict["log_name"] = log_name
            for drawer in self.drawers:
                legend_df = drawer.append_legend_part(legend_df, legend_dict)
        return legend_df