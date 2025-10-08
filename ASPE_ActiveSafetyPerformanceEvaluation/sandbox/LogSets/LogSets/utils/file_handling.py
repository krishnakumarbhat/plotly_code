import pickle
import os
import random
from tqdm import tqdm


def load(path):
    with open(path, 'rb') as handle:
        data = pickle.load(handle)
    return data


def save(data, output_filename):
    with open(output_filename, 'wb') as handle:
        pickle.dump(data, handle, protocol=pickle.HIGHEST_PROTOCOL)


def file_walker(path, extension=None, forbidden='rRf360t'):
    paths = []
    for root, dirs, files in tqdm(os.walk(path)):
        for file in files:
            if extension is None:
                paths.append(os.path.join(root, file))
            else:
                if extension in file and forbidden not in file:
                    paths.append(os.path.join(root, file))
    return paths


def directory_walker(path, directory=None):
    paths = []
    for root, dirs, files in os.walk(path):
        for dir_ in dirs:
            if directory is None:
                paths.append(os.path.join(root, dir_))
            else:
                if directory == dir_:
                    paths.append(os.path.join(root, dir_))
    return paths


def get_logs_paths_list(log_paths=None, folder_paths=None, extension='MUDP_Extracted.pickle'):
    if log_paths is None:
        log_paths = []
    if folder_paths is None:
        folder_paths = []
    all_paths = log_paths
    for paths in folder_paths:
        for path in os.listdir(paths):
            if extension in path:
                all_paths.append(os.path.join(paths, path))
    return all_paths
