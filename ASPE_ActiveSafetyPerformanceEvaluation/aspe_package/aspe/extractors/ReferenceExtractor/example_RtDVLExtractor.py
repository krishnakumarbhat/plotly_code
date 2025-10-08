from aspe.extractors.ReferenceExtractor.RtDVLExtractor import RtDVLExtractor
from aspe.utilities.SupportingFunctions import load_from_pkl, save_to_pkl


def example():
    """
    main rt extraction example
    :return:
    """
    import time

    from aspe.parsers.DvlParser.dvl_parser import DVLParser
    dvl_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\aspe.extractors_refactoring\w_ref\rRf360t4010304v202r1\FTP500_TC2_D30_134857_001_rRf360t4010304v202r1p50.dvl"
    dvl_config_path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\aspe.extractors_refactoring\parsers_config\dvl_data_parser_config.json '
    dvl_parser = DVLParser(dvl_path, dvl_config_path)
    dvl_parser.decode()
    dvl_parser.extract_message_sets()
    t1 = time.time()
    rt_extractor = RtDVLExtractor()
    rt_data = rt_extractor.extract_data(dvl_parser.__dict__)
    t2 = time.time()
    print(f'RtRange extraction from DVL file time = {t2-t1}')
    return rt_data


def example_save_n_load():
    """
    example function that using basic example, saving and loading data after
    :return: loaded rt data
    """
    dvl_path = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\aspe.extractors_refactoring\w_ref\rRf360t4010304v202r1\FTP500_TC2_D30_134857_001_rRf360t4010304v202r1p50.dvl"

    rt_data = example()

    save_path = dvl_path.replace('.dvl', '_DVL_Extracted.pickle')
    save_to_pkl(rt_data, save_path)

    loaded_rt_data = load_from_pkl(save_path)
    return loaded_rt_data


if __name__ == "__main__":
    rt_data = example()
    #example_save_n_load()
