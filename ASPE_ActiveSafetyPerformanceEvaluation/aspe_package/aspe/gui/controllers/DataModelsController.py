from pathlib import Path

from aspe.extractors.API.keg import extract_env_from_keg
from aspe.extractors.API.mudp import extract_f360_from_mudp
from aspe.extractors.API.xtrk import extract_f360_from_xtrk
from aspe.gui.models.DataModel import DataModel
from aspe.utilities.SupportingFunctions import load_from_pkl


class DataModelsManager:
    def __init__(self):
        self.data_models = []

    def load_data_using_source_info(self, source_info):
        if (source_info.pickle_path is not None) and (Path(source_info.pickle_path).exists()):
            extracted = load_from_pkl(source_info.pickle_path)
        else:
            extracted = self.extract_data(source_info)
        data_model = DataModel(extracted, source_info)
        self.data_models.append(data_model)
        return data_model

    def get_model_using_source_info(self, source_info):
        for data_model in self.data_models:
            if data_model.source_info == source_info:
                return data_model
        return None

    def remove_data(self, data_model):
        for dm in self.data_models:
            if dm is data_model:
                self.data_models.remove(data_model)
                break

    def remove_all_data(self):
        self.data_models = []

    def extract_data(self, source_info):
        # TODO FZD-379: this should call class which use some chosen configuration for extraction -
        #  user should be able to change extraction configuration in some pop up window or in some panel
        extracted = None
        log_path = Path(source_info.log_file_path)
        if log_path.exists():
            extension = log_path.suffix.lower()
            if "mudp" in extension:
                extracted = extract_f360_from_mudp(mudp_log_path=str(log_path), source_to_parse=35,
                                                   raw_signals=True, save_to_file=True,
                                                   oal_objects=True, internal_objects=True, detections=True,
                                                   sensors=True, stat_env=True)
            elif "xtrk" in extension:
                extracted = extract_f360_from_xtrk(xtrk_path=str(log_path), raw_signals=True, save_to_file=True,
                                                   sensors=True, detections=True, clusters=True, stat_env=True)
            # TODO: FZE-400 BEFORE MERGE change force_extract and save_to_file flags in keg and bin before merge
            elif "keg" in extension:
                extracted = extract_env_from_keg(keg_path=str(log_path),
                                                 raw_signals=True,
                                                 save_to_file=False,
                                                 force_extract=True)
            elif "mf4" in extension:
                # TODO FZD-379: now version is hardcoded which is nasty - create some configuration
                #  handling mentioned above
                try:
                    from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4
                    extracted = extract_f360_bmw_mid_from_mf4(str(log_path),
                                                              sw_version="A450",
                                                              raw_signals=True,
                                                              save_to_file=True)
                except ModuleNotFoundError:
                    print("mdf_parser_pkg not found - please install to make BN_FASETH extraction work")
        return extracted
