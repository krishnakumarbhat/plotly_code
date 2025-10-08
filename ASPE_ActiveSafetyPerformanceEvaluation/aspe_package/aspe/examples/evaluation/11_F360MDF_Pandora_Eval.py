import re
import warnings
from pathlib import Path
from time import time

from srr5_dev_tools import pkl_module
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.Association.RelEstToRefNN import RelEstToRefNN
from aspe.evaluation.RadarObjectsEvaluation.Flags import IsMovableFlag, IsVisibleFlag
from aspe.evaluation.RadarObjectsEvaluation.PairsFeatures.distance_metrics.ComplexMahalanobisDistance import (
    ComplexMahalanobisDistance,
)
from aspe.evaluation.RadarObjectsEvaluation.PEMultiLogEvaluation import PEMultiLogEvaluation
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.providers.F360MDF_Pandora_DataProvider import F360MDF_Pandora_DataProvider

warnings.filterwarnings('ignore')

"""
Run this script as 'Run File in Console'
Example of evaluating performance when several logs should be evaluated.
This class is configured by data provider and PE Pipeline and use log list
Output is then saved to pickle and can be further proceeded
"""

est_data_dir = r'E:\1k\data\FASETH\complete_data'
sw_version = 'A370'  # 'A' step
host_rear_axle_to_f_bumper = 3.7  # distance from ego vehicle rear axle to front bumper - need for CS transform
lidar_to_f_bumper = 3.29
host_length = 4.7
host_width = 1.9
sdb_f360_dt = 0.0  # time delay between RtRange and F360 which must be compensated
visibility_threshold = 0.3
stationary_threshold = 0.5
cs_system = 'VCS'  # coordinate system in which evaluation will be performed
force_evaluate = False

est_data_dir = Path(est_data_dir)
asp_main_dir = est_data_dir.parent / 'ASP_output'
logger_path = asp_main_dir / 'logger'
sdb_cache_dir = asp_main_dir / 'SDB_cache'
mdf_cache_dir = est_data_dir.parent / 'parse_result' / est_data_dir.name
eval_outputs_dir = asp_main_dir / 'eval_output'
asp_main_dir.mkdir(exist_ok=True)
logger_path.mkdir(exist_ok=True)
eval_outputs_dir.mkdir(exist_ok=True)


def split_list(a, parts):
    part_len = len(a) // parts + 1
    return [a[part_no * part_len:(part_no + 1) * part_len] for part_no in range(parts)]


parts_amount = 2
evaluated_part = 1

start_time = time()
sdb_data_sets = list(sdb_cache_dir.glob('*_sdb_raw.pickle'))
for sdb_data_set in sdb_data_sets.copy():
    m = re.search(r'log_(\d{8}_\d{6})_sdb_raw.pickle', sdb_data_set.name)
    set_id = m.group(1)
    output_path = eval_outputs_dir / f'pe_output_{set_id}.pickle'
    if output_path.exists() and not force_evaluate:
        sdb_data_sets.remove(sdb_data_set)

sdb_data_sets = split_list(sdb_data_sets, parts_amount)
for sdb_data_set in tqdm(sdb_data_sets[evaluated_part - 1], 'SDB Data sets'):
    m = re.search(r'log_(\d{8}_\d{6})_sdb_raw.pickle', sdb_data_set.name)
    set_id = m.group(1)
    output_path = eval_outputs_dir / f'pe_output_{set_id}.pickle'
    log_list = [str(path) for path in est_data_dir.glob(f'*{set_id}*.MF4')]
    data_provider = F360MDF_Pandora_DataProvider(
        set_id=set_id,
        sw_version=sw_version,
        host_rear_axle_to_front_bumper_dist=host_rear_axle_to_f_bumper,
        lidar_to_f_bumper=lidar_to_f_bumper,
        host_width=host_width,
        host_length=host_length,
        coordinate_system=cs_system,
        shift_offset=sdb_f360_dt,
        save_to_file=True,
        stationary_threshold=stationary_threshold,
    )
    pe_pipeline = PEPipeline(
        relevancy_reference_objects=[
            IsVisibleFlag(visibility_threshold),
            IsMovableFlag(),
        ],
        relevancy_estimated_objects=[
            IsMovableFlag(),
        ],
        association=RelEstToRefNN(distance_function=ComplexMahalanobisDistance(scale_x=1 / 2)),
    )

    multi_log_eval = PEMultiLogEvaluation(
        data_provider=data_provider,
        single_log_pipeline=pe_pipeline,
        logging_file_folder=str(logger_path),
    )
    pe_output = multi_log_eval.process_data(log_list)
    pkl_module.save(output_path, pe_output)

finish_time = time()
duration_time = round(finish_time - start_time, 2)
print(f'Duration: {duration_time} s')
