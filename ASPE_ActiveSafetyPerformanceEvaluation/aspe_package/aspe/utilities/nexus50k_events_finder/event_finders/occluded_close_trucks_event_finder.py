from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Interfaces.Enums.Object import ObjectClass
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder


class OccludedCloseTrucksEventFinder(BaseNexusEventFinder):
    def __init__(self, config: Nexus50kEvaluationConfig, max_visibility_rate: float = 0.5):
        self.max_visibility_rate = max_visibility_rate
        scenario_name = f'occluded_{max_visibility_rate}_close_trucks'
        super().__init__(config, scenario_name)

    def find_event(self, extracted_data: NexusExtractedData, nexus_id) -> Tuple[float, float]:
        auto_gt = extracted_data.auto_gt_lidar_cuboids.signals
        zone_x, zone_y = 20, 10

        event_start_time, event_end_time = np.inf, -np.inf

        trucks = auto_gt.loc[auto_gt.object_class == ObjectClass.TRUCK, :]
        trucks = trucks.query(f'{-zone_x} < position_x & position_x < {zone_x} & '
                              f'{-zone_y} < position_y & position_y < {zone_y} &'
                              f'velocity_otg_x > 20 &'
                              f'visibility_rate < {self.max_visibility_rate}')
        if len(trucks) > 0:
            event_start_time = trucks.timestamp.min()
            event_end_time = trucks.timestamp.max()

        return event_start_time, event_end_time
