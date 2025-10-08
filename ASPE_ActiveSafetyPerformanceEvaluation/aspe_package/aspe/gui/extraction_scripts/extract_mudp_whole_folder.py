from aspe.extractors.API.mudp import extract_f360_from_mudp_folder

"""
Helper script which uses aspe.extractors API to parse and extract data from files located in specified directory.
Check extract_f360_from_mudp_folder function docstring to see what type of arguments it's take.
"""

mudp_stream_def_path = r"" # if None parser looks for MUDP_STREAM_DEFINITIONS_PATH environmental variable,
mudp_log_folder_path = r""

extracted = extract_f360_from_mudp_folder(mudp_log_folder_path, mudp_stream_def_path=mudp_stream_def_path,
                                          save_to_file=True, internal_objects=True, oal_objects=True, sensors=True,
                                          detections=True, raw_signals=True, folder_search_level=4, force_extract=False)

"""
Now in log directory there will be LOG_NAME_f360_mudp_extracted.pickle file created. Now it could be loaded to ASPE GUI
"""