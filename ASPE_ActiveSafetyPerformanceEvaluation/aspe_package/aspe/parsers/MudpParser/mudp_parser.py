"""
mudp_parser.py
**************
Parser for mudp binary files.
"""


import os
import re
import uuid
import warnings
from struct import unpack
from typing import Optional

import numpy as np

from aspe.parsers.aptiv_data_parser import DataParser


class MudpParser(DataParser):
    def __init__(self, streams_to_read: tuple,
                 unknown_size_per_stream: dict,
                 mudp_stream_def_path: Optional[str] = None,
                 source_to_parse: Optional[int] = None,
                 unknown_type_handling: bool = True,
                 unknown_size_warnings: bool = False):
        """
        MudpParser is master class for MUDP decoding. Reads binary file using stream definition files and converts it to
        human readable format. Output data is stored as 'nested_data' which is nested dict of numpy arrays.

        :param streams_to_read: tuple of streams which should be parsed
        :type streams_to_read: tuple
        :param unknown_size_per_stream: when inside stream definitions there is variable type which is unknown
            (enums for example) parser use this dict to check how to read and store variable, this dictionary should be
            in form {stream_number: number_of_bytes} - it assumes different unknown sizes for each channel,
            some default configuration is saved inside parser file, passed dict overwrites only these keys which are
            different than default
        :type: dict
        :param mudp_stream_def_path: path to folder with streams definitions
            (it has priority over MUDP_STREAM_DEFINITIONS_PATH environment variable)
        :type mudp_stream_def_path: str.
        :param source_to_parse: source number to parse,
            default set to None - parser does not check streams source number,
        if set to some value - only streams which source number is equal to source_to_parse will be processed
        :type source_to_parse: int
        :param unknown_type_handling: if True unknown variables types which are stored in stream definitions (enums for
        example) are read using unknown_size_per_stream dict argument, if False - these variables will be not saved to
        output
        :type: bool
        :param unknown_size_warnings: if True when parser meet unknown variable type warning and appropriate
            info will be displayed
        :param: bool
        """
        super().__init__()

        self._streams_to_read = streams_to_read
        self.unknown_size_per_stream = unknown_size_per_stream
        self.unknown_type_handling = unknown_type_handling
        self.unknown_size_warnings = unknown_size_warnings
        self.source_to_parse = source_to_parse

        self.variable_size_streams = {35: {70, 71, 72}}

        self.streams_definition_path = self._check_stream_definition(mudp_stream_def_path)
        self._header_dict = {}
        self._payload_dict = {}

        self._drops_dict = {}
        self._available_streams = []

    def parse(self, mudp_file_path: str):
        """
        Main method to decode binary file and create output data structure:
        Reads data streams, down-selects buffer of interest and parses according to configuration file.
        :returns: dict(dict(...(np.ndarray)))
        """
        _, ext = os.path.splitext(mudp_file_path)
        if not ext == '.mudp':
            raise ValueError('Please provide .mudp file type')

        # clean up structures
        self._header_dict = {}
        self._payload_dict = {}
        self._drops_dict = {}
        self._available_streams = []

        with open(mudp_file_path, 'rb') as mudp_stream:
            self._read_headers(mudp_stream)
            self._read_streams_data(mudp_stream)
            parsed = self._decode_payload()

            if 70 in parsed:
                self._inflate_stream70(parsed)
            if 71 in parsed and 72 in parsed:
                self._extrapolate_stream72(parsed)

        output = {
            'parsed_data': parsed,
            'path': mudp_file_path,
            'header_data': self._header_dict,
        }
        return output

    def _inflate_stream70(self, parsed_data):
        """
        F360 Stream 70 (Objects) data is packed when logged, function inflates size of
        logged data such that each tracked object data is placed in the index corresponding to
        the track ID of the object.
        """
        stream_id = 70
        stream = parsed_data[stream_id].copy()
        key_field = stream["trkID"]
        n_records = key_field.shape[0]
        max_trk_id = key_field.max()

        for signal in stream:
            if stream[signal].ndim < 2:
                continue
            elif stream[signal].ndim == 2:
                field_shape = (n_records, max_trk_id)
            else:
                field_shape = (n_records, max_trk_id, stream[signal].shape[2])

            temp_signal = np.zeros(field_shape, dtype=stream[signal].dtype)
            for i in range(n_records):
                trk_ids = key_field[i]
                for j, ID in enumerate(trk_ids):
                    if ID > 0:
                        temp_signal[i][ID - 1] = stream[signal][i][j]

            parsed_data[stream_id][signal] = temp_signal

    def _extrapolate_stream72(self, parsed_data):
        """
        Stream 72 (Sensors) only logs data every 100th index. The data is assumed static and this function
        extrapolates data to fill the missing slots. Using stream 71 (Detections) as a reference stream
        of available scan indexes. The first and last repeat are special cases as they are recorded before and after
        logged sensor data points, in which closest logged data point is used for extrapolation.
        """
        stream_id = 72
        reference_stream_id = 71
        extrapolating_factor = 100
        stream = parsed_data[stream_id]

        reference_stream = parsed_data[reference_stream_id]
        stream_ref_index = stream['stream_ref_index']
        reference_stream_ref_index = reference_stream['stream_ref_index']

        stream_ref_index_n = len(stream_ref_index)
        reference_stream_ref_n = len(reference_stream_ref_index)
        start_index_diff = min(stream_ref_index) - min(reference_stream_ref_index)

        # No sensor data present or sensor data already extrapolated
        if stream_ref_index_n in (0, reference_stream_ref_n):
            return
        elif stream_ref_index_n == 1:
            repeats = [reference_stream_ref_n]
        else:
            repeats = [extrapolating_factor] * stream_ref_index_n
            repeats[0] += start_index_diff
            repeats = self._find_and_remove_missing_indexes(repeats, extrapolating_factor, reference_stream_ref_index)
            if sum(repeats) >= reference_stream_ref_n:
                repeats[-1] -= sum(repeats) - reference_stream_ref_n
            else:
                repeats[-1] += reference_stream_ref_n - sum(repeats)

        temp_stream = dict.fromkeys(stream.keys())
        for signal in stream:
            temp_stream[signal] = np.empty(shape=stream[signal].shape, dtype=stream[signal].dtype)
            if signal in {'stream_ref_index', 'data_timestamp_us'}:
                temp_stream[signal] = reference_stream[signal]
                continue
            temp_stream[signal] = np.repeat(stream[signal], repeats, axis=0)

        parsed_data[stream_id] = temp_stream

    def _find_and_remove_missing_indexes(self, bins, bin_scale, indexes):
        for i in range(len(indexes) - 1):
            bin_idx = i // bin_scale
            bin_idx = 1 if bin_idx == 0 else bin_idx
            if indexes[i] + 1 != indexes[i + 1]:
                bins[bin_idx - 1] -= 1
        return bins

    def _read_headers(self, mudp_stream):
        """
        Iterate over headers in the mudp data streams; down-select headers of interest, interpret and save them.

        :param mudp_stream: .mudp files data
        :type mudp_stream: binary str
        """
        while True:
            header_buffer = mudp_stream.read(Header.HEADER_LENGTH)
            if not len(header_buffer) == Header.HEADER_LENGTH:
                break

            current_stream = header_buffer[Header.STREAM_NUMBER_POSITION]
            current_source = header_buffer[Header.STREAM_SOURCE_INFO_POSITION]

            if current_stream not in self._available_streams:
                self._available_streams.append(current_stream)

            should_be_parsed = False
            if current_stream in self._streams_to_read:
                should_be_parsed = True
            if self.source_to_parse is not None:
                correct_source = current_source == self.source_to_parse
                should_be_parsed = should_be_parsed and correct_source

            if should_be_parsed:
                if not (current_stream in self._header_dict):
                    self._header_dict[current_stream] = Header(header_buffer)
                self._header_dict[current_stream].get_extended_data(header_buffer, mudp_stream.tell())

            stream_len = Header.get_stream_length(header_buffer)
            mudp_stream.seek(stream_len, 1)
        self._available_streams.sort()

    def _read_streams_data(self, mudp_stream):
        """
        Based on headers save payload of interesting streams.

        :param mudp_stream: .mudp file data
        :type mudp_stream: binary str
        """
        for stream_key, header in self._header_dict.items():
            self._get_mudp_definition_files(header)
            if not header.f_stream_chunks_is_0:
                header.header_chunks_sanity_check(self._drops_dict)
            else:
                header.stream_ref_idx_unique = header.stream_ref_index
            self._read_payload_data(mudp_stream, header)

    def _read_payload_data(self, mudp_stream, header):
        """
        Read payload for single stream and save it to self.payload data

        :param mudp_stream: .mudp files data
        :type mudp_stream: binary str
        :param header: structure containing information about stream structure
        :type header: Header
        """
        payload_data = None
        saved_payload_idx = 0

        if header.stream_number not in self._payload_dict:
            header.log_length = int(len(np.unique(header.stream_ref_index)))

        self._payload_dict[header.stream_number] = [None] * header.log_length

        for payload_index, payload_start_pos in enumerate(header.payload_start_file_position):
            mudp_stream.seek(payload_start_pos, 0)

            if header.stream_chunk_idx[payload_index] == 0:
                append_payload = False
            else:
                append_payload = True

            if header.stream_chunk_idx[payload_index] == header.stream_chunks_len[payload_index] - 1 or \
                    header.stream_chunks_len[payload_index] == 0:
                save_payload = True
            else:
                save_payload = False

            if append_payload:
                payload_data += mudp_stream.read(header.stream_data_len_all[payload_index])
            else:
                payload_data = mudp_stream.read(header.stream_data_len_all[payload_index])
            if save_payload:
                if not header.flag_same_stream_size_with_declaration:
                    payload_data = payload_data[:header.stream_declaration_len]
                self._payload_dict[header.stream_number][saved_payload_idx] = payload_data
                saved_payload_idx += 1

    def _decode_payload(self):
        """
        Converts binary buffer into proper variables based on mudp declaration and creates numpy array for every unique
        mudp parameter.
        Created arrays have shape: (log_length, number_of_occurences_of_parameter_in_mudp.names)

        eg.
                   |                     |                     |
                   | param_occureence: 1 | param_occureence: 2 |
        ___________|_____________________|_____________________|
                   |                     |                     |
        scan_idx: 1|         value       |         value       |
        ___________|_____________________|_____________________|
                   |                     |                     |
        scan_idx: 2|         value       |         value       |
        ___________|_____________________|_____________________|

        """
        output = {}
        for stream_key, header in self._header_dict.items():
            if header.source_info in self.variable_size_streams and header.stream_number in \
                    self.variable_size_streams[header.source_info]:
                variable_size_stream = True
            else:
                variable_size_stream = False

            mudp = MUDPDeclaration(header.configuration_file_path,
                                   unknown_type_handling=self.unknown_type_handling,
                                   f_unknown_size_warnings=self.unknown_size_warnings,
                                   unknown_size=self.unknown_size_per_stream[stream_key])

            if variable_size_stream:
                temp_id = uuid.uuid4()
                temp_streamdef_name = "temp_streamdef_{}.txt".format(temp_id)
                n_elements = self._find_num_elements(stream_key)
                logstruct_size = mudp.generate_streamdef(n_elements, temp_streamdef_name)
                mudp.file_path = temp_streamdef_name

                for i, rec in enumerate(self._payload_dict[stream_key]):
                    if len(rec) < logstruct_size:
                        fill = bytearray([0] * (logstruct_size - len(rec)))
                        rec += fill
                        self._payload_dict[stream_key][i] = rec

            mudp.decode_declaration_file()
            numpy_type = mudp.get_numpy_data_format()
            output[stream_key] = self._create_nested_structure(mudp)
            output[stream_key]['stream_ref_index'] = np.array(header.stream_ref_idx_unique)
            parameters = self._create_index_list(mudp)
            np_payload_dict = np.array(self._payload_dict[stream_key])
            if numpy_type.itemsize == header.stream_declaration_len or variable_size_stream:
                arr = np.frombuffer(np_payload_dict, dtype=numpy_type)
            else:
                warnings.warn(f'Calculated record size {numpy_type.itemsize} does not match expected record size '
                              f'{header.stream_declaration_len} in stream {stream_key}.')
                continue

            if variable_size_stream:
                try:
                    os.remove(mudp.file_path)
                except OSError as e:
                    print(e)

            for parameter, idx_list in parameters.items():
                nested_parameters_list = parameter.split('.')
                nested_value = output[stream_key]
                try:
                    for key in nested_parameters_list[:-1]:
                        nested_value = nested_value[key]
                except AttributeError:
                    print('Invalid or missing key ' + key)
                    break
                columns = [arr[i] for i in idx_list]
                if parameter in mudp._n_dimensional_structures.keys():
                    temp = np.vstack(columns).T  # get 2d numpy array
                    parameter_shape = mudp._n_dimensional_structures[parameter]
                    first_dim = len(columns[0])     # length of the log
                    assert len(columns) == np.product(parameter_shape) # check read data corespond to shape size
                    signal_array = temp.reshape((first_dim, *parameter_shape))      # reshape
                else:
                    signal_array = np.vstack(columns).T
                    if len(signal_array.shape) == 2:
                        if signal_array.shape[1] == 1:
                            signal_array = signal_array.reshape(-1)
                nested_value[nested_parameters_list[-1]] = signal_array
        return output

    def _create_nested_structure(self, mudp):
        """
        Creating empty nested structure to be filled in _parse_data()

        STRUCTURE:
        {stream_number_1: { nesting_1: { nesting_2 : .... None}}
        stream_number_2: { nesting_1: { nesting_2 : .... None}}
        .
        .
        .
        stream_number_n: { nesting_1: { nesting_2 : .... None}}}

        :param mudp: .mudp files data
        :type mudp: binary str
        :return: dict - empty nested data structure
        """
        empty_nested_structure = {}
        for keys in list(set(mudp.names)):
            if keys != 'UNKNOWN' and keys != 'PADDING':
                nested_data_for_key_copy = empty_nested_structure
                keys = keys.split('.')
                for key in keys[:-1]:
                    nested_data_for_key_copy = nested_data_for_key_copy.setdefault(key, {})
                nested_data_for_key_copy[keys[-1]] = None
        return empty_nested_structure

    def _create_index_list(self, mudp):
        """
        Method creates list of indexes for every occurrence of parameter in mudp.names
        per unique parameter in mudp.names

        :param mudp: .mudp files data
        :type mudp: binary str
        :return: dict - parameter: idx_list
        """
        mudp_names = mudp.names
        unique_mudp_stream_fields = list(set(mudp_names))
        parameters = dict.fromkeys(unique_mudp_stream_fields)
        if 'PADDING' in parameters:
            parameters.pop('PADDING', None)
        if 'UNKNOWN' in parameters:
            parameters.pop('UNKNOWN', None)
        mudp_names_ndarray = np.array(mudp_names)
        for parameter in parameters.keys():
            idx_list = np.where(mudp_names_ndarray == parameter)[0]
            parameters[parameter] = ['f' + str(idx) for idx in idx_list]
        return parameters

    def _get_mudp_definition_files(self, header):
        """
        Find declaration file for given stream: with respect to its version

        :param header: structure containing information about stream structure
        :type header: Header
        :return: list - list of streams data length
        """

        definitions_path = self.streams_definition_path

        str_def_src = 'src{:03d}'.format(header.source_info)
        str_def_str = 'str{:03d}'.format(header.stream_number)
        str_def_ver = 'ver{:03d}'.format(header.stream_version)
        file_name = 'strdef_{}_{}_{}.txt'.format(str_def_src, str_def_str, str_def_ver)
        full_path = os.path.join(definitions_path, file_name)
        if not os.path.exists(full_path):
            raise FileNotFoundError('Configuration file for this stream not found: expected file: {}'.format(file_name))
        header.configuration_file_path = full_path

        stream_data_length = []
        with open(full_path, 'r') as conf:
            for i in range(4):
                line = conf.readline()
                if line[:-1].isdigit():  # remove '\r\n' form end line
                    stream_data_length.append(int(line[:-1]))
            if len(stream_data_length) == 1:
                stream_data_length = stream_data_length[0]
            else:
                raise NotImplemented('stream not constant size not implemented yet')

            stream_is_variable_size = header.source_info in self.variable_size_streams and header.stream_number in self.variable_size_streams[header.source_info]
            if header.stream_chunks_len and header.stream_chunks_len[0] > 0 and not stream_is_variable_size:
                if stream_data_length % header.stream_chunks_len[0] != 0:
                    header.flag_same_stream_size_with_declaration = False
                if stream_data_length > header.stream_chunks_len[0] * header.stream_data_len:
                    raise IndexError('The extracted data is too short - please check declaration file')
            header.stream_declaration_len = stream_data_length
            return stream_data_length

    def _find_num_elements(self, str_num):
        n_elements = 0
        for rec in self._payload_dict[str_num]:
            elements = int.from_bytes(rec[4:8], byteorder='little', signed=False)
            n_elements = max(elements, n_elements)

        return n_elements


    @staticmethod
    def _check_stream_definition(mudp_stream_def_path):
        """
        Overwrite environment variable (just for this python instance) and check if it can be accessed.

        :param mudp_stream_def_path: path to folder with mudp streams definitions
        :type mudp_stream_def_path: str
        """
        class MudpPathNotProvided(Exception):
            """
            Raised when the MUDP_STREAM_DEFINITIONS_PATH is not provided neither as
            input string nor as environment variable
            """
            pass

        if mudp_stream_def_path is not None:
            streams_definition_path = mudp_stream_def_path
        else:
            try:
                streams_definition_path = os.environ['MUDP_STREAM_DEFINITIONS_PATH']
            except KeyError:
                raise MudpPathNotProvided('A path to the mudp stream definition folder is not provided:\n '
                 'Use MudpHandler parameter (mudp_stream_def_path) or set environment variable MUDP_STREAM_DEFINITIONS_PATH')
        return streams_definition_path


