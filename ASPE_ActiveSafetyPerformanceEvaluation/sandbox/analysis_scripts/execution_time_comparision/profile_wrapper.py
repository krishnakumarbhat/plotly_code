from extract_mudp_from_path import extract_mudp_from_path
from AptivDataExtractors.F360.F360MUDPExtractor import F360Extractor


def profile_diff(mudp_path1, mudp_path2, mudp_config_path1, mudp_config_path2):
    mudp_config1 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"
    mudp_config2 = r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json"

    p1 = r"C:\Users\mj2snl\Documents\logs\logs\clust\rRf360t4090309v205p50_original\2p13pE_27c2132_20191127_131407_030_rRf360t4090309v205p50.mudp"
    p2 = r"C:\Users\mj2snl\Documents\logs\logs\clust\rRf360t4090309v205p50\2p13pE_27c2132_20191127_131407_030_rRf360t4090309v205p50.mudp"

    result = profile_diff(p1, p2, mudp_config1, mudp_config2)

    clus_prof_diff = result.clustering

    c_min, c_max, c_mean = min(clus_prof_diff), max(clus_prof_diff), clus_prof_diff.mean()
