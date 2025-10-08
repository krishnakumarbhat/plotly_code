# coding=utf-8
"""
Main Data container for metadata and datasets
"""

from aspe.extractors.Interfaces.IDataSet import IDataSet


class ExtractedData:
    """
    interface of extracted data
    """
    def __init__(self):
        self.metadata = None
        self.objects = None
        self.internal_objects = None
        self.host = None
        self.detections = None
        self.sensors = None
        self.trailer = None

    def get_data_sets_list(self):
        return [att for att in vars(self).values() if isinstance(att, IDataSet)]

    def transfer_cs(self, coordinate_sys: str, dist_rear_axle_to_front_bumper: float = None):
        if dist_rear_axle_to_front_bumper is None:
            if self.host is not None and self.host.dist_of_rear_axle_to_front_bumper is not None:
                dist_rear_axle_to_front_bumper = self.host.dist_of_rear_axle_to_front_bumper
            else:
                raise AttributeError('Cannot transform coordinate system - missing information about host vehicle rear '
                                     'axle to front bumper distance - ensure that host extraction is done correctly '
                                     'or use dist_rear_axle_to_front_bumper parameter')

        data_sets = self.get_data_sets_list()
        for data_set in data_sets:
            if data_set is not None:
                data_set.transfer_cs(coordinate_sys, dist_rear_axle_to_front_bumper)

    def interpolate_values(self, new_timestamp_vals, new_scan_index_vals, timestamp_signal_name='timestamp'):
        for data_set in self.get_data_sets_list():
            if len(data_set.signals) > 0:
                data_set.interpolate_values(new_timestamp_vals, new_scan_index_vals, timestamp_signal_name)