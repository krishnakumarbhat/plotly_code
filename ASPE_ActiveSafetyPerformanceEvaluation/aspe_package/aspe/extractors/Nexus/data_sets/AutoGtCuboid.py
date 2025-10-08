from typing import Optional

from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription import GeneralSignals, PhysicalSignals


class AutoGTCuboid(IDataSet):
    """
    Representation of auto labeling ground-truth algorithm output.
    """

    def __init__(self):
        super().__init__()
        signal_names = [
            GeneralSignals.id,
            PhysicalSignals.position_x,
            PhysicalSignals.position_y,
            PhysicalSignals.position_z,
            PhysicalSignals.center_x,
            PhysicalSignals.center_y,
            PhysicalSignals.velocity_otg_x,
            PhysicalSignals.velocity_otg_y,
            PhysicalSignals.velocity_rel_x,
            PhysicalSignals.velocity_rel_y,
            PhysicalSignals.speed,
            PhysicalSignals.bounding_box_dimensions_x,
            PhysicalSignals.bounding_box_dimensions_y,
            PhysicalSignals.bounding_box_orientation,
            PhysicalSignals.bounding_box_refpoint_lat_offset_ratio,
            PhysicalSignals.bounding_box_refpoint_long_offset_ratio,
            GeneralSignals.movement_status,
            GeneralSignals.object_class,
            GeneralSignals.existence_indicator,
            PhysicalSignals.visibility_rate,
        ]
        self.update_signals_definition(signal_names)
        self.angular_signals = [PhysicalSignals.bounding_box_orientation.signature]
        self.coordinate_system: Optional[str] = None

    def _transfer_to_vcs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'ISO_VRACS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'velocity_rel_y'] = -self.signals.loc[:, 'velocity_rel_y']
            self.signals.loc[:, 'bounding_box_orientation'] = -self.signals.loc[:, 'bounding_box_orientation']
            self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = \
                1.0 - self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio']
            self.coordinate_system = 'VCS'

        if self.coordinate_system == 'PANCS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] - dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] - dist_rear_axle_to_front_bumper
            self.coordinate_system = 'VCS'

    def _transfer_to_iso_vracs(self, dist_rear_axle_to_front_bumper):
        if self.coordinate_system == 'VCS':
            self.signals.loc[:, 'position_x'] = self.signals.loc[:, 'position_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'center_x'] = self.signals.loc[:, 'center_x'] + dist_rear_axle_to_front_bumper
            self.signals.loc[:, 'position_y'] = -self.signals.loc[:, 'position_y']
            self.signals.loc[:, 'center_y'] = -self.signals.loc[:, 'center_y']
            self.signals.loc[:, 'velocity_otg_y'] = -self.signals.loc[:, 'velocity_otg_y']
            self.signals.loc[:, 'yaw_rate'] = -self.signals.loc[:, 'yaw_rate']
            self.signals.loc[:, 'bounding_box_orientation'] = -self.signals.loc[:, 'bounding_box_orientation']
            self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio'] = \
                1.0 - self.signals.loc[:, 'bounding_box_refpoint_lat_offset_ratio']
            self.coordinate_system = 'ISO_VRACS'

    def get_base_name(self):
        """
        Get objects base name
        :return: objects base name
        """
        return 'auto_gt_cuboids'


