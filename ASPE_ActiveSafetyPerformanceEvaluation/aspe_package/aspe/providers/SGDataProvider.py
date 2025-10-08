from pathlib import Path
from typing import Tuple

from aspe.extractors.API.keg import extract_env_from_keg
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl


class SGDataProvider(IPerfEvalInputDataProvider):
    def __init__(self,
                 save_to_pickle: bool = False,
                 force_extract: bool = False):

        self._save_to_pickle = save_to_pickle
        self._force_extract = force_extract

    def get_single_log_data(self, est_log_path: str, ref_log_path: str) -> Tuple[ENVExtractedData, ENVExtractedData]:
        """
        Get a pair of estimated and reference data, which can be used as performance evaluation input.

        Processing scheme:
        - extract data which needs to be evaluated using log_path
        - find corresponding reference data:
            1. Provide keg/pickle file with reference data
            2. Generate AutoGT reference data if keg/pickle has not been provided (TODO: FZD-336 feature)

        - save_to_pickle: default False. Set True to generate pickle file with extracted dataframes.
        - return extracted estimated data and extracted reference data as tuple

        :param est_log_path: path to estimated data
        :param ref_log_path: path to reference data

        :return: (estimated_data, reference_data)
        """

        estimated_data = self._get_extracted(log_path=est_log_path,
                                             f_is_ref=False)
        reference_data = self._get_extracted(log_path=ref_log_path,
                                             f_is_ref=True)

        return estimated_data, reference_data

    def _get_extracted(self, log_path: str, f_is_ref: bool) -> ENVExtractedData:
        pickle_path = self._get_pickle_path(log_path, f_is_ref)
        if not self._force_extract and Path(pickle_path).is_file():
            extracted = load_from_pkl(pickle_path)
        else:
            self._verify_path(log_path)
            extracted = extract_env_from_keg(log_path,
                                             raw_signals=True,
                                             save_to_file=False,
                                             force_extract=self._force_extract)

            if self._save_to_pickle:
                save_to_pkl(extracted, pickle_path)
        return extracted

    @staticmethod
    def _verify_path(path: str):
        if not Path(path).is_file():
            raise FileNotFoundError(f"File not found: {path}")

    @staticmethod
    def _get_pickle_path(log_path: str, f_is_ref: bool) -> str:
        suffix = "_ref_extracted.pickle" if f_is_ref else "_est_extracted.pickle"
        return log_path.replace(".keg", suffix)


if __name__ == '__main__':
    keg_path = r""
    provider = SGDataProvider(save_to_pickle=True,
                              force_extract=True)
    estimation_extracted, reference_extracted = provider.get_single_log_data(keg_path, keg_path)

