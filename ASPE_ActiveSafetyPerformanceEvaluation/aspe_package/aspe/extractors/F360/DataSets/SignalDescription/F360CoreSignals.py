import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

filter_type = SignalDescription(signature='filter_type',
                                dtype='category',
                                description='Motion model of object, see F360ObjectsFilterType',
                                unit='-')

tracker_id = SignalDescription(signature='tracker_id', # TODO change internal_id
                               dtype=np.int16,
                               description='Id of tracked object',
                               unit='-')

reduced_id = SignalDescription(signature='reduced_id',
                               dtype=np.int16,
                               description='Reduced (down-selected) Id of tracked object',
                               unit='-')

status = SignalDescription(signature='status',
                           dtype=np.int32,  # TODO change this to enum - DFT-1514
                           description='Status of tracked object',
                           unit='-')

confidence_level = SignalDescription(signature='confidence_level',
                                     dtype=np.float32,
                                     description='Confidence level of object existence',
                                     unit='-')

n_dets = SignalDescription(signature='n_dets',
                           dtype=np.int8,
                           description='Number of associated detections',
                           unit='-')

f_moving = SignalDescription(signature='f_moving',
                             dtype=np.bool_,
                             description='Indication if object is moving',
                             unit='-')

f_moveable = SignalDescription(signature='f_moveable',
                               dtype=np.bool_,
                               description='Indication if object is movable (moving or stopped)',
                               unit='-')

f360_object_class = SignalDescription(signature='f360_object_class',
                                      dtype=np.int32,  # TODO: change to enum - DFT-1514
                                      description='Object class using F360-defined enumerations',
                                      unit='-')

polynomial_p0 = SignalDescription(signature='polynomial_p0',
                                  dtype=np.float32,
                                  description='Zero degree coefficient of polynomial',
                                  unit='-')

polynomial_p1 = SignalDescription(signature='polynomial_p1',
                                  dtype=np.float32,
                                  description='First degree coefficient of polynomial',
                                  unit='-')

polynomial_p2 = SignalDescription(signature='polynomial_p2',
                                  dtype=np.float32,
                                  description='Second degree coefficient of polynomial',
                                  unit='-')

stat_env_type = SignalDescription(signature='stationary_env_type',
                                  dtype='category',
                                  description='Stationary environment representation type',
                                  unit='-')

x_max_limit = SignalDescription(signature='x_max_limit',
                                   dtype=np.float32,
                                   description='Maximum limit in X position coordinates')

x_min_limit = SignalDescription(signature='x_min_limit',
                                dtype=np.float32,
                                description='Minimum limit in X position coordinates')

reference_point = SignalDescription(signature='reference_point',
                                    dtype=np.uint8,
                                    description='Enumeration of which object point (corners, side midpoints or center) that is most likely seen from center of host',
                                    unit='F360_Reference_Point_T')