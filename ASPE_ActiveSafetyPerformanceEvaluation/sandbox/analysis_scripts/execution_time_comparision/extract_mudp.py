import os
import pickle

from AptivDataExtractors.F360.F360MUDPExtractor import F360Extractor
from AptivDataExtractors.utilities.SupportingFunctions import save_todict_pkl
from AptivDataParser import MudpParser
from tqdm import tqdm


def extract_mudp_from_mudp_file(mudp_path, mudp_config_path, f_save_parsed_data=False, f_save_extr_data=False):
    #   Parsing
    parser = MudpParser.MudpHandler(mudp_path, mudp_config_path)
    parsed_data = parser.decode()

    if f_save_parsed_data:
        parser.save_to_pickle()

    #   Extracting
    f360_extractor_object = F360Extractor(parsed_data)
    f360_extracted_data = f360_extractor_object.extract_data()

    if f_save_extr_data:
        save_path = mudp_path.replace('.mudp', '_MUDP_Extracted.pickle')
        save_todict_pkl(f360_extracted_data, save_path)

    return f360_extracted_data


def local_mudp_load(file_path, config_path=None):
    _, mudp_ext = os.path.splitext(file_path)
    if mudp_ext == '.pickle':
        file = open(file_path, 'rb')
        data = pickle.load(file)
        file.close()
    else:
        data = extract_mudp_from_mudp_file(file_path, config_path, f_save_extr_data=True)
    return data


def save_to_pickle(path, data):
    file = open(path, 'wb')
    pickle.dump(data, file)
    file.close()


def walk_and_find_files_with_ext(path, ext):
    array = []
    for root, dirs, files in os.walk(path):
        for name in files:
            _, file_ext = os.path.splitext(name)
            if file_ext == ext:
                array.append(os.path.join(root, name))
    return array


def extract_mudp_from_folder(folder_path, mudp_config_path, f_save_parsed_data=False, f_save_extr_data=True):
    mudp_files = []
    for root, dirs, files in os.walk(folder_path):
        for name in files:
            if name[-5::] == '.mudp':
                mudp_files.append(os.path.join(root, name))

    with open(os.path.join(folder_path, 'files_that_didnt_pass.txt'), 'w') as opened_file:
        for file in tqdm(mudp_files):
            try:
                extract_mudp_from_mudp_file(file,
                                            mudp_config_path,
                                            f_save_parsed_data=f_save_parsed_data,
                                            f_save_extr_data=f_save_extr_data)
            except:
                opened_file.writelines(file + '\n')


if __name__ == '__main__':
    extract_mudp_from_folder(r'C:\Users\mj2snl\Documents\logs\logs\clust_log_set',
                             r"C:\Users\mj2snl\Desktop\aspe_local\cea-258\parsers_config\mudp_data_parser_config.json")

    file = local_mudp_load(
        r"C:\Users\mj2snl\Documents\logs\logs\clust_log_set\rRf360t4090309v205p50\2p13pE_27c2132_20191127_110457_002_rRf360t4090309v205p50_MUDP_Extracted.pickle",
        None)