class MUDPDeclaration:
    """
    MUDPDeclaration is class that handles reading, interpreting and formatting MUDP declaration file to different python
    data_types structures.
    """
    def __init__(self, file_path, unknown_type_handling=False, f_unknown_size_warnings=True, unknown_size=4):
        """
        :param file_path: path to mudp declaration file
        :type file_path: str.
        :param unknown_type_handling: should unknown size be handled with const value
        :type unknown_type_handling: bool, optional
        :param f_unknown_size_warnings: should unknown size warnings be raised
        :type f_unknown_size_warnings: bool, optional
        :param unknown_size: size of UNKNOWN type in declaration file; default 4
        :type unknown_size: int, optional
        """
        self.file_path = file_path
        self.names = []
        self.types = []
        self.f_unknown_size_warnings = f_unknown_size_warnings
        self.unknown_type_handling = unknown_type_handling
        self.unknown_size = unknown_size

    def decode_declaration_file(self):
        """
        Execute reading declaration file with .mudp stream definitions.
        :return:
        """
        table_names = [self.names]
        table_types = [self.types]
        table_repeat = []
        # This dictionay contains signals which are represended by structures that are at least 2d
        self._n_dimensional_structures = {}

        with open(self.file_path, 'r') as conf:
            for line in conf:
                var_type, var_name, length = self._handle_declaration_name(line[:-1])
                if var_name is None:
                    continue
                elif var_name == 'UNKNOWN':
                    table_names[-1] += [var_name] * self.unknown_size
                    table_types[-1] += ['B'] * self.unknown_size  # single byte length
                elif var_name == 'PADDING':
                    table_names[-1] += [var_name] * length
                    table_types[-1] += ['B'] * length  # single byte length
                elif var_name == 'REPEAT':
                    table_names.append([])
                    table_types.append([])
                    table_repeat.append(length)
                elif var_name == 'END_REPEAT':
                    table_names[-2] += table_names[-1] * table_repeat[-1]
                    table_types[-2] += table_types[-1] * table_repeat[-1]

                    table_names = table_names[:-1]
                    table_types = table_types[:-1]
                    table_repeat = table_repeat[:-1]
                else:
                    if len(table_repeat) >= 2: # check if input structure is at least 2 dimensional
                        self._n_dimensional_structures[var_name] = tuple(table_repeat)
                    table_names[-1].append(var_name)
                    if isinstance(var_type, list):
                        table_types[-1] += var_type
                    elif isinstance(var_type, str):
                        table_types[-1].append(var_type)
                    else:
                        raise TypeError(f'{var_type} is unhandled type')

    def _handle_declaration_name(self, line):
        """
        Map single line from stream definition file to tuple of python variable type, variable name and it byte length.
        :param line: single declaration line from  mudp declaration file
        :return: tuple(type, name, byte_length)
        """
        var_type = None
        var_name = None
        length = 0
        if line.isdigit():
            pass
        elif re.match('PADDING', line):
            var_name = 'PADDING'
            line = line.replace('PADDING', '')
            length = int(line)
        elif 'END_REPEAT' in line:
            var_name = 'END_REPEAT'
        elif 'REPEAT' in line:
            var_name = 'REPEAT'
            line = line.replace('REPEAT', '')
            length = int(line)
        else:
            last_space = line.rfind(' ')
            var_name = line[last_space + 1:]
            full_type = line[:last_space].lower()
            if full_type in ['single', 'float', 'float32', 'float32_t', 'real32_t', 'f360_fpn_t']:
                var_type = 'f'
                length = 4
            elif full_type in ['double', 'float64', 'float64_t', 'real64_t', 'f360_dpn_t']:
                var_type = 'd'
                length = 8
            elif full_type in ['uint8', 'uint8_t', 'unsigned char', 'bool', 'boolean', 'boolean_t', 'unsigned8_t',
                               'f360_ui8n_t', 'f360_booln_t']:
                var_type = 'B'
                length = 1
            elif full_type in ['int8', 'sint8', 'int8_t', 'sint8_t', 'signed char', 'char', 'signed8_t', 'f360_si8n_t']:
                var_type = 'b'
                length = 1
            elif full_type in ['uint16', 'uint16_t', 'unsigned short', 'unsigned16_t', 'f360_ui16n_t']:
                var_type = 'H'
                length = 2
            elif full_type in ['int16', 'sint16', 'int16_t', 'sint16_t', 'signed short', 'short', 'signed16_t',
                               'f360_si16n_t']:
                var_type = 'h'
                length = 2
            elif full_type in ['uint32', 'uint32_t', 'unsigned int', 'unsigned32_t', 'f360_ui32n_t']:
                var_type = 'I'
                length = 4
            elif full_type in ['int32', 'sint32', 'int32_t', 'sint32_t', 'signed int', 'int', 'signed32_t',
                               'f360_si32n_t']:
                var_type = 'i'
                length = 4
            elif full_type in ['uint64', 'uint64_t', 'unsigned64_t', 'f360_ui64n_t']:
                var_type = 'Q'
                length = 8
            elif full_type in ['int64', 'sint64', 'int64_t', 'sint64_t', 'signed64_t', 'f360_si64n_t']:
                var_type = 'q'
                length = 8
            else:
                if self.unknown_type_handling:
                    length = self.unknown_size
                    if self.unknown_size == 1:
                        var_type = 'B'
                    elif self.unknown_size == 2:
                        var_type = 'H'
                    elif self.unknown_size == 4:
                        var_type = 'I'
                    elif self.unknown_size == 8:
                        var_type = 'Q'
                    else:
                        raise TypeError(f'unknown "unknown_size" = {self.unknown_size}. avaliable values [1, 2, 4, 8] ')

                    if self.f_unknown_size_warnings:
                        warnings.warn(f'unknown variable type {full_type} with name {var_name} in line {line}. Variable'
                                      f' will be added to output')
                else:
                    var_name = 'UNKNOWN'
                    if self.f_unknown_size_warnings:
                        warnings.warn(f'unknown variable type {full_type} with name {var_name} in line {line}. Variable'
                                      f' will not be added to output')

        return var_type, var_name, length

    def get_mudp_structure(self):
        """
        :return: iterable structure with decoded data
        """
        return zip(self.names, self.types)

    def get_mudp_data_type_structure(self):
        """
        :return:  default python unpack data_type
        """
        return '=' + ''.join(self.types)

    def get_numpy_data_format(self):
        """
        :return:  numpy frombuffer data_type
        """
        return np.dtype(','.join(self.types))

    def generate_streamdef(self, n_elements, temp_streamdef_name):
        f_first_line = True
        logstruct_size = 0

        with open(self.file_path) as f:
            for line in f:
                if(f_first_line):
                    logstruct_size = 16 + n_elements * int(line)
                    buffer = str(logstruct_size) + '\n' + 'uint8_t f360log_version\nPADDING3\nuint32_t num_elements\nuint64_t data_timestamp_us\nREPEAT' + str(n_elements) + '\n'
                    f_first_line = False
                else:
                    buffer += line
        buffer += 'END_REPEAT\n'

        f = open(temp_streamdef_name, "w")
        f.write(buffer)
        f.close()
        return logstruct_size

