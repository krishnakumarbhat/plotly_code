import re
from copy import deepcopy
from pathlib import Path

from srr5_dev_tools import pkl_module

from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4
from aspe.extractors.API.sdb import extract_pandora_log_from_sdb
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.ScanIndexSynch.ScanIndexInterpolationSynch import ScanIndexInterpolationSynch
from aspe.providers.Preprocessing.TimeSynch.sync_by_ptp_system_latency_compensation import SyncByPTP
from aspe.utilities.SupportingFunctions import save_to_pkl


class F360MDF_Pandora_DataProvider(IPerfEvalInputDataProvider):
    def __init__(self, *,
                 set_id,
                 sw_version: str,
                 host_rear_axle_to_front_bumper_dist: float,
                 lidar_to_f_bumper: float,
                 host_length: float,
                 host_width: float,
                 stationary_threshold: float,
                 coordinate_system: str,
                 shift_offset=0.03,
                 save_to_file=False,
                 label_sdb_cache_dir: Path = None,
                 extract_sdb_cache_dir: Path = None,
                 sync_sdb_cache_dir: Path = None,
                 parse_f360_cache_dir: Path = None,
                 extract_f360_cache_dir: Path = None,
                 sync_f360_cache_dir: Path = None,
                 ):
        self.set_id = set_id
        self.sw_version = sw_version
        self.host_rear_axle_to_front_bumper_dist = host_rear_axle_to_front_bumper_dist
        self.lidar_to_f_bumper = lidar_to_f_bumper
        self.host_length = host_length
        self.host_width = host_width
        self.stationary_threshold = stationary_threshold
        self.coordinate_system = coordinate_system
        self._time_synch = SyncByPTP(reference_dt=shift_offset)
        self._scan_index_synch = ScanIndexInterpolationSynch()
        self.reference_data = None
        self.save_to_file = save_to_file
        self.sync_f360_cache_dir = sync_f360_cache_dir
        self.extract_f360_cache_dir = extract_f360_cache_dir
        self.parse_f360_cache_dir = parse_f360_cache_dir
        self.sync_sdb_cache_dir = sync_sdb_cache_dir
        self.extract_sdb_cache_dir = extract_sdb_cache_dir
        self.label_sdb_cache_dir = label_sdb_cache_dir

    def get_single_log_data(self, log_path):
        estimated_data_path = Path(log_path)

        estimated_data, est_sync_save_path, f_est_loaded_from_pkl = self._get_est_data(estimated_data_path)
        reference_data, ref_sync_save_path, f_ref_loaded_from_pkl = self._get_ref_data(estimated_data_path)

        if not f_est_loaded_from_pkl or not f_ref_loaded_from_pkl:
            self.synchronize(estimated_data, reference_data)
            estimated_data.transfer_cs(self.coordinate_system, self.host_rear_axle_to_front_bumper_dist)
            reference_data.transfer_cs(self.coordinate_system, self.lidar_to_f_bumper)

            if self.save_to_file:
                save_to_pkl(estimated_data, est_sync_save_path)
                save_to_pkl(reference_data, ref_sync_save_path)

        return estimated_data, reference_data

    def _get_ref_data(self, estimated_data_path: Path):
        log_index = int(re.search(r'_(\d{4})\.', estimated_data_path.name).group(1))
        if self.sync_sdb_cache_dir is None:
            sync_sdb_cache_dir = estimated_data_path.parents[1] / 'ASP_output' / 'SDB_cache'
        else:
            sync_sdb_cache_dir = self.sync_sdb_cache_dir
        sync_sdb_cache_dir.mkdir(parents=True, exist_ok=True)
        ref_sync_save_path = sync_sdb_cache_dir / f'log_{self.set_id}_sdb_extracted_sync_{log_index}.pickle'
        if ref_sync_save_path.exists():
            f_loaded_from_pkl = True
            reference_data = pkl_module.load(ref_sync_save_path)
        else:
            f_loaded_from_pkl = False
            if self.reference_data is None:
                self.reference_data = extract_pandora_log_from_sdb(
                    str(estimated_data_path),
                    self.set_id,
                    self.stationary_threshold,
                    extract_sdb_cache_dir=self.extract_sdb_cache_dir,
                    label_sdb_cache_dir=self.label_sdb_cache_dir,
                )
            reference_data = deepcopy(self.reference_data)
        return reference_data, ref_sync_save_path, f_loaded_from_pkl

    def _get_est_data(self, mf4_log_path: Path):
        if self.sync_f360_cache_dir is None:
            sync_f360_cache_dir = mf4_log_path.parents[1] / 'parse_result' / mf4_log_path.parent.stem
        else:
            sync_f360_cache_dir = self.sync_f360_cache_dir
        sync_f360_cache_dir.mkdir(parents=True, exist_ok=True)
        est_sync_save_path = sync_f360_cache_dir / f'{mf4_log_path.stem}_f360_mf4_bmw_mid_extracted_sync.pickle'
        if est_sync_save_path.exists():
            f_loaded_from_pkl = True
            estimated_data = pkl_module.load(est_sync_save_path)
        else:
            f_loaded_from_pkl = False
            estimated_data = extract_f360_bmw_mid_from_mf4(
                str(mf4_log_path),
                self.sw_version,
                save_to_file=True,
                host_rear_axle_to_front_bumper_dist=self.host_rear_axle_to_front_bumper_dist,
                parse_cache_path=self.parse_f360_cache_dir,
                extract_cache_path=self.extract_f360_cache_dir,
            )
        return estimated_data, est_sync_save_path, f_loaded_from_pkl

    def synchronize(self, est_data, ref_data):
        self._time_synch.synch(master_extracted_data=est_data, slave_extracted_data=ref_data)
        self._scan_index_synch.synch(estimated_data=ref_data, reference_data=est_data,
                                     master_data_set=est_data.objects,
                                     slave_data_set=ref_data.objects)
