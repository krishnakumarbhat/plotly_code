from pathlib import Path

from aspe.extractors.API.mdf import extract_rt_range_3000_from_mf4, parse_faseth_data
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.F360.MDF4_BMW_mid.Builders.PTPDataBuilder import PTPDataBuilder
from aspe.providers.IPerfEvalInputDataProvider import IPerfEvalInputDataProvider
from aspe.providers.Preprocessing.ScanIndexSynch.ScanIndexInterpolationSynch import ScanIndexInterpolationSynch
from aspe.providers.Preprocessing.TimeSynch.sync_by_ptp_system_latency_compensation import SyncByPTP
from aspe.utilities.SupportingFunctions import iterate_parents_and_find_directory, load_from_pkl, save_to_pkl


class DummyExtractedData:
    def __init__(self, ptp_data):
        self.ptp_data = ptp_data


class F360MudpRtRangeMdfDataProvider(IPerfEvalInputDataProvider):
    def __init__(self, *,
                 mudp_stream_defs_path: str,
                 raw_signals: bool = False,
                 reference_time_shift=0.0,
                 rt_hunter_target_shift=None,
                 save_to_file=False,
                 force_extract=False):

        self.mudp_stream_defs_path = mudp_stream_defs_path
        self.raw_signals = raw_signals
        self._time_synch = SyncByPTP(reference_dt=reference_time_shift)
        self._scan_index_synch = ScanIndexInterpolationSynch()
        self.hunter_target_instances_shift = rt_hunter_target_shift
        self.save_to_file = save_to_file
        self._force_extract = force_extract

    def get_single_log_data(self, mudp_log_path: str, srr_reference_dir=None, bn_faseth_dir=None):

        srr_reference_dir, bn_faseth_dir = self.get_srr_reference_and_bn_faseth_dir(mudp_log_path, srr_reference_dir,
                                                                                    bn_faseth_dir)
        ref_path = self.get_reference_data_path(srr_reference_dir, mudp_log_path)
        ref_sync_save_path, est_sync_save_path = self.get_ref_est_sync_save_paths(ref_path, mudp_log_path)

        reference_data, ref_synchronized = self.get_reference_data(ref_sync_save_path, ref_path)
        estimated_data, _ = self.get_estimated_data(est_sync_save_path, mudp_log_path)
        ptp_data = self.get_ptp_data(mudp_log_path, bn_faseth_dir)

        estimated_data.transfer_cs('VCS', estimated_data.host.dist_of_rear_axle_to_front_bumper)
        reference_data.transfer_cs('VCS', estimated_data.host.dist_of_rear_axle_to_front_bumper)

        if not ref_synchronized:
            self._time_synch.synch(master_extracted_data=DummyExtractedData(ptp_data),
                                   slave_extracted_data=reference_data)
            self._scan_index_synch.synch(estimated_data=estimated_data, reference_data=reference_data,
                                         master_data_set=estimated_data.internal_objects,
                                         slave_data_set=reference_data.host)

        if self.save_to_file:
            save_to_pkl(reference_data, ref_sync_save_path)
            save_to_pkl(estimated_data, est_sync_save_path)

        return estimated_data, reference_data

    def get_srr_reference_and_bn_faseth_dir(self, mudp_log_path: str, srr_reference_dir, bn_faseth_dir):
        mudp_log_p = Path(mudp_log_path)
        mudp_dir = mudp_log_p.parent

        srr_reference_dir = self.get_srr_reference_dir(mudp_dir, srr_reference_dir)
        bn_faseth_dir = self.get_bn_faseth_dir(mudp_dir, bn_faseth_dir)
        return srr_reference_dir, bn_faseth_dir

    def get_srr_reference_dir(self, mudp_dir, srr_reference_dir):
        if srr_reference_dir is None:
            srr_reference_dir = iterate_parents_and_find_directory(mudp_dir, 'SRR_REFERENCE')
        elif isinstance(srr_reference_dir, str):
            srr_reference_dir = Path(srr_reference_dir)
        return srr_reference_dir

    def get_bn_faseth_dir(self, mudp_dir, bn_faseth_dir):
        if bn_faseth_dir is None:
            bn_faseth_dir = iterate_parents_and_find_directory(mudp_dir, 'BN_FASETH')
        elif isinstance(bn_faseth_dir, str):
            bn_faseth_dir = Path(bn_faseth_dir)
        return bn_faseth_dir

    def get_reference_data_path(self, srr_reference_dir, mudp_log_path):
        mudp_bare_name = get_mudp_bare_file_name(mudp_log_path)
        ref_name_patern = mudp_bare_name.replace('_deb_', '_ref_').replace('SRR_DEBUG', 'SRR_REFERENCE')
        mdf_ref_paths = list(srr_reference_dir.glob(ref_name_patern + '*.MF4'))
        pickle_ref_paths = list(srr_reference_dir.glob(ref_name_patern + '*_rt_range_3000_mdf_extracted.pickle'))

        if len(pickle_ref_paths) > 0:
            srr_ref_path = pickle_ref_paths[0]
        elif len(mdf_ref_paths) > 0:
            srr_ref_path = mdf_ref_paths[0]
        else:
            raise FileNotFoundError("Log file with reference data not found - ensure that log is in SRR_REFERENCE dir")
        return srr_ref_path

    def get_ref_est_sync_save_paths(self, reference_path, mudp_log_path):
        ref_ext = reference_path.suffix
        if ref_ext == '.pickle':
            ref_save_path = str(reference_path).replace(ref_ext, '_sync.pickle')
        else:
            ref_save_path = str(reference_path).replace(ref_ext, '_rt_range_3000_mdf_extracted_sync.pickle')
        est_save_path = mudp_log_path.replace('.mudp', '_f360_mudp_extracted_sync.pickle')
        return ref_save_path, est_save_path

    def get_reference_data(self, ref_sync_save_path, ref_path):
        if not Path(ref_sync_save_path).exists():
            ref_synchronized = False

            if ref_path.suffix == '.pickle':
                reference_data = load_from_pkl(str(ref_path))  # load raw (not synchronized) RtRange from pickle
            else:
                reference_data = \
                    extract_rt_range_3000_from_mf4(str(ref_path),
                                                   hunter_length=5.0, hunter_width=2.0,
                                                   save_to_file=True,
                                                   raw_signals=self.raw_signals,
                                                   save_mgp_parsed_data=True,
                                                   hunter_target_instances_shift=self.hunter_target_instances_shift)
        else:
            ref_synchronized = True
            reference_data = load_from_pkl(ref_sync_save_path)  # load synchronized RtRange from pickle
        return reference_data, ref_synchronized

    def get_estimated_data(self, est_sync_save_path, mudp_log_path):
        est_sync_exists = Path(est_sync_save_path).exists()
        if not est_sync_exists or self._force_extract:
            loaded_from_pkl = False
            estimated_data = extract_f360_from_mudp(mudp_log_path, self.mudp_stream_defs_path,
                                                    raw_signals=self.raw_signals,
                                                    internal_objects=True,
                                                    detections=True,
                                                    oal_objects=True,
                                                    save_to_file=True,
                                                    force_extract=self._force_extract)
        else:
            loaded_from_pkl = True
            estimated_data = load_from_pkl(est_sync_save_path)

        return estimated_data, loaded_from_pkl

    def get_ptp_data(self, mudp_log_path, bn_faseth_dir):
        mudp_bare_name = get_mudp_bare_file_name(mudp_log_path)
        fas_name_patern = mudp_bare_name.replace('_deb_', '_fas_').replace('SRR_DEBUG', 'BN_FASETH')
        ptp_data_path = bn_faseth_dir / (fas_name_patern + '_ptp_data.pickle')
        if Path(ptp_data_path).exists():
            ptp_data = load_from_pkl(ptp_data_path)
        else:
            bn_fas_paths = list(bn_faseth_dir.glob(fas_name_patern + '*.MF4'))
            if len(bn_fas_paths) > 0:
                bn_fas_path = str(bn_fas_paths[0])
            else:
                raise AttributeError
            ptp_parsed = parse_faseth_data(bn_fas_path, 'A370', 'mPAD-SRR_Master')
            ptp_data = PTPDataBuilder(ptp_parsed['data']['PTPMessages']).build()
            save_to_pkl(ptp_data, ptp_data_path)
        return ptp_data


