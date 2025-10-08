from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.providers.F360MdfRTRangeDataProvider import F360MdfRTRangeDataProvider

"""
Run this script as 'Run File in Console'
Example of single log evaluation:
pe - performance evaluation pipeline is configured by defaults strategies
"""

log_path = (r"C:\Users\zj9lvp\Documents\aspe_bmw_data\A370\TC1\BN_FASETH\120\MID_ECU_3.14.210_S_3.14"
            r".106_TRA_DS_01_120_150_L_BN_FASETH_WBATR91070LC63638_20200617_165000_fas_0003.MF4")
sw_version = 'A370'  # 'A' step
host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
host_length = 4.7
host_width = 1.9
cs_system = 'VCS'  # coordinate system in which evaluation will be performed
rt_f360_dt = 0.047  # time delay between RtRange and F360 which must be compensated

mdf_data_provider = F360MdfRTRangeDataProvider(sw_version=sw_version,
                                               host_rear_axle_to_front_bumper_dist=host_rear_axle_to_f_bumper,
                                               host_width=host_width,
                                               host_length=host_length,
                                               coordinate_system=cs_system,
                                               reference_time_shift=rt_f360_dt,
                                               rt_hunter_target_shift=10,
                                               save_to_file=True)

estimated_data, reference_data = mdf_data_provider.get_single_log_data(log_path)
pe = PEPipeline()
output = pe.evaluate(estimated_data, reference_data)
