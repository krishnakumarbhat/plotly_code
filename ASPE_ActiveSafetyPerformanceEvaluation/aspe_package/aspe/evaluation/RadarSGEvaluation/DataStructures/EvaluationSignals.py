import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

contour_unique_id = SignalDescription(signature='contour_unique_id',
                                      dtype=np.uint32,
                                      description='Unique id of the contour',
                                      unit='-')

sample_position_x = SignalDescription(signature='sample_position_x',
                                      dtype=np.float32,
                                      description='x coordinate of sample',
                                      unit='m')

sample_position_y = SignalDescription(signature='sample_position_y',
                                      dtype=np.float32,
                                      description='y coordinate of sample',
                                      unit='m')

projection_sample_position_x = SignalDescription(signature='projection_sample_position_x',
                                                 dtype=np.float32,
                                                 description='x coordinate of the projected sample',
                                                 unit='m')

projection_sample_position_y = SignalDescription(signature='projection_sample_position_y',
                                                 dtype=np.float32,
                                                 description='y coordinate of the projected sample',
                                                 unit='m')

start_position_x = SignalDescription(signature='start_position_x',
                                     dtype=np.float32,
                                     description='x coordinate of segment 1st point',
                                     unit='m')

end_position_x = SignalDescription(signature='end_position_x',
                                   dtype=np.float32,
                                   description='x coordinate of segment 2nd point',
                                   unit='m')

start_position_y = SignalDescription(signature='start_position_y',
                                     dtype=np.float32,
                                     description='y coordinate of segment 1st point',
                                     unit='m')

end_position_y = SignalDescription(signature='end_position_y',
                                   dtype=np.float32,
                                   description='y coordinate of segment 2nd point',
                                   unit='m')

segment_unique_id = SignalDescription(signature='segment_unique_id',
                                      dtype=np.int32,
                                      description='unique id of segment',
                                      unit='-')
segment_unique_id_paired = SignalDescription(signature='segment_unique_id_paired',
                                             dtype=np.int32,
                                             description='unique id of paired segment',
                                             unit='-')
projection_start_position_x = SignalDescription(signature='projection_start_position_x',
                                                dtype=np.float32,
                                                description='x projection position of start_position_x',
                                                unit='m')

projection_start_position_y = SignalDescription(signature='projection_start_position_y',
                                                dtype=np.float32,
                                                description='y projection position of start_position_y',
                                                unit='m')

unit_vector_x_paired = SignalDescription(signature='unit_vector_x_paired',
                                         dtype=np.float32,
                                         description='x coordinate of unit vector of paired segment',
                                         unit='-')
unit_vector_y_paired = SignalDescription(signature='unit_vector_y_paired',
                                         dtype=np.float32,
                                         description='y coordinate of unit vector of paired segment',
                                         unit='-')

dot_product = SignalDescription(signature='dot_product',
                                dtype=np.float32,
                                description='dot products of unit paired vectors',
                                unit='-')

distance = SignalDescription(signature='distance',
                             dtype=np.float32,
                             description='distance between referenced and estimated samples',
                             unit='m')

classification = SignalDescription(signature='classification',
                                   dtype=bool,
                                   description='True for TruePositives samples',
                                   unit='-')

multiple_segmentation = SignalDescription(signature='multiple_segmentation',
                                          dtype=bool,
                                          description='True for under or over segmentation samples',
                                          unit='-')

deviation_x = SignalDescription(signature='deviation_x',
                                dtype=np.float32,
                                description='deviation of x position',
                                unit='m')

deviation_y = SignalDescription(signature='deviation_y',
                                dtype=np.float32,
                                description='deviation of y position',
                                unit='m')

association_gate = SignalDescription(signature='association_gate',
                                     dtype=np.float32,
                                     description='association gate width',
                                     unit='m')

segmentation_type = SignalDescription(signature='segmentation_type',
                                      dtype='category',
                                      description='Enum of segmentation type',
                                      unit='-')

binary_classification = SignalDescription(signature='binary_classification',
                                          dtype='category',
                                          description='Enum of binary classification',
                                          unit='-')
