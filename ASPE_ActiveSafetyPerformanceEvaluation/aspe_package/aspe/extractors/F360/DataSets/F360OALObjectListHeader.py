from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.extractors.Interfaces.SignalDescription.SignalDecription import SignalDescription

number_of_objects = SignalDescription(signature='number_of_objects',
                                      dtype=int,
                                      description='Number of objects provided in current instance',
                                      unit='-')

min_sensor_timestamp = SignalDescription(signature='min_sensor_timestamp',
                                         dtype=float,
                                         description='Minimum value of timestamp of detection list in current instance',
                                         unit='s')

max_sensor_timestamp = SignalDescription(signature='max_sensor_timestamp',
                                         dtype=float,
                                         description='Maximum value of timestamp of detection list in current instance',
                                         unit='s')

mean_sensor_timestamp = SignalDescription(signature='mean_sensor_timestamp',
                                          dtype=float,
                                          description='Mean value of timestamp of detection list in current instance',
                                          unit='s')


class F360OALObjectsHeader(IDataSet):
    def __init__(self):
        super().__init__()
        self.raw_signals = None
        signal_names = [
            number_of_objects,
            min_sensor_timestamp,
            max_sensor_timestamp,
            mean_sensor_timestamp,
        ]
        self.update_signals_definition(signal_names)

    def get_base_name(self):
        return 'oal_objects_header'