def get_mudp_bare_file_name(mudp_log_path: str):
    file_name = Path(mudp_log_path).name
    resim_ext_position = file_name.find('_rRf360')
    if resim_ext_position == -1:
        mudp_bare_name = file_name.replace('.mudp', '')  # converted logs case
    else:
        mudp_bare_name = file_name[:resim_ext_position]  # converted and resimed logs case
    return mudp_bare_name


if __name__ == '__main__':
    logs_dir = r'C:\logs\VTV_mf4_logs\A370\DS_01_target_overtaking\BN_FASETH'
    logger_path = 'C:\logs\VTV_mf4_logs\A370\DS_01_target_overtaking\BN_FASETH'
    sw_version = 'A370'  # 'A' step
    host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
    host_length = 4.7
    host_width = 1.9
    cs_system = 'VCS'  # coordinate system in which evaluation will be performed
    rt_f360_dt = 0.0  # time delay between RtRange and F360 which must be compensated

    path = r"C:\logs\VTV_mf4_logs\A370\DS_01_target_overtaking\SRR_DEBUG\rRf360t4210309v205p50_last_dev\MID_ECU_3.14.210_S_3.14.106_TRA_DS_01_50_80_L_SRR_DEBUG_WBATR91070LC63638_20200617_164334_deb_0001_rRf360t4210309v205p50_last_dev.mudp"
    provider = F360MudpRtRangeMdfDataProvider(
        mudp_stream_defs_path="C:\wkspaces_git\F360Core\sw\zResimSupport\stream_definitions",
        host_rear_axle_to_front_bumper_dist=5,
        host_width=host_width,
        host_length=host_length,
        rt_hunter_target_shift=10,
        coordinate_system=cs_system,
        save_to_file=True,
    )
    estimated, reference = provider.get_single_log_data(path)
