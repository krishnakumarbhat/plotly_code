import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

position_x = SignalDescription(signature='position_x',
                               dtype=np.float32,
                               description='Position in x coordinate. '
                                           'In case of position of BB then it is position of reference point'
                                           'See bounding_box_refpoint_long_offset_ratio (and lat)',
                               unit='m')

position_y = SignalDescription(signature='position_y',
                               dtype=np.float32,
                               description='Position in y coordinate. '
                                           'In case of position of BB then it is position of reference point'
                                           'See bounding_box_refpoint_long_offset_ratio (and lat)',
                               unit='m')

position_z = SignalDescription(signature='position_z',
                               dtype=np.float32,
                               description='Position in z coordinate. '
                                           'In case of position of BB then it is position of reference point'
                                           'See bounding_box_refpoint_height_offset_ratio',
                               unit='m')

position_variance_x = SignalDescription(signature='position_variance_x',
                                        dtype=np.float32,
                                        description='Position variance in x coordinate. '
                                                    'In case of position of BB then it is for position of '
                                                    'reference point',
                                        unit='m^2')

position_variance_y = SignalDescription(signature='position_variance_y',
                                        dtype=np.float32,
                                        description='Position variance in y coordinate. '
                                                    'In case of position of BB then it is for position of '
                                                    'reference point',
                                        unit='m^2')

position_covariance = SignalDescription(signature='position_covariance',
                                        dtype=np.float32,
                                        description='Position covariance between x and y coordinate. '
                                                    'In case of position of BB then it is for position of '
                                                    'reference point',
                                        unit='m^2')

velocity_otg_x = SignalDescription(signature='velocity_otg_x',
                                   dtype=np.float32,
                                   description='Over-The-Ground velocity in x coordinate. '
                                               'In case of position of BB then it is velocity in position of '
                                               'reference point',
                                   unit='m/s')

velocity_otg_y = SignalDescription(signature='velocity_otg_y',
                                   dtype=np.float32,
                                   description='Over-The-Ground velocity in y coordinate. '
                                               'In case of position of BB then it is velocity in position of '
                                               'reference point',
                                   unit='m/s')

velocity_otg_z = SignalDescription(signature='velocity_otg_z',
                                   dtype=np.float32,
                                   description='Over-The-Ground velocity in z coordinate. '
                                               'In case of position of BB then it is velocity in position of '
                                               'reference point',
                                   unit='m/s')

velocity_rel_x = SignalDescription(signature='velocity_rel_x',
                                   dtype=np.float32,
                                   description='Relative velocity in x coordinate. '
                                               'In case of position of BB then it is velocity in position of '
                                               'reference point',
                                   unit='m/s')

velocity_rel_y = SignalDescription(signature='velocity_rel_y',
                                   dtype=np.float32,
                                   description='Relative velocity in y coordinate. '
                                               'In case of position of BB then it is velocity in position of '
                                               'reference point',
                                   unit='m/s')

velocity_otg_variance_x = SignalDescription(signature='velocity_otg_variance_x',
                                            dtype=np.float32,
                                            description='Over-The-Ground velocity variance in x coordinate. '
                                                        'In case of position of BB then it is velocity variance in '
                                                        'position of reference point',
                                            unit='m^2/s^2')

velocity_otg_variance_y = SignalDescription(signature='velocity_otg_variance_y',
                                            dtype=np.float32,
                                            description='Over-The-Ground velocity variance in y coordinate. '
                                                        'In case of position of BB then it is velocity variance in '
                                                        'position of reference point',
                                            unit='m^2/s^2')

velocity_otg_covariance = SignalDescription(signature='velocity_otg_covariance',
                                            dtype=np.float32,
                                            description='Over-The-Ground velocity covariance between x and y '
                                                        'coordinate. '
                                                        'In case of position of BB then it is velocity covariance in '
                                                        'position of reference point',
                                            unit='m^2/s^2')

