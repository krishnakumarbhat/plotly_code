import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

assoc_gates_center_x = SignalDescription(signature='assoc_gates_center_x',
                                         dtype=np.float32,
                                         description='X coordinate of detection-to-object association gates center',
                                         unit='m')

assoc_gates_center_y = SignalDescription(signature='assoc_gates_center_y',
                                         dtype=np.float32,
                                         description='Y coordinate of detection-to-object association gates center',
                                         unit='m')

assoc_gates_length = SignalDescription(signature='assoc_gates_length',
                                       dtype=np.float32,
                                       description='Longitudinal dimension of detection-to-object association gates',
                                       unit='m')

assoc_gates_width = SignalDescription(signature='assoc_gates_width',
                                      dtype=np.float32,
                                      description='Lateral dimension of detection-to-object association gates',
                                      unit='m')

assoc_gates_radius = SignalDescription(signature='assoc_gates_radius',
                                       dtype=np.float32,
                                       description='Radius dimension of detection-to-object association gates ('
                                                   'non-movable)',
                                       unit='m')

predicted_position_x = SignalDescription(signature='predicted_position_x',
                                         dtype=np.float32,
                                         description='Predicted longitudinal position of tracked object calculated in '
                                                     'Kalman filter time update step',
                                         unit='m')

predicted_position_y = SignalDescription(signature='predicted_position_y',
                                         dtype=np.float32,
                                         description='Predicted lateral position of tracked object calculated in '
                                                     'Kalman filter time update step',
                                         unit='m')

predicted_velocity_otg_x = SignalDescription(signature='predicted_velocity_x',
                                             dtype=np.float32,
                                             description='Over-The-Ground predicted velocity in X coordinate calculated'
                                                         'in Kalman filter time update step',
                                             unit='m/s')

predicted_velocity_otg_y = SignalDescription(signature='predicted_velocity_y',
                                             dtype=np.float32,
                                             description='Over-The-Ground predicted velocity in Y coordinate calculated'
                                                         'in Kalman filter time update step',
                                             unit='m/s')

predicted_orientation = SignalDescription(signature='predicted_bounding_box_orientation',
                                          dtype=np.float32,
                                          description='Predicted orientation of bounding box(called also yaw or '
                                                      'pointing angle) calculated in Kalman filter time update step',
                                          unit='rad')

pseudo_position_x = SignalDescription(signature='pseudo_position_x',
                                      dtype=np.float32,
                                      description='Representation of position X measurement used in Kalman filter '
                                                  'measurement update step',
                                      unit='m')

pseudo_position_y = SignalDescription(signature='pseudo_position_y',
                                      dtype=np.float32,
                                      description='Representation of position Y measurement used in Kalman filter '
                                                  'measurement update step',
                                      unit='m')

reference_point = SignalDescription(signature='reference_point',
                                    dtype=np.uint8,
                                    description='Enumeration of which object point (corners, side midpoints or '
                                                'center) that is most likely seen from center of host',
                                    unit='F360_Reference_Point_T')
