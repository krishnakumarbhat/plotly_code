import os

from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4, extract_rt_range_3000_from_mf4
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.ScanIndexSynch.ScanIndexInterpolationSynch import ScanIndexInterpolationSynch
from aspe.providers.Preprocessing.TimeSynch.sync_by_ptp_system_latency_compensation import SyncByPTP
from aspe.providers.support_functions import get_single_mdf_log_pe_input_paths
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl


class F360MdfRTRangeDataProvider(IPerfEvalInputDataProvider):
    def __init__(self, *,
                 sw_version: str,
                 host_rear_axle_to_front_bumper_dist: float,
                 host_length: float,
                 host_width: float,
                 coordinate_system: str,
                 reference_time_shift=0.0,
                 rt_hunter_target_shift=None,
                 save_to_file=False,
                 time_synch_metod = None,
                 force_extract=False):
        self.sw_version = sw_version
        self.host_rear_axle_to_front_bumper_dist = host_rear_axle_to_front_bumper_dist
        self.host_length = host_length
        self.host_width = host_width
        self.coordinate_system = coordinate_system
        if time_synch_metod is None:
            self._time_synch = SyncByPTP(reference_dt=reference_time_shift)
        else:
            self._time_synch= time_synch_metod
        self._scan_index_synch = ScanIndexInterpolationSynch()
        self.hunter_target_instances_shift = rt_hunter_target_shift
        self.save_to_file = save_to_file
        self._force_extract = force_extract

    def get_single_log_data(self, log_path):
        estimated_data_path, reference_data_path = get_single_mdf_log_pe_input_paths(log_path)

        estimated_data, est_sync_save_path, f_est_synced = self._get_est_data(estimated_data_path)
        reference_data, ref_sync_save_path, f_ref_synced = self._get_ref_data(reference_data_path)

        if not f_est_synced or not f_ref_synced:
            self.synchronize(estimated_data, reference_data)
            estimated_data.transfer_cs(self.coordinate_system, self.host_rear_axle_to_front_bumper_dist)
            reference_data.transfer_cs(self.coordinate_system, self.host_rear_axle_to_front_bumper_dist)

        if self.save_to_file:
            save_to_pkl(reference_data, ref_sync_save_path)
            save_to_pkl(estimated_data, est_sync_save_path)

        return estimated_data, reference_data

    def _get_ref_data(self, reference_data_path):
        ref_sync_save_path = reference_data_path.replace('.MF4', '_rt_range_3000_mdf_extracted_sync.pickle')
        ref_raw_save_path = reference_data_path.replace('.MF4', '_rt_range_3000_mdf_extracted.pickle')

        if os.path.exists(ref_sync_save_path) and not self._force_extract:
            data_synced = True
            reference_data = load_from_pkl(ref_sync_save_path)
        elif os.path.exists(ref_raw_save_path) and not self._force_extract:
            data_synced = False
            reference_data = load_from_pkl(ref_raw_save_path)
        else:
            data_synced = False
            reference_data = extract_rt_range_3000_from_mf4(reference_data_path,
                                                            hunter_target_instances_shift=self.hunter_target_instances_shift,
                                                            hunter_length=self.host_length,
                                                            hunter_width=self.host_width)
        return reference_data, ref_sync_save_path, data_synced

    def _get_est_data(self, estimated_data_path):
        est_sync_save_path = estimated_data_path.replace('.MF4', '_f360_mf4_bmw_mid_extracted_sync.pickle')
        est_raw_save_path = estimated_data_path.replace('.MF4', '_f360_mf4_bmw_mid_extracted.pickle')

        if os.path.exists(est_sync_save_path) and not self._force_extract:
            data_synced = True
            estimated_data = load_from_pkl(est_sync_save_path)
        elif os.path.exists(est_raw_save_path) and not self._force_extract:
            data_synced = False
            estimated_data = load_from_pkl(est_raw_save_path)
        else:
            data_synced = False
            estimated_data = extract_f360_bmw_mid_from_mf4(estimated_data_path,
                                                           sw_version=self.sw_version,
                                                           host_rear_axle_to_front_bumper_dist=self.host_rear_axle_to_front_bumper_dist,
                                                           force_extract=self._force_extract)
        return estimated_data, est_sync_save_path, data_synced

    def synchronize(self, est_data, ref_data):
        self._time_synch.synch(master_extracted_data=est_data, slave_extracted_data=ref_data)
        self._scan_index_synch.synch(estimated_data=est_data, reference_data=ref_data,
                                     master_data_set=est_data.objects,
                                     slave_data_set=ref_data.host)


if __name__ == '__main__':
    path = r"C:\Users\zj9lvp\Documents\RT\BN_FASETH\A350_ECU3_12_4_SENS_3_12_5_SCW_DS1_V30_D1M_L_BN_FASETH_WBATR91070LC63638_20200309_165033_fas_0012.MF4"
    mdf_data_provider = F360MdfRTRangeDataProvider('A370', 3.7, 'VCS', save_to_file=True)
    estimated, reference = mdf_data_provider.get_single_log_data(path)
    pass
