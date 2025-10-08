from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4_folder

"""
Helper script which uses aspe.extractors API to parse and extract someip data from mf4 files stored in given directory.
Check extract_f360_bmw_mid_from_mf4 function docstring to see what type of arguments it's take.
"""

mdf_folder_path = r"PATH\TO\FOLDER\WITH\MDF\FILES"
sw_version = "A450"  # A-step

extracted = extract_f360_bmw_mid_from_mf4_folder(mdf_folder_path,
                                                 sw_version=sw_version,
                                                 save_to_file=True,
                                                 raw_signals=True)

"""Now in log directory there will be 
LOG_NAME_f360_mf4_bmw_mid_extracted.pickle files are created. Now it could be loaded to ASPE GUI"""