acceleration_otg_x = SignalDescription(signature='acceleration_otg_x',
                                       dtype=np.float32,
                                       description='Over-The-Ground acceleration in x coordinate'
                                                   'In case of position of BB then it is acceleration in position of '
                                                   'reference point',
                                       unit='m/s^2')

acceleration_otg_y = SignalDescription(signature='acceleration_otg_y',
                                       dtype=np.float32,
                                       description='Over-The-Ground acceleration in y coordinate. '
                                                   'In case of position of BB then it is acceleration in position of '
                                                   'reference point',
                                       unit='m/s^2')

acceleration_rel_x = SignalDescription(signature='acceleration_rel_x',
                                       dtype=np.float32,
                                       description='Relative acceleration in x coordinate. '
                                                   'In case of position of BB then it is acceleration in position of '
                                                   'reference point',
                                       unit='m/s^2')

acceleration_rel_y = SignalDescription(signature='acceleration_rel_y',
                                       dtype=np.float32,
                                       description='Relative acceleration in y coordinate. '
                                                   'In case of position of BB then it is acceleration in position of '
                                                   'reference point',
                                       unit='m/s^2')

acceleration_otg_variance_x = SignalDescription(signature='acceleration_otg_variance_x',
                                                dtype=np.float32,
                                                description='Over-The-Ground acceleration variance in x coordinate. '
                                                            'In case of position of BB then it is acceleration '
                                                            'variance in position of reference point',
                                                unit='m^2/s^4')

acceleration_otg_variance_y = SignalDescription(signature='acceleration_otg_variance_y',
                                                dtype=np.float32,
                                                description='Over-The-Ground acceleration variance in y coordinate. '
                                                            'In case of position of BB then it is acceleration '
                                                            'variance in position of reference point',
                                                unit='m^2/s^4')

acceleration_otg_covariance = SignalDescription(signature='acceleration_otg_covariance',
                                                dtype=np.float32,
                                                description='Over-The-Ground acceleration covariance between x and y '
                                                            'coordinate. '
                                                            'In case of position of BB then it is acceleration '
                                                            'covariance in position of reference point',
                                                unit='m^2/s^4')

bounding_box_dimensions_x = SignalDescription(signature='bounding_box_dimensions_x',
                                              dtype=np.float32,
                                              description='Dimension in x coordinate - length',
                                              unit='m')

bounding_box_dimensions_y = SignalDescription(signature='bounding_box_dimensions_y',
                                              dtype=np.float32,
                                              description='Dimension in y coordinate - width',
                                              unit='m')

bounding_box_dimensions_z = SignalDescription(signature='bounding_box_dimensions_z',
                                              dtype=np.float32,
                                              description='Dimension in z coordinate - height',
                                              unit='m')

bounding_box_orientation = SignalDescription(signature='bounding_box_orientation',
                                             dtype=np.float32,
                                             description='Orientation of bounding box - called also yaw or '
                                                         'pointing angle',
                                             unit='rad')

bounding_box_orientation_variance = SignalDescription(signature='bounding_box_orientation_variance',
                                                      dtype=np.float32,
                                                      description='Variance of orientation of bounding box',
                                                      unit='rad^2')

bounding_box_refpoint_long_offset_ratio = SignalDescription(signature='bounding_box_refpoint_long_offset_ratio',
                                                            dtype=np.float32,
                                                            description='Ratio of distance from rear side to reference '
                                                                        'point and bounding box length. '
                                                                        'It defines where reference point is located '
                                                                        'within bounding box. '
                                                                        'In range [0, 1]',
                                                            unit='-')

bounding_box_refpoint_lat_offset_ratio = SignalDescription(signature='bounding_box_refpoint_lat_offset_ratio',
                                                           dtype=np.float32,
                                                           description='Ratio of distance from left (or right - '
                                                                       'depending of coordinate system) side to '
                                                                       'reference point and bounding box width. '
                                                                       'It defines where reference point is located '
                                                                       'within bounding box. '
                                                                       'In range [0, 1]',
                                                           unit='-')

