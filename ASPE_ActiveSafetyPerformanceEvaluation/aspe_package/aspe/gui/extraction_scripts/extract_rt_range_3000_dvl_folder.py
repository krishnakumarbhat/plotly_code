from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl_folder

"""
Helper script which uses aspe.extractors API to parse and extract RTRange3000 data from .dvl files stored in 
give directory. Check extract_rt_range_3000_from_dvl_folder function docstring to see what type of arguments it's take.
"""

dvl_folder_path = r"PATH\TO\FOLDER\WITH\DVL\FILES"
extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path, save_to_file=True, raw_signals=True)

"""Now in log directory there will be 
LOG_NAME_rt_range_3000_dvl_extracted.pickle files are created. Now it could be loaded to ASPE GUI"""