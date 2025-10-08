from copy import deepcopy
from aspe.estractors.Interfaces.IDataSet import IDataSet
from aspe.estractors.Interfaces.ExtractedData import ExtractedData


class ScanIndexExtraction:
    @staticmethod
    def extract_from_single_data_set(data_set: IDataSet, si: int, inplace=False) -> IDataSet:
        """

        :param data_set: Input Data Set
        :type data_set: IDataSet
        :param si: scan index for which Data Set should be filtered
        :type si: int
        :param inplace: flag indicating if DataSet should be overwritten, default False
        :return: IDataSet
        """
        if inplace:
            out_data_set = data_set
        else:
            out_data_set = deepcopy(data_set)

        out_data_set.signals = out_data_set.signals[out_data_set.signals.scan_index == si]
        return out_data_set

    @staticmethod
    def extract_from_extracted_data(extracted_data: ExtractedData, si: int, inplace=False) -> ExtractedData:
        """

        :param extracted_data: Input Extracted Data
        :type extracted_data: ExtractedData
        :param si: scan index for which extracted_data should be filtered
        :type si: int
        :param inplace: flag indicating if DataSet should be overwritten, default False
        :return: ExtractedData
        """
        if inplace:
            out_extracted_data = extracted_data
        else:
            out_extracted_data = deepcopy(extracted_data)

        for data_set in out_extracted_data.__dict__.values():
            if isinstance(data_set, IDataSet):
                ScanIndexExtraction.extract_from_single_data_set(data_set, si, inplace=True)
        return out_extracted_data

    @staticmethod
    def get_min_scan_index(extracted_data):
        """

        :param extracted_data: Input Extracted Data
        :return: min scan index for given xtracted data
        """

        scan_indexes = []
        for data_set in extracted_data.__dict__.values():
            if isinstance(data_set, IDataSet):
                scan_indexes.append(data_set.signals.scan_index.min())
        scan_index = min(scan_indexes)
        return scan_index

    @staticmethod
    def get_max_scan_index(extracted_data):
        """

        :param extracted_data: Input Extracted Data
        :return: max scan index for given extracted data
        """

        scan_indexes = []
        for data_set in extracted_data.__dict__.values():
            if isinstance(data_set, IDataSet):
                scan_indexes.append(data_set.signals.scan_index.min())
        scan_index = max(scan_indexes)
        return scan_index