class Header:
    """
    Header class is responsible for handling stream header and decoding data needed for stream payload read.
    """

    HEADER_LENGTH = 32

    STREAM_PC_TIME_POSITION = 0
    STREAM_C_TIME_POSITION = 4
    STREAM_VERSION_INF_POSITION = 8
    SOURCE_TX_CNT_POSITION = 10
    STREAM_TX_TIME_POSITION = 12
    STREAM_SOURCE_INFO_POSITION = 16
    STREAM_SRC_1_POSITION = 17
    STREAM_SRC_2_POSITION = 18
    STREAM_SRC_3_POSITION = 19
    STREAM_REF_POSITION = 20
    STREAM_DATA_LEN_POSITION = 24
    STREAM_TX_CNT_POSITION = 26
    STREAM_NUMBER_POSITION = 27
    STREAM_VERSION_POSITION = 28
    STREAM_CHUNKS_POSITION = 29
    STREAM_CHUNK_ID_POSITION = 30
    STREAM_STR_3_POSITION = 31

    def __init__(self, buffer):
        """
        :param buffer: input date with encoded header.
        :type buffer: binary str.
        """
        self.version_info, self.big_endian = self.get_version(buffer)
        self.stream_number = buffer[self.STREAM_NUMBER_POSITION]
        p = self.STREAM_DATA_LEN_POSITION
        self.stream_data_len = unpack('>H' if self.big_endian else 'H', buffer[p:p + 2])[0]
        self.stream_version = buffer[self.STREAM_VERSION_POSITION]
        self.stream_chunks_len = []
        self.payload_start_file_position = []
        self.pc_time = []
        self.c_time = []
        self.source_tx_cnt = []
        self.source_tx_time = []
        self.stream_ref_index = []
        self.stream_ref_idx_unique = []
        self.stream_tx_cnt = []
        self.stream_chunk_idx = []
        self.reserved_src_1 = []
        self.reserved_src_2 = []
        self.reserved_src_3 = []
        self.reserved_str_3 = []
        self.stream_data_len_all = []

        self.configuration_file_path = None
        self.log_length = None
        self.source_info = None
        self.stream_declaration_len = None
        self.flag_same_stream_size_with_declaration = True
        self.f_stream_chunks_is_0 = True if buffer[self.STREAM_CHUNKS_POSITION] == 0 else False

    def get_extended_data(self, buffer, in_file_localization):
        """
        Read specific byte positions from header and write it as attributes.
        :param buffer: mudp buffer, binary string
        :param in_file_localization: infile pointer
        :return:
        """
        self.source_info = buffer[self.STREAM_SOURCE_INFO_POSITION]
        # TODO:
        # if structure already has source_info and it doesn't match
        # or src2_opt not empty and not equal reserved_src_2
        #   do sth -> look matlab read_mudp_data.m -> line 245
        self.payload_start_file_position.append(in_file_localization)
        p = self.STREAM_PC_TIME_POSITION
        self.pc_time.append(
            unpack('>I' if self.big_endian else 'I', buffer[p:p + 4])[0])
        p = self.STREAM_C_TIME_POSITION
        self.c_time.append(
            unpack('>I' if self.big_endian else 'I', buffer[p:p + 4])[0])
        p = self.SOURCE_TX_CNT_POSITION
        self.source_tx_cnt.append(
            unpack('>H' if self.big_endian else 'H', buffer[p:p + 2])[0])
        p = self.STREAM_TX_TIME_POSITION
        self.source_tx_time.append(
            unpack('>I' if self.big_endian else 'I', buffer[p:p + 4])[0])
        p = self.STREAM_REF_POSITION
        self.stream_ref_index.append(
            unpack('>I' if self.big_endian else 'I', buffer[p:p + 4])[0])
        p = self.STREAM_DATA_LEN_POSITION
        self.stream_data_len_all.append(
            unpack('>H' if self.big_endian else 'H', buffer[p:p + 2])[0])
        p = self.STREAM_CHUNKS_POSITION
        self.stream_chunks_len.append(
            unpack('>B' if self.big_endian else 'B', buffer[p:p + 1])[0])
        self.stream_tx_cnt.append(buffer[self.STREAM_TX_CNT_POSITION])
        self.stream_chunk_idx.append(buffer[self.STREAM_CHUNK_ID_POSITION])
        self.reserved_src_1.append(buffer[self.STREAM_SRC_1_POSITION])
        self.reserved_src_2.append(buffer[self.STREAM_SRC_2_POSITION])
        self.reserved_src_3.append(buffer[self.STREAM_SRC_3_POSITION])
        self.reserved_str_3.append(buffer[self.STREAM_STR_3_POSITION])

    @staticmethod
    def get_version(buffer):
        """
        Extract buffer encoding version.

        :param buffer: input date with encoded header
        :type buffer: binary str.
        :returns:
            - int - version
            - bool - file is encoded with big_endian
        """
        p = Header.STREAM_VERSION_INF_POSITION
        version_info = unpack('H', buffer[p:p + 2])[0]
        big_endian = False
        if version_info == 0x18a1:
            big_endian = True
            version_info = unpack('>H' if big_endian else 'H', buffer[p:p + 2])[0]
        elif version_info != 0xa118:
            warnings.warn("Wrong header version - not supported")
        return version_info, big_endian

    @staticmethod
    def get_stream_length(buffer):
        """
        Extract corresponding stream length from header.

        :param buffer: input date with encoded header
        :type buffer: binary str.
        :return: int - length of binary payload for given stream
        """
        _, endian = Header.get_version(buffer)
        p = Header.STREAM_DATA_LEN_POSITION
        return unpack('>H' if endian else 'H', buffer[p:p + 2])[0]

    def header_chunks_sanity_check(self, drops_dict):
        """
        Checking if chunks in stream are consistent, removing corrupted chunks(data) from header.

        :param drops_dict: dropped chunks
        :type drops_dict: dict.
        """
        drops_dict[self.stream_number] = {'missing_indexes': [],
                                          'corrupted_stream_ref_index': []}
        ok_indexes, corrupted_indexes = [], []
        beg_idx, end_idx = 0, self.stream_chunks_len[0]

        while end_idx <= len(self.stream_ref_index):
            stream_ref_index_chunk = self.stream_ref_index[beg_idx: end_idx]

            if self.check_if_stream_ref_index_chunk_is_ok(stream_ref_index_chunk):
                self.stream_ref_idx_unique.append(stream_ref_index_chunk[0])
                ok_indexes.extend(range(beg_idx, end_idx))
                beg_idx = end_idx
            else:
                """
                Case when chunk is for example [100, 100, 101] for stream chunk == 3. 
                Stream ref index 100 is corrupted, so beg_index should be set on 101, chunks 100 should be removed.
                """
                last_ref_idx_in_chunk = stream_ref_index_chunk[-1]
                last_ref_idx_position = stream_ref_index_chunk.index(last_ref_idx_in_chunk)
                next_chunk_idx = beg_idx + last_ref_idx_position

                corrupted_indexes.extend(range(beg_idx, next_chunk_idx))
                drops_dict[self.stream_number]['corrupted_stream_ref_index'].extend(
                    self.stream_ref_index[beg_idx: next_chunk_idx])
                beg_idx = next_chunk_idx

            if end_idx >= len(self.stream_ref_index):
                break
            else:
                end_idx = beg_idx + self.stream_chunks_len[beg_idx]

        corrupted_stream_ref_index = np.array(self.stream_ref_index)[corrupted_indexes]
        drops_dict[self.stream_number]['missing_indexes'] = corrupted_indexes
        drops_dict[self.stream_number]['corrupted_stream_ref_index'] = np.unique(corrupted_stream_ref_index).tolist()
        self.keep_only_ok_indexes(ok_indexes)

    def check_if_stream_ref_index_chunk_is_ok(self, stream_ref_index_chunk):
        """
        Check if stream_ref_index_chunk is not corrupted - it should contains single unique value.
        :param stream_ref_index_chunk:
        :return:
        """
        unique_ref_indexes_in_chunk = list(set(stream_ref_index_chunk))
        only_single_index = len(unique_ref_indexes_in_chunk) == 1
        return only_single_index

    def keep_only_ok_indexes(self, ok_indexes):
        """
        Remove corrupted indexes from header attributes.
        :return: None
        """
        for attribute_name, values_list in self.__dict__.items():
            if isinstance(values_list, list) and attribute_name != 'stream_ref_idx_unique':
                self.__dict__[attribute_name] = list(np.array(values_list)[ok_indexes])


if __name__ == '__main__':
    '''
    Example of .mudp parser usage
    '''
    from aspe.parsers.MudpParser.f360_defaults_configs import unknown_size_per_stream
    mudp_stream_def_path = r'C:\bd\git\OT_ObjectTracking\modules\F360Core\sw\zResimSupport\stream_definitions'
    parser = MudpParser(streams_to_read=(3, 4, 5, 6, 7, 9, 10, 17, 19, 27, 70, 71, 72), source_to_parse=35,
                        unknown_size_per_stream=unknown_size_per_stream,
                        mudp_stream_def_path=mudp_stream_def_path)

    mudp_log_path = r"C:\Users\jjvblt\Documents\F360\Tickets\DFF-1578\SCW\rRf360t8000309v205p50_resimmed\BMW_SRR5_MID_715489_20200924_5_1_3_018_rRf360t8000309v205p50_resimmed.mudp"
    output = parser.parse(mudp_log_path)
    a = 3