from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig

'''
User should specify his own config either by passing user-specific values as arguments in Nexus50kEvaluationConfig
or by  creating environment variables for:
 NEXUS_USER_NAME -  email address registered on https://nexus.aptiv.com/v2/auth
 ASPERA_USER_NAME -  netid
 MDF2MUDP_CONV_PATH - absolute path to mdf2dvl_mudp.exe , i.e. C:/Users/netid/Repo/AGFS_Tools/mdf2dvl_mudp/output/Debug_x64/mdf2dvl_mudp.exe
'''

user_config = Nexus50kEvaluationConfig(
    main_cache_dir=None,
    mdf2mudp_converter_path=None,
    nexus_user_name=None,
    aspera_user_name=None,
)