bounding_box_refpoint_height_offset_ratio = SignalDescription(signature='bounding_box_refpoint_height_offset_ratio',
                                                              dtype=np.float32,
                                                              description='Ratio of distance from button (or top - '
                                                                          'depending of coordinate system) side to '
                                                                          'reference point and bounding box height. '
                                                                          'It defines where reference point is located '
                                                                          'within bounding box. '
                                                                          'In range [0, 1]',
                                                              unit='-')

center_x = SignalDescription(signature='center_x',
                             dtype=np.float32,
                             description='Position in x coordinate of center of given entity',
                             unit='m')

center_y = SignalDescription(signature='center_y',
                             dtype=np.float32,
                             description='Position in y coordinate of center of given entity',
                             unit='m')

center_z = SignalDescription(signature='center_z',
                             dtype=np.float32,
                             description='Position in z coordinate of center of given entity',
                             unit='m')

speed = SignalDescription(signature='speed',
                          dtype=np.float32,
                          description='Over-The-Ground velocity magnitude',
                          unit='m/s')

yaw_rate = SignalDescription(signature='yaw_rate',
                             dtype=np.float32,
                             description='yaw rate of given entity',
                             unit='rad/s')

yaw_rate_variance = SignalDescription(signature='yaw_rate_variance',
                                      dtype=np.float32,
                                      description='yaw rate variance of given entity',
                                      unit='rad^2/^2')

yaw_acceleration = SignalDescription(signature='yaw_acceleration',
                                     dtype=np.float32,
                                     description='yaw acceleration of given entity',
                                     unit='rad/s^2')

yaw_acceleration_variance = SignalDescription(signature='yaw_acceleration_variance',
                                              dtype=np.float32,
                                              description='yaw acceleration variance of given entity',
                                              unit='rad^2/^4')

curvature = SignalDescription(signature='curvature',
                              dtype=np.float32,
                              description='Curvature of object state',
                              unit='1/m')

range = SignalDescription(signature='range',
                          dtype=np.float32,
                          description='range, distance of physical entity',
                          unit='m')

range_variance = SignalDescription(signature='range_variance',
                                   dtype=np.float32,
                                   description='variance of range, distance of physical entity',
                                   unit='m^2')

azimuth = SignalDescription(signature='azimuth',
                            dtype=np.float32,
                            description='azimuth of position of physical entity',
                            unit='rad')

azimuth_vcs_aligned = SignalDescription(signature='azimuth_vcs_aligned',
                                        dtype=np.float32,
                                        description='VCS aligned azimuth of position of physical entity',
                                        unit='rad')

azimuth_variance = SignalDescription(signature='azimuth_variance',
                                     dtype=np.float32,
                                     description='variance of azimuth of position of physical entity',
                                     unit='rad^2')

elevation = SignalDescription(signature='elevation',
                              dtype=np.float32,
                              description='elevation of position of physical entity',
                              unit='rad')

elevation_variance = SignalDescription(signature='elevation_variance',
                                       dtype=np.float32,
                                       description='variance of azimuth of position of physical entity',
                                       unit='rad^2')

range_rate = SignalDescription(signature='range_rate',
                               dtype=np.float32,
                               description='range rate of physical entity',
                               unit='m/s')

range_rate_variance = SignalDescription(signature='range_rate_variance',
                                        dtype=np.float32,
                                        description='variance of range rate of physical entity',
                                        unit='m^2/s^2')

amplitude = SignalDescription(signature='amplitude',
                              dtype=np.float32,
                              description='amplitude of physical entity',
                              unit='dB')

visibility_rate = SignalDescription(signature='visibility_rate',
                                    dtype=np.float32,
                                    description='Rate of how much part of object is visible by radar given in 0 - 1 '
                                                'range. Based on advEngWup algo which was run on Nexus',
                                    unit='-')
