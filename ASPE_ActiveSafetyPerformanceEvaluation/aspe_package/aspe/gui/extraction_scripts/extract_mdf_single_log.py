from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4

"""
Helper script which uses aspe.extractors API to parse and extract someip data from single mf4 file.
Check extract_f360_bmw_mid_from_mf4 function docstring to see what type of arguments it's take.
"""

mdf_log_file_path = r"C:\logs\VTV_mf4_logs\A370\DS_01_target_overtaking\
BN_FASETH\MID_ECU_3.14.210_S_3.14.106_TRA_DS_01_30_40_L_BN_FASETH_WBATR91070LC63638_20200617_164034_fas_0005.MF4"
sw_version = "A450"  # A-step

extracted = extract_f360_bmw_mid_from_mf4(mdf_log_file_path,
                                          sw_version=sw_version,
                                          save_to_file=True,
                                          raw_signals=True)

"""Now in log directory there will be 
LOG_NAME_f360_mf4_bmw_mid_extracted.pickle file created. Now it could be loaded to ASPE GUI"""