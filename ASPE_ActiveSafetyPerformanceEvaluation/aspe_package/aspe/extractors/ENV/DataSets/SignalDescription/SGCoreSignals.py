import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

# SG_Output_T
num_contours = SignalDescription(signature='num_contours',
                                 dtype=np.uint16,
                                 description='Number of all contours reported in current tracking cycle',
                                 unit='-')

f_valid = SignalDescription(signature='f_valid',
                            dtype=bool,
                            description='Valid output indicator',
                            unit='-')

# SG_Contour_Out_T
drivability_class = SignalDescription(signature='drivability_class',
                                      dtype='category',
                                      description='Class of drivability. See ENV.Enums.env_stationary_geometries',
                                      unit='-')

contour_id = SignalDescription(signature='contour_id',
                               dtype=np.uint32,
                               description='Unique index of contour',
                               unit='-')

num_vertices = SignalDescription(signature='num_vertices',
                                 dtype=np.uint16,
                                 description='Number of vertices in contour',
                                 unit='-')

contour_type = SignalDescription(signature='contour_type',
                                 dtype='category',
                                 description='Type of contour. See ENV.Enums.env_stationary_geometries',
                                 unit='-')
