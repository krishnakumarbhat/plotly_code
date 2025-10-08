from LogSets.utils.resim.f360_resim_booter import F360ResimBooter
from LogSets.utils.file_handling import file_walker, directory_walker, save, load
from LogSets.utils.data_provider import DataProvider
from LogSets.events.events_calculator import EventsCalculator
from LogSets.features.features_calculator import FeaturesCalculator
from LogSets.logsets_creators.full_set_creator import FullSetCreator
from LogSets.logsets_creators.subsets_creator import SubSetsCreator
import os
import random
from tqdm import tqdm
import pandas as pd


class LogSets:
    @staticmethod
    def find_logs_paths_and_their_respective_ini_files(root=r'\\10.224.186.68\AD-Shared\F360\Logs', save_as_files=True,
                           save_folder=r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\resim\logs_for_resim_demo',
                                                       saturate_n_logs_per_ini_file=False, max_n_logs=50):
        """
        Find log's paths that can be re simulated (have .ini files), starting from root directory downwards
        :param root:
        :param save_folder:
        :param save_as_files:
        :param saturate_n_logs_per_ini_file:
        :param max_n_logs:
        :return:
        """
        paths = file_walker(root, extension='.ini')

        dirs = dict.fromkeys(paths)
        for path in tqdm(paths):
            dirs[path] = path.replace('\\' + path.split('\\')[-1], '')

        logs_paths_and_ini_files = []
        for k, v in tqdm(dirs.items(), desc='Finding ini files'):
            files = set(file_walker(v, extension='.dvl'))
            if len(files) > 0:
                if len(files) > max_n_logs and saturate_n_logs_per_ini_file:
                    logs_paths_and_ini_files.append({'dvl_paths': random.sample(files, max_n_logs), 'ini': k})
                else:
                    logs_paths_and_ini_files.append({'dvl_paths': files, 'ini': k})

        if save_as_files:
            os.makedirs(save_folder, exist_ok=True)
            for c, log in enumerate(logs_paths_and_ini_files):
                with open(os.path.join(save_folder, f'ini{c}.txt'), 'w') as file:
                    file.write(log['ini'])

                with open(os.path.join(save_folder, f'logs{c}.txt'), 'w') as file:
                    for path in log['dvl_paths']:
                        file.writelines(path + '\n')

        return logs_paths_and_ini_files

    @staticmethod
    def re_simulate_logs_using_already_created_resim_exe(logs_paths_and_their_respective_ini_files, resim_exe_path):
        n_logs_resimulated = sum([len(paths['dvl_paths']) for paths in logs_paths_and_their_respective_ini_files])
        resim = F360ResimBooter(resim_exe_path)
        pbar = tqdm(total=n_logs_resimulated, desc='Re-simulating...')
        for ini_file_and_logs_paths in logs_paths_and_their_respective_ini_files:
            resim.resim_log_list(ini_file_and_logs_paths['dvl_paths'], ini_file_and_logs_paths['ini'], pbar)
        pbar.close()

    @staticmethod
    def find_re_simulated_logs_by_resim_version(root_directory=r'\\10.224.186.68\AD-Shared\F360\Logs',
                                                resim_ver='rRf360t4000304v202r1', extension='.mudp'):
        """
        Finding resimulated log's paths from root_directory downwards, resimulated with resim_ver
        :param root_directory:
        :param resim_ver:
        :param extension:
        :return:
        """
        re_simulated_logs_paths = []
        dirs = directory_walker(root_directory, directory=resim_ver)
        for dir_ in tqdm(dirs, desc='Finding re-simulated logs'):
            for file in os.listdir(dir_):
                if extension in file:
                    re_simulated_logs_paths.append(os.path.join(dir_, file))
        return re_simulated_logs_paths

    @staticmethod
    def get_extracted_logs_generator(re_simulated_logs_paths):
        mudp_parser_config = r"\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS" \
                             r"\Extractors_refactoring\parsers_config\mudp_data_parser_config.json"
        logged_info_path = r"\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\LogSet_DEMO_logged_info.txt"
        return DataProvider().generate_extracted_logs(re_simulated_logs_paths, mudp_parser_config,
                                                      log_info_path=logged_info_path)

    @staticmethod
    def calculate_all_components(re_simulated_logs_paths):
        extracted_logs_generator = LogSets.get_extracted_logs_generator(re_simulated_logs_paths)
        components = pd.DataFrame()
        pbar = tqdm(total=len(re_simulated_logs_paths), desc='Parsing, extracting and calculating features...')

        for extracted_log in extracted_logs_generator:
            extracted_log_data = extracted_log
            components_per_log = pd.DataFrame()
            components_per_log['path'] = pd.Series(extracted_log_data['metadata']['data_path'])

            extracted_objects_signals = extracted_log_data['data']['objects']['signals']

            features = FeaturesCalculator(extracted_objects_signals, components)
            components = features.calculate_all_available_features()

            events = EventsCalculator(extracted_objects_signals, components)
            components = events.calculate_all_available_events()

            components = components.append(components_per_log, ignore_index=True, sort=False)
            pbar.update(1)

        pbar.close()
        return components


