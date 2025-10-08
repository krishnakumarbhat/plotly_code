import pickle


def load_pickle_file(pickle_path):
    with open(pickle_path, 'rb') as file:
        agg_data = pickle.load(file)
    return agg_data


def save_pickle_file(data, path):
    with open(path, 'wb') as file:
        pickle.dump(data, file)
