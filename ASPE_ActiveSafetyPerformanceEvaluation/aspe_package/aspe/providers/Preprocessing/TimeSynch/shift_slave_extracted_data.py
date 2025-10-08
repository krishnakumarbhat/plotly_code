from copy import deepcopy

from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.extractors.Interfaces.IDataSet import IDataSet
from aspe.providers.Preprocessing.TimeSynch.DataSetsProcessing.shift_by_offset import ShiftDataSetTimestampByOffset
from aspe.providers.Preprocessing.TimeSynch.time_synch_iface import TimeSynchInterface


class ShiftSlaveExtractedDataTimestamp(TimeSynchInterface):
    """
    Simple synchronization - shifting by offset of one extracted data
    """

    def __init__(self, offset: float = 0.0, inplace: bool = False, ts_slave_name: str = 'timestamp', **kwargs):
        """
        :param offset: Time offset by which timestamp of each data set from slave extracted data should be shifted
                       Default: 0.0
        :type offset: float
        :param inplace: flag indicating if input data should be modified or copied
                        Default: False
        :type offset: bool
        :param kwargs:
        """
        super().__init__(**kwargs)
        self.offset = offset
        self.inplace = inplace
        self.data_set_synchronization = ShiftDataSetTimestampByOffset(offset=self.offset, inplace=True,
                                                                      ts_slave_name=ts_slave_name)

    def synch(self,
              master_extracted_data: ExtractedData,
              slave_extracted_data: ExtractedData,
              *args, **kwargs) -> (ExtractedData, ExtractedData):
        """
        :param master_extracted_data: extracted data which will not be modified
        :type master_extracted_data: ExtractedData
        :param slave_extracted_data: extracted data which will  be modified - timestamp of each data set will be shifted
        :type slave_extracted_data: ExtractedData
        :param args:
        :param kwargs:
        :return:
        """

        if self.inplace:
            master_extracted_data = master_extracted_data
            slave_extracted_data = slave_extracted_data
        else:
            master_extracted_data = deepcopy(master_extracted_data)
            slave_extracted_data = deepcopy(slave_extracted_data)

        for data_set in slave_extracted_data.__dict__.values():
            if isinstance(data_set, IDataSet):
                self.data_set_synchronization.shift(data_set)

        return master_extracted_data, slave_extracted_data
