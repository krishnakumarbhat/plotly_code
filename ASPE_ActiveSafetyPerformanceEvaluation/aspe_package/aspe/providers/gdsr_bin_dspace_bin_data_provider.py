import pandas as pd

from aspe.extractors.API.bin import extract_dspace_from_bin, extract_gdsr_from_bin
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.objects_in_sensors_fov import are_objects_in_fov


class GdsrBinDSpaceBinDataProvider(IPerfEvalInputDataProvider):
    """Data provider specialization for GDSR and dSpace ground-truth stored in bin files.

    The GDSR is often used in configurations that do not allow 360deg of FoV. The dSpace ground-truth on the other hand
    is for the whole scenario. Naturally, without any further processing the evaluation would report many false
    negatives caused by ground-truth objects outside the FoV. This provider solves that problem by flagging ground-truth
    objects that are outside the FoV of all sensors under evaluation. The flag can then be used during evaluation to
    exclude ground-truth objects outside the FoV from the calculation of the metrics.

    No time/scan index synchronization is performed by this class because both the GDSR and dSpace bin files are output
    by the customer resim which does the synchronization.

    :param extract_raw_signals: flag indicating if raw signals should be extracted. No translation is applied to raw
    signals. If yes extracted raw signals are placed in separate DataFrame within the extracted data. Defaults to False.
    :param save_to_file: flag indicating if extracted output should be saved as pickle file. Defaults to False.
    :param force_extract: forces extraction even if a pickle file already exists.
    Defaults to False.
    """

    def __init__(self, extract_raw_signals=False, save_to_file=False, force_extract=False):
        super().__init__()
        self._extract_raw_signals = extract_raw_signals
        self._save_to_file = save_to_file
        self._force_extract = force_extract

    def get_single_log_data(self, estimated_data_root_dir, reference_data_root_dir, log_name):
        estimated_data = extract_gdsr_from_bin(
            estimated_data_root_dir,
            log_name,
            extract_raw_signals=self._force_extract,
            save_to_file=self._save_to_file,
            force_extract=self._force_extract,
        )
        reference_data = extract_dspace_from_bin(
            reference_data_root_dir,
            log_name,
            extract_raw_signals=self._force_extract,
            save_to_file=self._save_to_file,
            force_extract=self._force_extract,
        )

        # Flag ground-truth objects that are not within the FoV of the sensors
        are_objects_in_fov_df = are_objects_in_fov(reference_data.objects, estimated_data.sensors)
        reference_data.objects.signals = pd.concat([reference_data.objects.signals, are_objects_in_fov_df], axis=1)

        return estimated_data, reference_data
