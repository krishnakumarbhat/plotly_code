"""
Scenario Generator CONTRA Reference Signals Descriptions
"""

import numpy as np

from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

vertex_id = SignalDescription(signature='vertex_id',
                              dtype=np.uint16,
                              description='Reference vertex id.',
                              unit='-')

contour_id = SignalDescription(signature='contour_id',
                               dtype=np.uint16,
                               description='Reference contour id.',
                               unit='-')
