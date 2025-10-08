from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Interfaces.Enums.Object import ObjectClass
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder


class ClosePedestriansEventFinder(BaseNexusEventFinder):
    def __init__(self, config: Nexus50kEvaluationConfig):
        super().__init__(config, 'close_pedestrians')

    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        host = extracted_data.host.signals
        auto_gt = extracted_data.auto_gt_lidar_cuboids.signals
        host_filtered = host.query('2 < raw_speed & raw_speed < 10')
        event_start_time, event_end_time = np.inf, -np.inf

        if len(host_filtered) > 0:
            host_filtered_ts = host_filtered.timestamp
            time_min, time_max = host_filtered_ts.iloc[0], host_filtered_ts.iloc[-1]

            pedestrians = auto_gt.loc[auto_gt.object_class == ObjectClass.PEDESTRIAN, :]
            pedestrians = pedestrians.query(f'{time_min} < timestamp & timestamp < {time_max}')
            pedestrians = pedestrians.query(f'speed > 1.0')

            pedestrians['range'] = np.hypot(pedestrians.position_x.to_numpy(), pedestrians.position_y.to_numpy())
            close_pedestrians = pedestrians.query('range < 20')
            event_start_time, event_end_time = close_pedestrians.timestamp.min(), close_pedestrians.timestamp.max()

        return event_start_time, event_end_time

