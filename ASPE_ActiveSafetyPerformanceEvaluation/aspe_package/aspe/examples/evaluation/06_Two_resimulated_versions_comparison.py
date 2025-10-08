from time import time

from aspe.evaluation.RadarObjectsEvaluation.Flags.AllTrueFlag import AllTrueFlag
from aspe.evaluation.RadarObjectsEvaluation.PairsPreBuilders.PEPairedObjectsPreBuilderGating import (
    PEPairedObjectsPreBuilderGating,
)
from aspe.evaluation.RadarObjectsEvaluation.PEPipeline import PEPipeline
from aspe.examples.evaluation.private.user_data import ExampleData
from aspe.providers.extraction_pipelines import F360MudpExtractionPipeline

"""
Run this script as 'Run File in Console'
Example shows how to compare two resimulated version. 
Please not that in this example reference data is resimulated data baseline and estimated data is after some
changes were applied - in this particular example two SW version were compared
Please note as well that default relevancy filter were changed to evaluate all objects.
Also gating threshold is decreased to find out for which object there were any changes - they are marked as:
* FN - baseline.
* FP - after changes
This kind of script can be used for example for refactoring verification - where no changes are expected
"""

baseline_log = (ExampleData.rt_range_data_path +
                r"\RCTA\rRf360t4060306v204p50_2_12\FTP402_TC4_TT_155253_001_rRf360t4060306v204p50_2_12.mudp")
log_after_changes = (ExampleData.rt_range_data_path +
                     r"\RCTA\rRf360t4100309v205p50_2_15\FTP402_TC4_TT_155253_001_rRf360t4100309v205p50_2_15.mudp")
mudp_stream_def_path = ExampleData.mudp_stream_def_path

mudp_extraction_pipeline = F360MudpExtractionPipeline(mudp_stream_def_path)

gating = PEPairedObjectsPreBuilderGating(distance_threshold=0.01)
relevancy = AllTrueFlag()
pe = PEPipeline(pairs_builder=gating, relevancy_estimated_objects=relevancy, relevancy_reference_objects=relevancy)

t1 = time()
estimated_data = mudp_extraction_pipeline.process(baseline_log)
reference_data = mudp_extraction_pipeline.process(log_after_changes)
t2 = time()

output = pe.evaluate(estimated_data, reference_data)
t3 = time()

print('Providing Data: ', t2 - t1)
print('Evaluation: ', t3 - t2)
