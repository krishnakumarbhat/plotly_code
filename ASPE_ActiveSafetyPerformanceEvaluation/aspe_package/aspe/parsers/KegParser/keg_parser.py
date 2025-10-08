from warnings import warn

import h5py

from aspe.parsers.aptiv_data_parser import DataParser

supported_matlab_classes = {'char':         "b'char'",
                            'double':       "b'double'",
                            'dataset':      "b'cell'",
                            }


class KegParser(DataParser):
    """
    KegParser is master class for keg decoding. Reads keg file and converts it to
    human readable format.
    """
    def __init__(self, *args, **kwargs):
        """
        :param args:
        :param kwargs:
        """
        super().__init__(*args, **kwargs)
        self.decoded = {}
        self.metadata = {}

    def parse(self, file_path: str):
        """
        Main method to decode keg file and create output data structure
        :param file_path: absolute path to log file
        :type file_path: object
        :return: dictionary
        """
        kegfile = KegReaderDeclaration(file_path)
        self.decoded, self.metadata = kegfile.decode_all()
        output = {
            'parsed_data': self.decoded.copy(),
            'metadata': self.metadata.copy(),
            'path': file_path,
        }
        return output


class KegReaderDeclaration:
    """
    KegReaderDeclaration is class that handles reading, formatting Keg file to different python
    data_types structures. This class uses h5py module for parsing keg file data.
    """
    def __init__(self, path):
        """
        :param path: path to a keg file
        :type path: str.
        """
        self.path = path
        self.file = h5py.File(self.path, 'r')
        self.keg_data = None
        self.keg_metadata = {}

    def decode_all(self):
        """
        Main .keg reading function.
        """
        self.keg_metadata = self.get_metadata(self.file)
        self.keg_data = self.unpack_keg(self.file)
        self.close_file()
        return self.keg_data, self.keg_metadata

    def get_metadata(self, file):
        if file.attrs is not None:
            for key, value in file.attrs.items():
                self.keg_metadata[key] = value
        return self.keg_metadata

    def unpack_keg(self, data):
        """
        Function to choose the right way to unpack data.
        """
        data_type = type(data)
        if data_type is h5py.Dataset:
            return self.unpack_dataset(data)
        if data_type in [h5py.Group, h5py.File]:
            return self.unpack_group(data)

    def unpack_dataset(self, dataset):
        """
        Function extract dataset, checks if dataset contains subdatasets, references.
        """
        dtype = self.matlab_class_type(dataset)
        if dtype == supported_matlab_classes['dataset']:
            refs = dataset[:]
            # Change data shape to (1,n) to handle all kind of dataset shapes
            shape = dataset.shape
            if len(shape) == 1:
                shape = shape + (1,)
            if shape[1] == 1:
                refs = refs.reshape([1, shape[0]])
            refs = refs[0]

            dataset = list()
            for ref in refs:
                if type(ref) is h5py.Reference:
                    decoded_ref_data = self.decode_reference(ref)
                else:
                    decoded_ref_data = ref
                dataset.append(decoded_ref_data)

        elif dtype in supported_matlab_classes.values():
            return self.decode_data(dataset, dtype)

        is_dataset_accessible = len(dataset.shape) > 0
        if is_dataset_accessible:
            return dataset[:]
        else:
            warn(f"Empty dataset: {dataset.name.split('/')[-1]}")
            return dataset

    def unpack_group(self, group):
        """
        Function extract data from the group based on group's keys. Can also extract 'h5py.File' structure.
        """
        group_dict = {}
        keys = []
        for key in group.keys():
            data = group[key]
            keys.append(key)
            group_dict[key] = self.unpack_keg(data)
        return group_dict

    def decode_data(self, data, dtype):
        """
        Function decodes the data based on its matlab class type. Function uses supported_matlab_classes dictionary.
        """
        if dtype == supported_matlab_classes['char']:
            decoded_data = "".join([chr(i) for i in data[:]])
        elif dtype == supported_matlab_classes['double']:
            decoded_data = data[:]
        elif dtype == supported_matlab_classes['dataset']:
            decoded_data = self.unpack_keg(data)
        else:
            decoded_data = data
        return decoded_data

    def decode_reference(self, reference):
        """
        Function loads the data via h5py reference, check matlab class type and decodes the data based od this.
        """
        dataset = self.file[reference]
        data_type = self.matlab_class_type(dataset)
        decoded_ref = self.decode_data(dataset, data_type)
        return decoded_ref

    def close_file(self):
        """
        Function closes previously opened.keg file
        """
        self.file.close()

    def matlab_class_type(self, data):
        """
        Function returns matlab class as string
        """
        attributes = data.attrs
        matlab_class_type = attributes.get('MATLAB_class')
        return str(matlab_class_type)


if __name__ == '__main__':

    keg_path = r"path_to_file.keg"
    parser = KegParser()
    parsed = parser.parse(keg_path)
    print('Parsing test done.')