def main():
    #   STEP 1: Find logs paths and their respective ini files
    logs_paths_and_their_respective_ini_files = LogSets.find_logs_paths_and_their_respective_ini_files(
        saturate_n_logs_per_ini_file=True, max_n_logs=200, root=r'\\10.224.186.68\AD-Shared\F360\Logs')

    #   STEP 2: Re-simulate logs using already created resim exe file
    resim_exe_path = r"\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\resim\f360_resim\Debug_Win32\resim_f360.exe"
    LogSets.re_simulate_logs_using_already_created_resim_exe(logs_paths_and_their_respective_ini_files,
    resim_exe_path)

    #   STEP 3: Find logs paths that were re-simulated with provided resim version
    re_simulated_logs_paths = LogSets.find_re_simulated_logs_by_resim_version(resim_ver='rRf360t4010304v202r1')

    #   STEP 4: Parse, extract and then calculate features and events for re-simulated logs in loop
    components = LogSets.calculate_all_components(re_simulated_logs_paths)
    save(components, r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\raw_components\components.pickle')

    #   STEP 5: Create FUll Set
    creator = FullSetCreator(path=r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_test_2.pickle')
    full_set = creator.create(components, plot=False, fit=True, drop_features_with_nan_values=True,
                              drop_features_full_of_zeros=True)
    creator.save()

    #   STEP 6: Create subsets from full set
    alpha_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\alpha_set.pickle'
    golden_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\GoldenSets\golden_set.pickle'
    outlier_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\OutlierSets\outlier_set.pickle'
    sub_sets_creator = SubSetsCreator(full_set, alpha_set_path, golden_set_path, outlier_set_path)
    sub_sets_creator.create_all(plot=False, plot_clusters=False)
    sub_sets_creator.save_all()
    print()


def demo():
    #   No resimulation, extraction, parsing and calculation of features and events
    #   Load pre-calculated components
    components = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\raw_components\components.pickle')

    #   STEP 6: Create FUll Set and subsets
    full_set_creator = FullSetCreator(path=r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_demo.pickle')
    full_set = full_set_creator.create(components, plot=True, fit=True, drop_features_with_nan_values=True,
                                       drop_features_full_of_zeros=True)
    full_set_creator.save()

    alpha_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\alpha_set.pickle'
    golden_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\GoldenSets\golden_set.pickle'
    outlier_set_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\OutlierSets\outlier_set.pickle'

    sub_sets_creator = SubSetsCreator(full_set, alpha_set_path, golden_set_path, outlier_set_path)
    sub_sets_creator.create_all(plot=True, plot_clusters=True)
    sub_sets_creator.save_all()


if __name__ == '__main__':
    demo()
    # main()
