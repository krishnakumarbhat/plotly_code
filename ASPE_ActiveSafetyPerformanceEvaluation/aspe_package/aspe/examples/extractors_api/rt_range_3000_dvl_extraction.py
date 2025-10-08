if __name__ == '__main__':
    """
    --------------------------------------------------------------------------------------------------------------------
    SINGLE LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract RtRange3000 from .dvl log file and few usages of different parameters. 
    Example logs with RTRange data can be found: 
    https://aspera-emea.aptiv.com/browse/shares/136?path=/RNA_SRR5/Opole_CW19.
    First 'extract_rt_range_3000_from_dvl' needs to be imported:
    """
    from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl

    log_path = r"C:\logs\log_name.dvl"

    """
    EXAMPLE 1 
    Extract object list and object list header data from given log. After parsing .mgp file with parsed data will be
    saved in log's directory.
    """
    extracted = extract_rt_range_3000_from_dvl(log_path)

    """
    EXAMPLE 2 
    Same as example 1, but save .pickle file also.
    """
    extracted = extract_rt_range_3000_from_dvl(log_path, save_to_file=True)

    """
    EXAMPLE 3 
    Same as example 2, but extract raw signals also.
    """
    extracted = extract_rt_range_3000_from_dvl(log_path, save_to_file=True, raw_signals=True)

    """
    --------------------------------------------------------------------------------------------------------------------
    
    MULTI LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract RtRange3000 from .dvl log files located in given folder. Basically all
    arguments passed to 'extract_rt_range_3000_from_dvl' are also available in 'extract_rt_range_3000_from_dvl_folder' 
    function. First 'extract_rt_range_3000_from_dvl_folder' needs to be imported:
    """
    from aspe.extractors.API.dvl import extract_rt_range_3000_from_dvl_folder

    dvl_folder_path = r'C:\logs\some_dvl_folder'

    """
    EXAMPLE 4 
    Extract object list and object list header data from logs in given folder. After parsing .mgp file with parsed data 
    will be saved in log's directory.
    """
    extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path)

    """
    EXAMPLE 5 
    For folder extraction saving .pickle file is default. To prevent that:
    """
    extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path, save_to_file=False)

    """
    EXAMPLE 6
    Extract data only from logs which names contain substrings 001 or 002. 
    """
    extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path, required_path_sub_strings=['001', '002'])

    """
    EXAMPLE 7
    Same as example 8, but do NOT extract logs which names contain substrings 003 or 004
    """
    extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path, restricted_path_sub_strings=['003', '004'])

    """
    EXAMPLE 8
    Extract logs in given folder and look also in also in directory sub-folders and their sub-folders. 
    Extract only logs which names contain substring '2_20'. It is useful when logs are resimmed with newest release and 
    we want to extract logs within some folder, but only for this specific resim extension. 
    folder_search_level=0 is default argument - it means that only logs in given directory are extracted. 
    folder_search_level=1 means that logs in given directory and logs in subfolders in directory are extracted
    folder_search_level=2 means that logs in given directory and logs in subfolders in directory and logs in
    subfolders of directory subfolders are extracted, and so on ... 
    """
    extracted = extract_rt_range_3000_from_dvl_folder(dvl_folder_path, required_path_sub_strings=['2_20'],
                                                      folder_search_level=2)
