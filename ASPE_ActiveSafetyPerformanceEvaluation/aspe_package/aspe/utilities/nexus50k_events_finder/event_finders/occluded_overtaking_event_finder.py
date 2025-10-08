from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder


class OccludedOvertakingEventFinder(BaseNexusEventFinder):

    def __init__(self, config: Nexus50kEvaluationConfig, max_visibility_rate=0.5):
        self.max_visibility_rate = max_visibility_rate
        scenario_name = f'occluded_{max_visibility_rate}_overtaking'
        super().__init__(config, scenario_name)

    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        host = extracted_data.host.signals
        auto_gt = extracted_data.auto_gt_lidar_cuboids.signals
        host_filtered = host.query('raw_speed > 15')
        host_filtered_ts = host_filtered.timestamp
        event_start_time, event_end_time = np.inf, -np.inf

        if len(host_filtered_ts) > 0:
            min_ts, max_ts = host_filtered_ts.min(), host_filtered_ts.max()

            filtering_query = f'{min_ts} < timestamp & timestamp < {max_ts} & velocity_rel_x > 2 & visibility_rate < {self.max_visibility_rate}'
            overtaking = auto_gt.query(filtering_query)

            for unq_id, obj in overtaking.groupby(by='unique_id'):
                if np.any(obj.position_x > 0) and np.any(obj.position_x < 0):
                    event_start_time = min(event_start_time, obj.timestamp.to_numpy()[0])
                    event_end_time = max(event_end_time, obj.timestamp.to_numpy()[-1])

        return event_start_time, event_end_time
