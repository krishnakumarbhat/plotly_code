import pickle
from sandbox.refactoring_prototype.prototype import ExtractedData
from sandbox.refactoring_prototype.prototype import DataContainer


dict_path = r'C:\New folder\data_as_dict.pickle'
obj_path = r'C:\New folder\data_as_obj.pickle'

with open(dict_path, 'rb') as handle:
    dict_data = pickle.load(handle)
    print()

with open(obj_path, 'rb') as handle:
    obj_data = pickle.load(handle)
    print()
