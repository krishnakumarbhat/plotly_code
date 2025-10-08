from aspe.gui.drawers.DrawerBinaryClassification import DrawerBinaryClassification
from aspe.gui.drawers.DrawerDetections import DrawerDetections
from aspe.gui.drawers.DrawerDetectionsClusters import DrawerDetectionsClusters
from aspe.gui.drawers.DrawerDetToClustersAssociation import DrawerDetToClustersAssociation
from aspe.gui.drawers.DrawerDetToObjAssociation import DrawerDetToObjAssociation
from aspe.gui.drawers.DrawerHost import DrawerHost
from aspe.gui.drawers.DrawerInternalObjects import DrawerInternalObjects
from aspe.gui.drawers.DrawerObjects import DrawerObjects
from aspe.gui.drawers.DrawerOccupancyGrid import DrawerOccupancyGrid
from aspe.gui.drawers.DrawerRefToEstAssociation import DrawerRefToEstAssociation
from aspe.gui.drawers.DrawerRelevancy import DrawerRelevancy
from aspe.gui.drawers.DrawerSensors import DrawerSensors
from aspe.gui.drawers.DrawerStaticEnvironment import DrawerStaticEnvironment
from aspe.gui.drawers.DrawerStationaryGeometries import DrawerStationaryGeometries
from aspe.gui.drawers.DrawerTrailer import DrawerTrailer
from aspe.gui.drawers.DrawerXtrkObjects import DrawerXtrkObjects


class DrawersFactory:
    birds_eye_view = None

    @staticmethod
    def create_objects_drawer(data_source, extracted_objects, name):
        return DrawerObjects(data_source, extracted_objects, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_internal_objects_drawer(data_source, extracted_objects, name):
        return DrawerInternalObjects(data_source, extracted_objects, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_xtrk_objects_drawer(data_source, extracted_objects, name):
        return DrawerXtrkObjects(data_source, extracted_objects, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_detection_drawer(data_source, extracted_detections, name, rrate_signal_name, azimuth_signal_name):
        return DrawerDetections(data_source, extracted_detections, DrawersFactory.birds_eye_view, name,
                                rrate_signal_name, azimuth_signal_name)

    @staticmethod
    def create_host_drawer(data_source, extracted_host, name):
        return DrawerHost(data_source, extracted_host, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_trailer_drawer(data_source, extracted_trailer, name):
        return DrawerTrailer(data_source, extracted_trailer, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_sensors_drawer(parent_drawer, extracted_sensors, name):
        return DrawerSensors(parent_drawer, extracted_sensors, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_det_to_obj_association_drawer(parent_drawer, associations, name):
        return DrawerDetToObjAssociation(parent_drawer, name, DrawersFactory.birds_eye_view, associations)

    @staticmethod
    def create_est_to_ref_association_drawer(parent_drawer, associations, name):
        return DrawerRefToEstAssociation(parent_drawer, name, DrawersFactory.birds_eye_view, associations)

    @staticmethod
    def create_not_relevant_objs_drawer(parent_drawer, not_relevant_objs, name):
        return DrawerRelevancy(parent_drawer, name, DrawersFactory.birds_eye_view, not_relevant_objs)

    @staticmethod
    def create_binary_classification_drawer(parent_drawer, binary_classification, name):
        return DrawerBinaryClassification(parent_drawer, binary_classification, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_cluster_drawer(data_source, extracted_clusters, name, rrate_signal_name, azimuth_signal_name):
        return DrawerDetectionsClusters(data_source,
                                        extracted_clusters,
                                        DrawersFactory.birds_eye_view,
                                        name,
                                        rrate_signal_name,
                                        azimuth_signal_name)

    @staticmethod
    def create_det_to_clusters_association_drawer(parent_drawer, associations, name):
        return DrawerDetToClustersAssociation(parent_drawer, name, DrawersFactory.birds_eye_view, associations)

    @staticmethod
    def create_stat_env_drawer(parent_drawer, stat_env, name):
        return DrawerStaticEnvironment(parent_drawer, stat_env, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_occupancy_grid_drawer(parent_drawer, occupancy_grid, name):
        return DrawerOccupancyGrid(parent_drawer, occupancy_grid, DrawersFactory.birds_eye_view, name)

    @staticmethod
    def create_stationary_geometric_drawer(parent_drawer, stationary_geometries, name):
        return DrawerStationaryGeometries(parent_drawer, stationary_geometries, DrawersFactory.birds_eye_view, name)
