from aspe.extractors.Interfaces.IExtractor import IExtractor
from aspe.extractors.ReferenceExtractor.SDB.Builders.SDB_ObjectListBuilder import SDB_ObjectListBuilder
from aspe.extractors.ReferenceExtractor.SDB.DataSets.SDB_ExtractedData import SDB_ExtractedData


class SDB_Extractor(IExtractor):
    def __init__(self, *, stationary_threshold, rear_axle_to_front_bumper_distance=None, f_extract_raw_signals=True):
        super().__init__()
        self.stationary_threshold = stationary_threshold
        self._f_extract_raw_signals = f_extract_raw_signals
        self._rear_axle_to_front_bumper_distance = rear_axle_to_front_bumper_distance

    def extract_data(self, parsed_data: dict):
        """
        Extract data from SDB output and transform it into ASPE data structures.
        """
        extracted = SDB_ExtractedData()
        extracted.objects = self._build_objects_list(parsed_data)
        return extracted

    def _build_objects_list(self, parsed_data):
        return SDB_ObjectListBuilder(parsed_data, self.stationary_threshold, self._f_extract_raw_signals).build()

    @staticmethod
    def _get_extractor_name():
        return 'SDB'


if __name__ == '__main__':
    from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl

    extr = SDB_Extractor()
    sdb_data_path = r"C:\logs\pandora\test.pickle"
    data = load_from_pkl(sdb_data_path)
    extracted = extr.extract_data(data)
    save_to_pkl(extracted, sdb_data_path.replace('.pickle', '_sdb_extracted.pickle'))
