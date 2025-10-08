import os
from time import time

from aspe.evaluation.RadarObjectsEvaluation.Flags import IsMovableFlag, SignalEqualityFlag
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers.gdsr_bin_dspace_bin_data_provider import GdsrBinDSpaceBinDataProvider

"""
Example of single log evaluation for GDSR tracker output with dSpace ground-truth.
"""

log_name = "Tracker_UC_10_6_E0_T0_DtoT05_I20_lm2"
estimated_data_root_dir = os.path.join(
    ExampleData.gdsr_dspace_data_path,
    "Tracker_UC_10_6_E0_T0_DtoT05_I20_B0",
    "FRONT_RIGHT",
)
# With dSpace data the REAR_LEFT sensor is the only one that produces the ground-truth.
# This is pretty weird, and we should ask the resim for this to be changed.
reference_data_root_dir = os.path.join(
    ExampleData.gdsr_dspace_data_path,
    "Tracker_UC_10_6_E0_T0_DtoT05_I20_B0",
    "REAR_LEFT",
)

t1 = time()
data_provider = GdsrBinDSpaceBinDataProvider(save_to_file=False)
estimated_data, reference_data = data_provider.get_single_log_data(
    estimated_data_root_dir,
    reference_data_root_dir,
    log_name,
)

t2 = time()
pe = PEPipeline(
    relevancy_estimated_objects=IsMovableFlag(),
    relevancy_reference_objects=[IsMovableFlag(), SignalEqualityFlag("f_any_ref_points_in_fov", True)],
)
output = pe.evaluate(estimated_data, reference_data)
t3 = time()
print("Providing Data: ", t2 - t1)
print("Evaluation: ", t3 - t2)
