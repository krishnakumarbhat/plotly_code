from datetime import datetime

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluation import Nexus50kEvaluation, NexusEvent
from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.utilities.nexus50k_events_finder.pe_piepelines_setup import get_default_pipeline_for_50k_eval

if __name__ == '__main__':
    """
    Example script of evaluation f360 resimulated data with auto-ground truth produced from lidar data taken from
    50k drives stored in Nexus. User specifies:
    1. PEPipeline - configured performance evaluation pipeline object
    2. Evaluation config -  all user specific inputs which are needed for automation like local dir for caching files,
                            credentials for ASPERA and Nexus etc.
    3. Event - part of the nexus log which is evaluated
    4. Resim .exe path - path to resim .exe which will be used for resimulation
    5. Resim suffix - suffix which will be added to resimmed files

    """

    eval_config = Nexus50kEvaluationConfig(
        main_cache_dir=r'C:\logs\50k_cache',
        mdf2mudp_converter_path=r"C:\wkspaces_git\AGFS_Tools\mdf2dvl_mudp\output\Release_x64\mdf2dvl_mudp.exe",
        nexus_user_name='jakub.wrobel@aptiv.com',
        aspera_user_name='zj2hns',
    )
    pe_pipeline = get_default_pipeline_for_50k_eval()

    evaluation = Nexus50kEvaluation(config=eval_config, pe_pipeline=pe_pipeline)

    # Evaluation inputs
    event = NexusEvent(
        name='traffic_jam',
        nexus_log_id="5f665646b9a83273056a84ee",
        tmin=datetime(2020, 7, 28, 13, 43, 20, 0),  # given in UTC
        tmax=datetime(2020, 7, 28, 13, 44, 30, 0),
    )

    resim_exe_path = r"C:\wkspaces_git\F360Core\sw\output\Release_Win32\resim_f360.exe"
    resim_suffix = '_nexus_eval_test'
    resim_mydir = 'output'
    pe_out = evaluation.evaluate_f360_resim_with_nexus_event(event, resim_exe_path, resim_suffix, resim_mydir)
