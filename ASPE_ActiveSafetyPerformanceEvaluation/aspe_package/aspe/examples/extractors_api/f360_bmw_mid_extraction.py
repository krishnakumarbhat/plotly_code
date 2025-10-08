if __name__ == '__main__':
    """
    --------------------------------------------------------------------------------------------------------------------
    SINGLE LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract F360 data from .mf4 log file and few usages of different parameters. 
    First 'extract_f360_bmw_mid_from_mf4' needs to be imported:
    """
    from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4

    log_path = r"C:\logs\log_name.mf4"

    """
    EXAMPLE 1 
    Extract object list and object list header data from given log. After parsing .mgp file with parsed data will be
    saved in log's directory.
    """
    extracted = extract_f360_bmw_mid_from_mf4(log_path)

    """
    EXAMPLE 2 
    Same as example 1, but extract raw signals also.
    """
    extracted = extract_f360_bmw_mid_from_mf4(log_path, raw_signals=True)

    """
    EXAMPLE 3 
    Extract object list and object list header data from given log. Don't save .mgp file after parsing. 
    """
    extracted = extract_f360_bmw_mid_from_mf4(log_path, save_mgp_parsed_data=False)

    """
    EXAMPLE 4 
    Extract object list and object list header data from given log. Save results to .pickle file.
    """
    extracted = extract_f360_bmw_mid_from_mf4(log_path, save_to_file=True)

    """
    EXAMPLE 5 
    If there is already saved .mgp file with parsed data, function will try to load it instead of parsing. To prevent 
    that and do parsing instead use below call. But this is much slower than loading .mgp file, so keep that in mind. 
    """
    extracted = extract_f360_bmw_mid_from_mf4(log_path, force_parse=True)

    """
    --------------------------------------------------------------------------------------------------------------------
    
    MULTI LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract F360 data from all .mf4 log files located in given folder. Basically all
    arguments passed to 'extract_f360_bmw_mid_from_mf4' are also available in 'extract_f360_bmw_mid_from_mf4_folder' 
    function. First 'extract_f360_bmw_mid_from_mf4_folder' needs to be imported:
    """
    from aspe.extractors.API.mdf import extract_f360_bmw_mid_from_mf4_folder

    mf4_folder_path = r'C:\logs\some_mf4_folder'

    """
    EXAMPLE 6 
    Extract object list and object list header data from logs in given folder. After parsing .mgp file with parsed data 
    will be saved in log's directory and extracted output will be saved as .pickle files. 
    """
    extracted = extract_f360_bmw_mid_from_mf4_folder(mf4_folder_path)

    """
    EXAMPLE 7 
    Same as example 6, but after parsing there will be also .pickle files saved with extracted output.
    """
    extracted = extract_f360_bmw_mid_from_mf4_folder(mf4_folder_path, save_to_file=False)

    """
    EXAMPLE 8
    Extract data only from logs which names contain substrings 001 or 002. 
    """
    extracted = extract_f360_bmw_mid_from_mf4_folder(mf4_folder_path, required_path_sub_strings=['001', '002'])

    """
    EXAMPLE 9
    Same as example 8, but do NOT extract logs which names contain substrings 003 or 004
    """
    extracted = extract_f360_bmw_mid_from_mf4_folder(mf4_folder_path, restricted_path_sub_strings=['003', '004'])

    """
    EXAMPLE 10
    Extract logs in given folder and look also in also in directory sub-folders and their sub-folders. 
    Extract only logs which names contain substring '2_20'. It is useful when logs are resimmed with newest release and 
    we want to extract logs within some folder, but only for this specific resim extension. 
    folder_search_level=0 is default argument - it means that only logs in given directory are extracted. 
    folder_search_level=1 means that logs in given directory and logs in subfolders in directory are extracted
    folder_search_level=2 means that logs in given directory and logs in subfolders in directory and logs in
    subfolders of directory subfolders are extracted, and so on ... 
    """
    extracted = extract_f360_bmw_mid_from_mf4_folder(mf4_folder_path, required_path_sub_strings=['2_20'],
                                                     folder_search_level=2)
