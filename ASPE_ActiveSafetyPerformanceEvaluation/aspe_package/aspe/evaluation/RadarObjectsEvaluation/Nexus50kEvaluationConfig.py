import os
from pathlib import Path

import keyring


class Nexus50kEvaluationConfig:
    def __init__(self,
                 main_cache_dir: str,
                 mdf2mudp_converter_path: str = None,
                 nexus_user_name: str = None,
                 aspera_user_name: str = None,
                 perf_eval_out_dir: str = None,
                 nexus_to_aspera_table_name: str = 'vtv_high_endurance_run_nexus_mapping.csv',
                 aspera_dir_on_hpc: str = '/net/8k3/e0fs01/irods/PLKRA-PROJECTS'):
        '''
        Class which stores main configuration for 50k Nexus evaluation. Class uses keyring library to get passwords
        for ASPERA and Nexus.
        :param main_cache_dir: main direction for cached files.
        :param mdf2mudp_converter_path: path to mdf2dvl_mudp converter .exe
        :param nexus_user_name: Nexus account user name - must be created on https://nexus.aptiv.com/v2/auth
        :param aspera_user_name: ASPERA user name - should be just netid
        :param nexus_to_aspera_table_name: name of table with linkage between SRR_DEBUG logs on hpcc and nexus logs ids
        '''

        self.DATA_50K_CACHE_DIR = main_cache_dir

        if mdf2mudp_converter_path is None:
            conv_path = os.environ.get('MDF2MUDP_CONV_PATH')
            self.MDF2MUDP_CONV_PATH = Path(conv_path) if conv_path is not None else None
        else:
            self.MDF2MUDP_CONV_PATH = mdf2mudp_converter_path

        if nexus_user_name is None:
            self.NEXUS_USER_NAME = os.environ.get('NEXUS_USER_NAME')
        else:
            self.NEXUS_USER_NAME = nexus_user_name

        if aspera_user_name is None:
            self.ASPERA_USER_NAME = os.environ.get('ASPERA_USER_NAME')
        else:
            self.ASPERA_USER_NAME = aspera_user_name

        self.ASPERA_DIR_ON_HPC = aspera_dir_on_hpc
        self.NEXUS_TO_ASPERA_TABLE_NAME = nexus_to_aspera_table_name
        self.NEXUS_TO_SRR_DEB_TABLE_ASPERA_DIR = os.path.join('/BMW-SRR5/1-Raw/VTV/', f'{self.NEXUS_TO_ASPERA_TABLE_NAME}')
        self.NEXUS_PASSWORD = keyring.get_password('nexus', self.NEXUS_USER_NAME)
        self.ASPERA_PASSWORD = keyring.get_password('aspera', self.ASPERA_USER_NAME)

        if self.DATA_50K_CACHE_DIR:
            self.NEXUS_CACHE_DIR = os.path.join(self.DATA_50K_CACHE_DIR, r'nexus_cache')
            self.SRR_DEBUG_CACHE_DIR = os.path.join(self.DATA_50K_CACHE_DIR, r'srr_debug_cache')
            if perf_eval_out_dir is None:
                self.PERF_EVAL_OUT_DIR = os.path.join(self.DATA_50K_CACHE_DIR, r'perf_eval_out')
            else:
                self.PERF_EVAL_OUT_DIR = perf_eval_out_dir

            # create cache dirs if not created
            Path(self.DATA_50K_CACHE_DIR).mkdir(exist_ok=True, parents=True)
            Path(self.NEXUS_CACHE_DIR).mkdir(exist_ok=True, parents=True)
            Path(self.SRR_DEBUG_CACHE_DIR).mkdir(exist_ok=True, parents=True)
            Path(self.PERF_EVAL_OUT_DIR).mkdir(exist_ok=True, parents=True)
        else:
            self.NEXUS_CACHE_DIR = None
            self.SRR_DEBUG_CACHE_DIR = None
            self.PERF_EVAL_OUT_DIR = None

