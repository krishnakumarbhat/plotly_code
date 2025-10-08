import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

scan_index = SignalDescription(signature='scan_index',
                               dtype=np.int32,
                               description='Unique identifier for data coming from the same package of data. '
                                           'Several records within DataSet can have same scan_index. '
                                           'Each DataSet within single DataSetContainer has the same scan base '
                                           '(identified by scan_index)',
                               unit='-')

timestamp = SignalDescription(signature='timestamp',
                              dtype=np.float64,
                              description='Time identifier for each single record  - there may be different timestamps'
                                          ' for records with the same scan index. '
                                          'Each DataSet within single DataSetContainer has the same time base '
                                          '(identified by timestamp)',
                              unit='s')

slot_id = SignalDescription(signature='slot_id',
                            dtype=np.int16,
                            description='location in parsed data for array-structured data (id of column) - zero-based',
                            unit='-')

log_data_row = SignalDescription(signature='log_data_row',
                            dtype=np.int32,
                            description='location in parsed data for array-structured data (id of row) - zero-based',
                            unit='-')

unique_id = SignalDescription(signature='unique_id',
                              dtype=np.int32,
                              description='Unique identifier for entity (lifetime) within all records in DataSet - '
                                          'zero-based',
                              unit='-')

id = SignalDescription(signature='id',
                       dtype=np.int16,
                       description='Identifier for entity - may be reused by different entity, for unique '
                                   'discrimination between entities use unique_id',
                       unit='-')

movement_status = SignalDescription(signature='movement_status',
                                    dtype='category',
                                    description='Status of movement of given object. See: '
                                                'aspe.extractors.Interfaces.Enums.Object.MovementStatus',
                                    unit='Enum')

motion_model_type = SignalDescription(signature='motion_model_type',
                                      dtype=np.int32,  # TODO: change it to object
                                      description='Type of motion model (dtype should be updated to enum)',
                                      unit='-')

motion_model_state_suppl = SignalDescription(signature='motion_model_state_suppl',
                                             dtype=object,
                                             description='list of physical attributes related to motion model',
                                             unit='-')

motion_model_variances_suppl = SignalDescription(signature='motion_model_variances_suppl',
                                                 dtype=object,
                                                 description='list of physical attributes related to motion model - '
                                                             'variances',
                                                 unit='-')

motion_model_covariances_suppl = SignalDescription(signature='motion_model_covariances_suppl',
                                                   dtype=object,
                                                   description='list of physical attributes related to motion model - '
                                                               'covariances',
                                                   unit='-')

object_class = SignalDescription(signature='object_class',
                                 dtype='category',
                                 description='Class of the object See: '
                                             'aspe.extractors.Interfaces.Enums.Object.ObjectClass',
                                 unit='Enum')

object_class_probability = SignalDescription(signature='object_class_probability',
                                             dtype=np.float32,
                                             description='Probability that object is with given class - coupled with '
                                                         'object_class. In range [0,1]',
                                             unit='-')

existence_indicator = SignalDescription(signature='existence_indicator',
                                        dtype=np.float32,
                                        description='Probability that object exists - corresponds to '
                                                    'Positive Predictive Value. In range [0,1]',
                                        unit='-')

ticks_since_created = SignalDescription(signature='ticks_since_created',
                                        dtype=np.float32,
                                        description='milliseconds since object was created',
                                        unit='ms')

ticks_since_last_detected = SignalDescription(signature='ticks_since_last_detected',
                                              dtype=np.float32,
                                              description='milliseconds since object was detected last time',
                                              unit='ms')

ticks_since_state_updated = SignalDescription(signature='ticks_since_state_updated',
                                              dtype=np.float32,
                                              description='milliseconds since object state was updated',
                                              unit='ms')

sensor_id = SignalDescription(signature='sensor_id',
                              dtype=np.int8,
                              description='ID of sensor',
                              unit='-')

look_id = SignalDescription(signature='look_id',
                            dtype=np.int8,
                            description='ID of look type',
                            unit='-')

look_index = SignalDescription(signature='look_index',
                               dtype=np.int32,
                               description='Unique identifier of the sensor look',
                               unit='-')

sensor_valid = SignalDescription(signature='sensor_valid',
                                 dtype=bool,
                                 description='Indication if sensor is valid',
                                 unit='-')

new_measurement_update = SignalDescription(signature='new_measurement_update',
                                           dtype=bool,
                                           description='Indication if there is any new information available',
                                           unit='-')

utc_timestamp = SignalDescription(signature='utc_timestamp',
                                  dtype='datetime64[ns]',
                                  description='UTC timestamp of entity, should be in datetime.datetime format',
                                  unit='datetime')