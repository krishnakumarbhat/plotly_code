import csv
import struct

import numpy as np
import pandas as pd

from aspe.parsers.aptiv_data_parser import DataParser


class BinParser(DataParser):
    """
    BinReader is master class for bin decoding. Reads binary file  and converts it to
    human readable format. Output data is stored as 'nested_data' which is nested dict of numpy arrays.
    """
    def __init__(self, *args, **kwargs):
        """

        :param args:
        :param kwargs:
        """
        super().__init__(*args, **kwargs)
        self.decoded = {}

    def parse(self, file_path: str = None):
        """
        Main method to decode binary file and create output data structure
        :param file_path: absolute path to log file
        :type file_path: object
        :return: dictionary
        """
        binfile = BinReaderDeclaration(file_path)
        self.decoded = binfile.decode_all_dict()
        output = {
            'parsed_data': self.decoded.copy(),
            'path': file_path,
        }
        return output


class BinReaderDeclaration:
    """
    BinReaderDeclaration is class that handles reading, formatting Bin file to different python
    data_types structures.
    """
    def __init__(self, path):
        """

        :param path: path to a bin file
        :type path: str.
        """

        self.path = path
        self.title_names = []
        self.title_names_temp = None
        self.title_tab = []
        self.decoded_bin_file = []
        self.signals_dict = dict()
        self.file = open(self.path, 'rb')
        self.data_size = 8
        self.title_size = 256
        self.data_format = '=d'
        self.title_format = '=256s'

    def arrange_title_names(self):
        """
        Unfolds title names form bin file in preparation of data list creation.
        Signals that are matrices have names created for each column.

        :return:
        """
        # arrange row of title names
        # num of columns

        raw_data = self.file.read(self.data_size)
        col_len, = struct.unpack(self.data_format, raw_data)
        col_len = int(col_len)

        for i in range(col_len):
            raw_data_title = self.file.read(self.title_size)
            name, = struct.unpack(self.title_format, raw_data_title)
            name = name.rstrip(b'\x00')
            name = name.split(b'\x00')
            name = name[0].decode("utf-8")
            self.title_names.append(name)

        # copy title names for use of decode_all_dict
        self.title_names_temp = self.title_names.copy()

        # arrange all possible columns of all objects

        while len(self.title_names_temp) != 0:
            if 'REPEAT' in self.title_names_temp[0]:
                temp = []
                num_repeats = self.title_names_temp[0].strip('REPEAT')
                self.title_names_temp.pop(0)
                while 'END' not in self.title_names_temp[0]:
                    pop_name = self.title_names_temp.pop(0)
                    temp.append(pop_name)
                for idx in range(int(num_repeats)):
                    for obj in temp:
                        name_with_index = obj + '_' + str(idx)
                        self.title_tab.append(name_with_index)  # add a separate column name for columns in a matrix
                self.title_names_temp.pop(0)
            else:
                self.title_tab.append(self.title_names_temp[0])
                self.title_names_temp.pop(0)

    def decode_all_list(self):
        """
        Decodes bin file to list type

        :return lists
        """
        self.arrange_title_names()
        self.decoded_bin_file.append(self.title_tab)  # appending titles

        pos = self.file.tell()  # checking current pointer position
        eof = self.file.seek(-1, 2)  # changing pointer position to the end of file
        self.file.seek(pos, 0)  # changing pointer position to the previous position

        # reading all lines

        row_construction = []
        while eof >= self.file.tell():
            for i in range(len(self.title_tab)):
                raw_data = self.file.read(self.data_size)
                if len(raw_data) < self.data_size:
                    break
                if raw_data != b'':
                    record, = struct.unpack(self.data_format, raw_data)
                    row_construction.append(record)
                    if len(row_construction) == len(self.title_tab):
                        self.decoded_bin_file.append(row_construction)
                        row_construction = []
        self.file.close()
        return self.decoded_bin_file

    def decode_all_dict(self):
        """
        Transform list to dictionary of numpy arrays

        :return dictionary
        """
        self.decode_all_list()  # decode bin file to a list of lists
        data = np.array(self.decoded_bin_file[1:], dtype=np.float64)
        data_column = 0
        while len(self.title_names) != 0:
            if 'REPEAT' in self.title_names[0]:
                num_repeats = self.title_names[0].strip('REPEAT')
                self.title_names.pop(0)
                num_r_signals = 0  # number of repeated signals

                # get a list of signals with arrays
                repeat_list = []
                while 'END' not in self.title_names[0]:
                    self.signals_dict[self.title_names[0]] = np.zeros((len(data[:, 0]), int(num_repeats)))
                    repeat_list.append(self.title_names[0])
                    num_r_signals += 1
                    self.title_names.pop(0)
                self.title_names.pop(0)

                # fill the arrays
                for idx_repeat in range(int(num_repeats)):
                    for idx_r_signal in range(num_r_signals):
                        self.signals_dict[repeat_list[idx_r_signal]][:, idx_repeat] = data[:, data_column]
                        data_column += 1
            else:
                self.signals_dict[self.title_names[0]] = data[:, data_column]
                data_column += 1
                self.title_names.pop(0)
        return self.signals_dict

    def decode_all_pandas(self):
        """
        List containing decoded data is transformed into pandas

        :return: pandas
        """

        list_array = self.decode_all_list()
        pan = pd.DataFrame(list_array[1::], columns=list_array[0])
        pan = pan.set_index(['scan_index'])
        return pan

    def save_decoded_file(self, pandas_data, path):
        """
        Save pandas to pickle file
        Save list to csv file

        :param pandas_data: decoded data in pandas
        :param path: path to output folder
        :return:
        """

        if isinstance(pandas_data, pd.DataFrame):
            pandas_data.to_pickle(path)
        if isinstance(pandas_data, list):
            with open(path, 'w', newline='') as myfile:
                writer = csv.writer(myfile)
                for row in pandas_data:
                    writer.writerow(row)

    @staticmethod
    def load_decoded_file(path, var_type):
        """
        Load pandas from pickle
        Load list from csv

        :return pandas DataFrame
        """
        if var_type == 'pkl':
            df = pd.read_pickle(path)
        if var_type == 'csv':
            with open(path) as csvfile:
                r = csv.reader(csvfile)
                df = []
                for row in r:
                    df.append(row)
        return df
