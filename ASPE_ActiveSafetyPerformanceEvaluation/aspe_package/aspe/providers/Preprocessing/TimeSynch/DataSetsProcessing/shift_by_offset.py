from copy import deepcopy

from aspe.extractors.Interfaces.IDataSet import IDataSet


class ShiftDataSetTimestampByOffset:
    """
    Taking timestamp from single data_set and shifts it by given offset
    """

    def __init__(self, offset, ts_slave_name, inplace=True):
        self._offset = offset
        self._inplace = inplace
        self._data = None
        self._ts_slave_name = ts_slave_name

    def shift(self,
              data_set: IDataSet,
              *args, **kwargs) -> IDataSet:

        if self._inplace:
            self._data = data_set
        else:
            self._data = deepcopy(data_set)

        self._shift_data_timestamps_by_offset()

        return self._data

    def _shift_data_timestamps_by_offset(self):
        self._data.signals[self._ts_slave_name] += self._offset
