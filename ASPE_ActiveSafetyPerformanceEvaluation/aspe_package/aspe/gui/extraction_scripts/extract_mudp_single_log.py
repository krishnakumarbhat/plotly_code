from aspe.extractors.API.mudp import extract_f360_from_mudp

"""
Helper script which uses aspe.extractors API to parse and extract data from single mudp file.
Check extract_f360_from_mudp function docstring to see what type of arguments it's take.
"""

# if None parser looks for MUDP_STREAM_DEFINITIONS_PATH environmental variable,
mudp_stream_def_path = "C:\wkspaces_git\F360Core\sw\zResimSupport\stream_definitions"
mudp_log_file_path = r"C:\logs\BYK_589_DEX_530\SRR_DEBUG\20200131T114558_20200131T114618_543078_LB36408_SRR_DEBUG.mudp"

extracted = extract_f360_from_mudp(mudp_log_file_path,
                                   mudp_stream_def_path=mudp_stream_def_path,
                                   save_to_file=True,
                                   oal_objects=True,
                                   internal_objects=True,
                                   sensors=True,
                                   detections=True,
                                   raw_signals=True)

"""Now in log directory there will be 
LOG_NAME_f360_mudp_extracted.pickle file created. Now it could be loaded to ASPE GUI"""