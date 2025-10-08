import numpy as np

from aspe.extractors.Interfaces.IRadarDetections import IRadarDetections
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

# TODO FZE-400 consider if its needed, seems to be duplicate of F360RadarDetections

raw_det_id = SignalDescription(signature='raw_det_id',
                               dtype=np.int16,
                               description='Raw ID of detection (from sensor)',
                               unit='-')

f_bistatic = SignalDescription(signature='f_bistatic',
                               dtype=bool,
                               description='Flag indicating detections seems to be a multipath detection with '
                                           'direction of arrival different from direction of departure',
                               unit='-')

f_host_veh_clutter = SignalDescription(signature='f_host_veh_clutter',
                                       dtype=bool,
                                       description='Flag indicating the detection is host vehicle clutter and '
                                                   'additional processing should be done with',
                                       unit='-')

f_nd_target = SignalDescription(signature='f_nd_target',
                                dtype=bool,
                                description='Flag indicating if detection is a near target discrimination',
                                unit='-')

f_super_res = SignalDescription(signature='f_super_res',
                                dtype=bool,
                                description='Flag indicating that the detection is derived using super resolution '
                                            'algorithms',
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


class ENVDetections(IRadarDetections):
    def __init__(self):
        super().__init__()
        signal_names = [
            raw_det_id,
            f_nd_target,
            f_host_veh_clutter,
            f_super_res,
            f_bistatic,
            assigned_obj_id,
            cluster_id,
            range_rate_comp,
            range_rate_dealiased,
            motion_status,
            wheel_spin_status,
        ]
        self.update_signals_definition(signal_names)
