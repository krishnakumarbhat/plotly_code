from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Interfaces.Enums.Object import ObjectClass
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder


class CloseTrucksEventFinder(BaseNexusEventFinder):
    def __init__(self, config: Nexus50kEvaluationConfig):
        super().__init__(config, 'close_trucks')

    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        auto_gt = extracted_data.auto_gt_lidar_cuboids.signals
        zone_x, zone_y = 20, 10

        trucks = auto_gt.loc[auto_gt.object_class == ObjectClass.TRUCK, :]
        trucks = trucks.query(f'{-zone_x} < position_x & position_x < {zone_x} & '
                              f'{-zone_y} < position_y & position_y < {zone_y} &'
                              f'velocity_otg_x > 20')
        return trucks.timestamp.min(), trucks.timestamp.max()
