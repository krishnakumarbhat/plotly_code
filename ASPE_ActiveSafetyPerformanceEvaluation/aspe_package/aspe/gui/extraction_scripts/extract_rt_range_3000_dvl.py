from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl

"""
Helper script which uses aspe.extractors API to parse and extract RTRange3000 data from single .dvl file.
Check extract_rt_range_3000_from_dvl function docstring to see what type of arguments it's take.
"""

dvl_file_path = r"PATH\TO\DVL\FILE.dvl"
extracted = extract_rt_range_3000_from_dvl(dvl_file_path, save_to_file=True, raw_signals=True)

"""Now in log directory there will be 
LOG_NAME_rt_range_3000_dvl_extracted.pickle files are created. Now it could be loaded to ASPE GUI"""