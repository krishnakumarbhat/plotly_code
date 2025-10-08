import pandas as pd

from aspe.extractors.Interfaces.IObjects import IObjects
from aspe.extractors.Interfaces.SignalDescription import PhysicalSignals
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

FL_total_area = SignalDescription(signature='SRR5-FL_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Front Left sensor',
                                  unit='m^2')

FL_visible_area = SignalDescription(signature='SRR5-FL_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Front Left sensor',
                                    unit='m^2')

FR_total_area = SignalDescription(signature='SRR5-FR_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Front Right sensor',
                                  unit='m^2')

FR_visible_area = SignalDescription(signature='SRR5-FR_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Front Right sensor',
                                    unit='m^2')

RL_total_area = SignalDescription(signature='SRR5-RL_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Rear Left sensor',
                                  unit='m^2')

RL_visible_area = SignalDescription(signature='SRR5-RL_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Rear Left sensor',
                                    unit='m^2')

RR_total_area = SignalDescription(signature='SRR5-RR_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Rear Right sensor',
                                  unit='m^2')

RR_visible_area = SignalDescription(signature='SRR5-RR_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Rear Right sensor',
                                    unit='m^2')

CL_total_area = SignalDescription(signature='SRR5-CL_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Center Left sensor'
                                              '(bi-pillar)',
                                  unit='m^2')

CL_visible_area = SignalDescription(signature='SRR5-CL_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Center Left sensor'
                                                '(bi-pillar)',
                                    unit='m^2')

CR_total_area = SignalDescription(signature='SRR5-CR_total_area',
                                  dtype=float,
                                  description='Total area of object which may be visible by Center Right sensor'
                                              '(bi-pillar)',
                                  unit='m^2')

CR_visible_area = SignalDescription(signature='SRR5-CR_visible_area',
                                    dtype=float,
                                    description='Total area of object visible by Center Right sensor'
                                                '(bi-pillar)',
                                    unit='m^2')

vigem_timestamp = SignalDescription(signature='vigem_timestamp',
                                    dtype=float,
                                    description='timestamp of Vigem data logger',
                                    unit='s')


class SDB_ObjectList(IObjects):

    def __init__(self, hysicalSignals=None):
        super().__init__()
        occlusion_signals = [
            FL_total_area,
            FL_visible_area,
            FR_total_area,
            FR_visible_area,
            RL_total_area,
            RL_visible_area,
            RR_total_area,
            RR_visible_area,
            CL_total_area,
            CL_visible_area,
            CR_total_area,
            CR_visible_area,
        ]
        signal_names = [
            PhysicalSignals.position_z,
            PhysicalSignals.bounding_box_dimensions_z,
            PhysicalSignals.center_z,
            PhysicalSignals.velocity_otg_z,
            vigem_timestamp,
        ]

        self.update_signals_definition(occlusion_signals)
        self.update_signals_definition(signal_names)
        self.signals_info.loc[self.signals_info.signature == 'id', 'signal_type'] = str  # TODO: is it still needed?

    def get_base_name(self):
        return 'object_list'
