if __name__ == '__main__':
    """
    SINGLE LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract F360 data from .mudp log files. To achieve that API function 
    extract_f360_from_mudp will be used. Import it using:
    """
    from aspe.extractors.API.mudp import extract_f360_from_mudp

    log_path = r"C:\logs\log_name.mudp"
    mudp_stream_definition_path = r"C\some_path\mudp_stream_definition_folder"

    """
    EXAMPLE 1 
    Extract defaults data sets - tracker info, host and reduced objects.
    """
    extracted = extract_f360_from_mudp(log_path)

    """
    EXAMPLE 2 
    Extract defaults data sets - tracker info, host and reduced objects. Mudp parser needs mudp stream definition path.
    By default parser uses environmental variable MUDP_STREAM_DEFINITIONS_PATH, but there is also way to give stream 
    definition folder path.
    """
    extracted = extract_f360_from_mudp(log_path, mudp_stream_def_path=mudp_stream_definition_path)

    """
    EXAMPLE 3 
    Extract defaults + all objects and detections data sets. Do not extract reduced objects. 
    """
    extracted = extract_f360_from_mudp(log_path, mudp_stream_def_path=mudp_stream_definition_path,
                                       objects=False, internal_objects=True, detections=True)

    """
    EXAMPLE 4 
    Same as example 2, but also save .pickle file with extracted results.
    After extraction file 'log_name_f360_mudp_extracted.pickle' will be created within log file directory. 
    """
    extracted = extract_f360_from_mudp(log_path, mudp_stream_def_path=mudp_stream_definition_path,
                                       objects=False, internal_objects=True, detections=True, save_to_file=True)

    """
    Example 5
    Load save .pickle file
    """
    from aspe.utilities.SupportingFunctions import load_from_pkl
    extracted = load_from_pkl(log_path.replace('.mudp', '_f360_mudp_extracted.pickle'))

    """
    EXAMPLE 6 
    Extract only tracker info and reduced objects (which are defaults). Save output to pickle file. 
    """
    extracted = extract_f360_from_mudp(log_path, mudp_stream_def_path=mudp_stream_definition_path,
                                       host=False, save_to_file=True)

    """
    MULTI LOG EXTRACTION EXAMPLES
    Next few examples will show you how extract F360 data from all .mudp log files located in given folder. 
    To achieve that API function 'extract_f360_from_mudp_folder' will be used. Import it using:
    """
    from aspe.extractors.API.mudp import extract_f360_from_mudp_folder
    mudp_folder_path = r'C:\logs\some_mudp_folder'

    """
    EXAMPLE 7 
    Extract defaults data sets - tracker info, host and reduced objects from all .mudp files within given folder. For
    this function saving .pickle files is default. 
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path)

    """
    EXAMPLE 8
    Extract defaults data sets - tracker info, host and reduced objects from all .mudp files within given folder. Force
    not saving .pickle files. 
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path,
                                              save_to_file=False)

    """
    EXAMPLE 9 
    Extract defaults + all objects and detections data sets. Do not extract reduced objects. Output will be saved to
    pickle files.
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path,
                                              objects=False, internal_objects=True, detections=True)

    """
    EXAMPLE 10
    Same as example 8, but extract only from logs which names contain substrings 001 or 002. 
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path,
                                              objects=False, internal_objects=True, detections=True,
                                              required_path_sub_strings=['001', '002'])

    """
    EXAMPLE 11
    Same as example 8, but do NOT extract logs which names contain substirngs 003 or 004
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path,
                                              objects=False, internal_objects=True, detections=True,
                                              restricted_path_sub_strings=['003', '004'])

    """
    EXAMPLE 12
    Same as example 8, but look also in directory sub-folders and their sub-folders. Extract only logs which names 
    contain substring '2_20'. It is useful when logs are resimmed with newest release and we want to extract all logs
    within some folder, but only for this specific resim extension. 
    folder_search_level=0 is default argument - it means that only logs in given directory are extracted. 
    folder_search_level=1 means that logs in given directory and logs in subfolders in directory are extracted
    folder_search_level=2 means that logs in given directory and logs in subfolders in directory and logs in
    subfolders of directory subfolders are extracted, and so on ... 
    """
    extracted = extract_f360_from_mudp_folder(mudp_folder_path, mudp_stream_def_path=mudp_stream_definition_path,
                                              objects=False, internal_objects=True, detections=True,
                                              required_path_sub_strings=['2_20'], folder_search_level=2)