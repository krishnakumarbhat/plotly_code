from pathlib import Path

import pandas as pd

from aspe.gui.models.DataSourceInfo import DataSourceInfo
from aspe.utilities.SupportingFunctions import (
    create_pickle_path,
    is_log_newer_than_pickle,
    iterate_parents_and_find_directory,
)


class AvailableDataSources:
    # TODO FZD-375: this class is garbage. Create Log class and subclasses like MudpLog, XtrkLog etc.
    #  and move logic there
    def __init__(self, log_path):
        self.log_path = log_path
        self.sources_info = pd.DataFrame(columns=["file_path", "pickle_path", "root_folder", "type"])
        self._fill_sources_info()

    def _fill_sources_info(self):
        associated_files_paths = self._find_associated_log_files()
        for assoc_file_path in associated_files_paths:
            file_path = Path(assoc_file_path)
            log_type = self._get_log_type(file_path)
            if log_type is not None:
                pickle_patch = self.find_associated_pickle_path(file_path)
                if log_type == "PE output 50k events":
                    root_folder = str(file_path.stem).split("_events_")[-1]  # dirty fix to distinguish different resims
                else:
                    root_folder = str(file_path.parent.stem)
                self.sources_info = self.sources_info.append({"file_path": str(file_path),
                                                              "pickle_path": pickle_patch,
                                                              "root_folder": root_folder,
                                                              "type": log_type}, ignore_index=True)
        self._find_reference_data_pickle_paths()

    def _find_associated_log_files(self):
        log_path = Path(self.log_path)
        log_extension = log_path.suffix
        path_phrase_to_search = log_path.stem + "*" + log_extension
        associated_files = [str(p) for p in log_path.parent.rglob(path_phrase_to_search)]
        if log_extension != ".xtrk":
            associated_files += [str(p) for p in log_path.parent.rglob(log_path.stem + "*" + "xtrk")]
        if log_extension == ".pickle" and "_events_" in log_path.stem:
            event_name, rest = log_path.stem.split("_events_")
            event_number = int(rest.split("_")[0])
            associated_files = [str(p) for p in log_path.parent.glob(f"{event_name}_events_{event_number}_*.pickle")]
        return associated_files

    def _find_reference_data_pickle_paths(self):
        log_path = Path(self.log_path)
        log_stem = log_path.stem
        if log_path.suffix == ".mudp" and "_deb_" in log_stem:
            srr_reference_dir = iterate_parents_and_find_directory(log_path.parent, "SRR_REFERENCE", 3)
            if srr_reference_dir is not None:
                srr_ref_file_pattern = log_stem.replace("SRR_DEBUG", "SRR_REFERENCE").replace("deb", "ref") + "*.pickle"
                ref_paths = list(srr_reference_dir.glob(srr_ref_file_pattern))
                for ref_path in ref_paths:
                    pickle_type = self._get_pickle_file_type(str(ref_path))
                    self.sources_info = self.sources_info.append({"file_path": str(ref_path),
                                                                  "pickle_path": str(ref_path),
                                                                  "root_folder": ref_path.parent.stem,
                                                                  "type": pickle_type}, ignore_index=True)

    def get_source_info_using_dict(self, source_info_dict):
        root_folder = self.sources_info.loc[:, "root_folder"]
        type = self.sources_info.loc[:, "type"]
        root_folder_mask = root_folder == source_info_dict["root_folder"]
        type_mask = type == source_info_dict["type"]
        choosen_source = self.sources_info.loc[root_folder_mask & type_mask, :].iloc[0, :]
        return DataSourceInfo(choosen_source.file_path,
                              choosen_source.pickle_path,
                              choosen_source.root_folder,
                              choosen_source.type)


    def _get_log_type(self, file_path):
        log_extension = file_path.suffix.lower()
        type = None
        if log_extension == ".mudp":
            type = "F360 .mudp data"
        elif log_extension == ".xtrk":
            type = "F360 .xtrk data"
        elif log_extension == ".mf4" and "_FASETH" in self.log_path:
            type = "F360 .mf4 BMW mid data"
        elif log_extension == ".bin":
            type = ".bin data"
        elif log_extension == ".keg":
            type = ".keg data"
        elif log_extension == ".pickle":
            if "_events" in file_path.stem and "pe_output" in file_path.stem:
                type = "PE output 50k events"
            elif "pe_output" in file_path.stem:
                type = "PE output"
            elif "_nexus" in file_path.stem:
                type = "NEXUS output"
            elif "_bin" in file_path.stem:
                type = "BIN output"
            else:
                type = "undefined output"

        return type

    def _get_pickle_file_type(self, pickle_path):
        if "_f360_mudp_extracted" in pickle_path:
            pickle_type = "F360 .mudp data"
        elif "_f360_mf4_bmw_mid_extracted" in pickle_path:
            pickle_type = "F360 .mf4 BMW mid data"
        elif "_rt_range_3000_dvl_extracted" in pickle_path:
            pickle_type = "RTRange3000 .dvl data"
        elif "_rt_range_3000_mdf_extracted" in pickle_path:
            pickle_type = "RTRange3000 .mf4 data"
        elif "pe_output" in pickle_path:
            pickle_type = "ASPE output"
        elif "_sdb_extracted" in pickle_path:
            pickle_type = "Pandora SDB data"
        elif "_f360_xtrk_extracted" in pickle_path:
            pickle_type = "F360 .xtrk data"
        elif "_nexus_extracted" in pickle_path:
            pickle_type = "Nexus data"
        elif "_bin" in pickle_path:
            pickle_path = "BIN data"
        else:
            pickle_type = "undefined"

        if pickle_path and ("_sync" in pickle_path):
            pickle_type += " synchronized"
        return pickle_type

    def find_associated_pickle_path(self, file_path):
        log_extension = file_path.suffix.lower()
        if log_extension == ".mudp":
            matches = list(file_path.parent.glob(f"{file_path.stem}_f360_mudp_extracted.pickle"))
        elif log_extension == ".xtrk":
            matches = list(file_path.parent.glob(f"{file_path.stem}_f360_xtrk_extracted.pickle"))
        elif log_extension == ".mf4" and "_FASETH" in self.log_path:
            matches = list(file_path.parent.glob(f"{file_path.stem}_f360_mf4_bmw_mid_extracted.pickle"))
        elif log_extension == ".pickle":
            return file_path
        else:
            return None
        if matches:
            pickle_path = str(matches[0])
            if is_log_newer_than_pickle(file_path, create_pickle_path(file_path)):
                print("Pickle file is older than original log")
                return None
            return pickle_path
        return None