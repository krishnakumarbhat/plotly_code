from typing import Tuple

import numpy as np

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.extractors.Interfaces.Enums.Object import MovementStatus, ObjectClass
from aspe.extractors.Nexus.NexusExtractedData import NexusExtractedData
from aspe.utilities.nexus50k_events_finder.event_finders.base_event_finder import BaseNexusEventFinder


class OccludedCTAScenariosEventFinder(BaseNexusEventFinder):
    def __init__(self, config: Nexus50kEvaluationConfig, max_visibility_rate: float = 0.5):
        self.max_visibility_rate = max_visibility_rate
        scenario_name = f'occluded_{max_visibility_rate}_cta_scenarios'
        super().__init__(config, scenario_name)

    def find_event(self, extracted_data: NexusExtractedData) -> Tuple[float, float]:
        host = extracted_data.host.signals
        auto_gt = extracted_data.auto_gt_lidar_cuboids.signals
        max_allowed_host_speed = 5
        host_filtered = host.loc[abs(host.raw_speed) <= max_allowed_host_speed]
        event_start_time, event_end_time = np.inf, -np.inf
        highest_allowed_min_position_y = -10
        lowest_allowed_max_position_y = 10
        lowest_allowed_max_object_speed = 5
        min_num_objects = 1
        expected_abs_bbox_orientation = np.deg2rad(90.0)
        max_allowed_abs_bbox_orientation_difference = np.deg2rad(45.0)
        max_abs_position_y = 100
        max_abs_position_x = 60

        if len(host_filtered) > 0:
            host_filtered_ts = host_filtered.timestamp
            time_min, time_max = host_filtered_ts.iloc[0], host_filtered_ts.iloc[-1]
            is_moving = auto_gt.movement_status == MovementStatus.MOVING
            is_x_within_limit = abs(auto_gt.position_x) <= max_abs_position_x
            is_y_within_limit = abs(auto_gt.position_y) <= max_abs_position_y
            obj_orientation_difference = abs(
                auto_gt.bounding_box_orientation) - expected_abs_bbox_orientation
            is_moving_perpendicular_to_host = abs(
                obj_orientation_difference) <= max_allowed_abs_bbox_orientation_difference
            is_vehicle = (auto_gt.object_class == ObjectClass.CAR) | (auto_gt.object_class == ObjectClass.TRUCK)
            cta_objects = auto_gt.loc[
                is_moving_perpendicular_to_host & is_moving & is_vehicle & is_x_within_limit & is_y_within_limit]
            cta_objects = cta_objects.query(f'{time_min} < timestamp & timestamp < {time_max} & visibility_rate < {self.max_visibility_rate}')

            ids_correct = []
            for id, item in cta_objects.groupby('unique_id'):
                if (item.position_y.min() < highest_allowed_min_position_y) and (
                        item.position_y.max() > lowest_allowed_max_position_y) and (item.speed.max() > lowest_allowed_max_object_speed):
                    ids_correct.append(id)

            cta_objects = cta_objects[cta_objects.unique_id.isin(ids_correct) == True]
            if ((len(cta_objects)) > 0) and (len(cta_objects.unique_id.unique()) >= min_num_objects):
                event_start_time, event_end_time = cta_objects.timestamp.min(), cta_objects.timestamp.max()

        return event_start_time, event_end_time
