# coding=utf-8
"""
F360 Radar's Detections Data Set
"""
import numpy as np

from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

raw_det_id = SignalDescription(signature='raw_det_id',
                               dtype=np.int16,
                               description='Raw ID of detection (from sensor)',
                               unit='-')

assigned_obj_id = SignalDescription(signature='assigned_obj_id',
                                    dtype=np.int16,
                                    description='ID of assigned object',
                                    unit='-')

cluster_id = SignalDescription(signature='cluster_id',
                               dtype=np.int16,
                               description='ID of assigned cluster',
                               unit='-')

range_rate_comp = SignalDescription(signature='range_rate_comp',
                                    dtype=np.float32,
                                    description='range rate compensated by host motion',
                                    unit='m/s')

range_rate_dealiased = SignalDescription(signature='range_rate_dealiased',
                                         dtype=np.float32,
                                         description='de-aliased range rate (valid with f_dealiased flag)',
                                         unit='m/s')

f_dealiased = SignalDescription(signature='f_dealiased',
                                dtype=bool,
                                description='Flag indicating if detection is de-aliased',
                                unit='-')

motion_status = SignalDescription(signature='motion_status',
                                  dtype='category',
                                  description='Motion status of detection',
                                  unit='-')

wheel_spin_status = SignalDescription(signature='wheel_spin_status',
                                      dtype='category',
                                      description='Wheel spin status of detection',
                                      unit='-')


class F360RadarDetections(IRadarDetections):
    """
    F360 Radar Detections dataset class
    """
    def __init__(self):
        super().__init__()
        signal_names = [
            raw_det_id,
            assigned_obj_id,
            cluster_id,
            range_rate_comp,
            range_rate_dealiased,
            f_dealiased,
            motion_status,
            wheel_spin_status,
            ]
        self.update_signals_definition(signal_names)


if __name__ == '__main__':
    f360_dets = F360RadarDetections()
