import os
from pathlib import Path

from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.Interfaces.ExtractedData import ExtractedData
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.utilities.ground_truth_from_dets import calc_velocity_profile_for_objects, create_ground_truth
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl


class F360MudpGTfromDetsProvider(IPerfEvalInputDataProvider):
    def __init__(self, *,
                 mudp_stream_defs_path: str,
                 save_to_file=False,
                 force_extract=False,
                 stat_u_ids={},
                 prev_scans_v_profile=1,
                 next_scans_v_profile=1,
                 prev_scans_bbox=4,
                 next_scans_bbox=1,
                 range_value=30):
        """
        Provides estimated and reference data for KPI calculation and extracted data file for detection ground truth
        visualization.
        @param mudp_stream_defs_path: path to stream definitions directory
        @param save_to_file: if true output is saved to .pickle file
        @param force_extract: if true extraction is performed even if .pickle files exists
        @param stat_u_ids: unique ids of stationary objects that should be analyzed
        @param prev_scans_v_profile: number of previous scans that should be analyzed during velocity profile estimation
        @param next_scans_v_profile: number of next scans that should be analyzed during velocity profile estimation
        @param prev_scans_bbox: number of previous scans that should be analyzed during bbox placement
        @param next_scans_bbox: number of next scans that should be analyzed during bbox placement
        @param range_value: range value (in meters) in which objects should be analyzed, allows filtering poor object
        size estimation when objects are away
        If you need detailed parameter description checkout calc_velocity_profile_for_objects and create_ground_truth
        functions documentation. (location: aspe.utilities.ground_truth_from_dets)
        """


        self._mudp_stream_defs_path = mudp_stream_defs_path
        self._save_to_file = save_to_file
        self._force_extract = force_extract
        self._stat_u_ids = stat_u_ids
        self._prev_scans_v_profile = prev_scans_v_profile
        self._next_scans_v_profile = next_scans_v_profile
        self._prev_scans_bbox = prev_scans_bbox
        self._next_scans_bbox = next_scans_bbox
        self._range_value = range_value

    def get_single_log_data(self, mudp_log_path: str):

        if self._stat_u_ids:
            stat_ids_to_analyze = self.get_stat_u_ids(mudp_log_path)
        else:
            stat_ids_to_analyze = []

        is_ref_path = Path(mudp_log_path.replace(".mudp", "_f360_mudp_reference.pickle")).is_file()
        is_est_path = Path(mudp_log_path.replace(".mudp", "_f360_mudp_estimated.pickle")).is_file()
        is_ext_path = Path(mudp_log_path.replace(".mudp", "_f360_mudp_extracted.pickle")).is_file()

        if is_ext_path and is_ref_path and is_est_path and not self._force_extract:
            print("Found pickle file. Loading instead of extracting.")
            estimated_data = load_from_pkl(mudp_log_path.replace(".mudp", "_f360_mudp_estimated.pickle"))
            reference_data = load_from_pkl(mudp_log_path.replace(".mudp", "_f360_mudp_reference.pickle"))
            extracted = load_from_pkl(mudp_log_path.replace(".mudp", "_f360_mudp_extracted.pickle"))
            print("Loaded successfully.")
        else:
            print("Extracting MUDP file.")
            extracted = extract_f360_from_mudp(mudp_log_path,
                                               internal_objects=True,
                                               detections=True,
                                               raw_signals=True,
                                               force_extract=True)
            print("Calculating velocity profile for objects.")
            calc_velocity_profile_for_objects(extracted, stat_u_ids=stat_ids_to_analyze,
                                              prev_scans=self._prev_scans_v_profile,
                                              next_scans=self._next_scans_v_profile)
            print("Creating ground truth from detection.")
            extracted.detections_based_gt = create_ground_truth(extracted,
                                                                stat_u_ids=stat_ids_to_analyze,
                                                                prev_scans=self._prev_scans_bbox,
                                                                next_scans=self._next_scans_bbox,
                                                                range_value=self._range_value)

            estimated_data = extracted
            reference_data = ExtractedData()
            reference_data.objects = estimated_data.detections_based_gt

        if self._save_to_file:
            save_to_pkl(reference_data, mudp_log_path.replace(".mudp", "_f360_mudp_reference.pickle"))
            save_to_pkl(estimated_data, mudp_log_path.replace(".mudp", "_f360_mudp_estimated.pickle"))
            save_to_pkl(extracted, mudp_log_path.replace(".mudp", "_f360_mudp_extracted.pickle"))
            print("Results saved as .pickle files.")

        return estimated_data, reference_data

    def get_stat_u_ids(self, mudp_log_path : str):
        dir, filename = os.path.split(mudp_log_path)
        names = [name for name in os.listdir(dir) if '.mudp' in name]
        logs = list(self._stat_u_ids.keys())
        return self._stat_u_ids[logs[names.index(filename)]]


if __name__ == "__main__":

    path = r"E:\logfiles\DFT-1978\20220704_BMWSP25_iBrake_Asta\KPI_calculation"
    provider = F360MudpGTfromDetsProvider(
        mudp_stream_defs_path=r"C:\wkspaces_git\F360Core\sw\zResimSupport\stream_definitions",
        save_to_file=True,
        force_extract=True,
    )
    estimated, reference = provider.get_single_log_data(path)
