"""
dvl_parser.py
**************
Parser for dvl binary files.
"""
import os
import warnings
from abc import ABC, abstractmethod
from enum import Enum
from math import ceil
from struct import unpack

from cantools import database as can_database

from aspe.parsers.aptiv_data_parser import DataParser


class DVLParser(DataParser):
    def __init__(self, dbc_config: dict):
        """
        Main DVL parser class: it handles reading raw binary file and converts it to human interpretable object.

        :param path: path to dvl file
        :type path: str
        :param dbc_config: configuration for used dbc files, input is dictionary in form: {channel_num: path_to_dbc}
        :type dbc_config: dict
        """
        super().__init__()
        self.dbc_config = dbc_config
        self.dbc_per_channel = self._get_dbc_structure(dbc_config)
        self.not_give_dbc_channels = []
        self.not_found_ids_in_dbc_files = []
        self.raw_data = {'channel': [], 'id': [], 'timestamp': [], 'length': [], 'payload': []}
        self.parsed_data = {channel: {} for channel, _ in self.dbc_per_channel.items()}
        self.ids_declared = {channel: self._get_all_ids(dbc_database)
                             for channel, dbc_database in self.dbc_per_channel.items()}
        self.id_names = {channel: self.get_id_name_dict(dbc_database)
                         for channel, dbc_database in self.dbc_per_channel.items()}

    def parse(self, dvl_file_path: str):
        """
        Iterate over records/packages in the dlv stream.
        """
        _, ext = os.path.splitext(dvl_file_path)
        if not ext == '.dvl':
            raise ValueError('Please provide .dvl file type')

        with open(dvl_file_path, 'rb') as dvl_raw:
            for dvl_package in DvlStream(dvl_raw):
                # TODO waklaround due to missing mux in dbc file - if this issue will be solved delete try
                try:
                    self._extract_raw_data_from_single_package(dvl_package)
                    self._handle_single_package(dvl_package)
                except Exception:
                    warnings.warn('Unhandled exception missing mux in dbc file')
        output = {
            'parsed_data': self.parsed_data,
            'path': dvl_file_path,
            'dbc_config': self.dbc_config,
        }
        return output

    @staticmethod
    def _get_dbc_structure(dbc_per_channel):
        """
        Load all dbc files and return cantools database structure

        :param dbc_per_channel: dictionary of dbc files per each stream; i.e.
                dbc_per_channel = {}
                dbc_per_channel[3] = 'path_to_dbc_1.dbc'
        :type dbc_per_channel: dict
        :return: dict - dictionary with cantools database (for channels)
        """
        out = {}
        for channel, path in dbc_per_channel.items():
            out[channel] = can_database.load_file(path)
        return out

    def _handle_single_package(self, dvl_package):
        """
        For dvl record/package decode a payload and save it to storage structure

        :param dvl_package: single dvl record/package
        :type dvl_package: dict
        """
        if dvl_package.channel in self.dbc_per_channel:
            if dvl_package.id in self.ids_declared[dvl_package.channel]:
                decoded_message = self.dbc_per_channel[dvl_package.channel].decode_message(dvl_package.id,
                                                                                           dvl_package.payload)
                id_object = self._get_object_by_channel_and_id(dvl_package.channel, dvl_package.id)
                id_object[dvl_package.timestamp] = decoded_message
            else:
                warnings.warn(
                    'undeclared id: {} in dbc file for channel {} '.format(dvl_package.id, dvl_package.channel))
        elif dvl_package.channel not in self.not_give_dbc_channels:
            self.not_give_dbc_channels.append(dvl_package.channel)

    def _extract_raw_data_from_single_package(self, dvl_package):
        """
        Extracts raw data.

        :param dvl_package:  single dvl record/package
        :type dvl_package: dict
        """
        raw_id = dvl_package.id
        raw_payload = dvl_package.payload.hex()
        raw_timestamp = dvl_package.timestamp
        raw_channel = dvl_package.channel
        raw_length = dvl_package.length

        self.raw_data['channel'].append(raw_channel)
        self.raw_data['id'].append(raw_id)
        self.raw_data['payload'].append(raw_payload)
        self.raw_data['timestamp'].append(raw_timestamp)
        self.raw_data['length'].append(raw_length)

    @staticmethod
    def _get_all_ids(dbc_database):
        """
        Extracts all message ids for given dbc dataset

        :param dbc_database: dbc database (in canntools format)
        :type dbc_database: cantools dataset
        :return: list - all message ids in this database
        """
        ids = []
        for message in dbc_database.messages:
            ids.append(message.frame_id)
        return ids

    @staticmethod
    def get_id_name_dict(dbc_database):
        """
        :param dbc_database: dbc database
        :type dbc_database: cantools dataset
        :return: dict - ids with corresponding names
        """
        id_name = {}
        for message in dbc_database.messages:
            id_name[message.frame_id] = message.name
        return id_name

    def _get_object_by_channel_and_id(self, channel, can_id):
        """
        Supporting function for _handle_single_package() returns structure to save data; if such not exists it creates
        it
        :param channel: data channel
        :type channel: int
        :param can_id: data id
        :type can_id: int
        :return: dict - structure to save data
        """
        out = self.parsed_data[channel]
        if can_id not in out:
            out[can_id] = {}
        out = out[can_id]
        return out


class DVLError(Exception):
    """
    Exception raised for errors in the input.
    """

    def __init__(self, message):
        self.message = message


class DVLFileVersion(Enum):
    """
    DVL Supported Versions:
        * V2.2
        * V3.1
        * V3.2
    """
    Unknown = 0
    V2_2 = 1
    V3_1 = 2
    V3_2 = 3


class DvlHeader:
    """
    This class handles the dvl header; it detects dvl stream version set single record length
    """

    def __init__(self, file_stream):
        header_buffer = file_stream.read(64)
        version_var = str(header_buffer, 'ASCII')
        self.version = DVLFileVersion.Unknown

        if version_var.startswith('CAN log V2.2'):
            self.version = DVLFileVersion.V2_2
            self.record_length = 14

        if version_var.startswith('CAN log V3.1'):
            self.version = DVLFileVersion.V3_1
            self.record_length = 18

        if version_var.startswith('DVLFile V3.2'):
            self.version = DVLFileVersion.V3_2
            self.record_length = 19

        if self.version == DVLFileVersion.Unknown:
            raise DVLError("Unknown file format {0} ".format(version_var))


class DvlPackage(ABC):
    """ Interface for specific dvl package version"""
    finished = False

    @abstractmethod
    def __init__(self):
        pass


class DvlPackageV22(DvlPackage):
    def __init__(self, file_stream):
        super().__init__()
        buffer = file_stream.read(14)
        self.id = unpack('H', buffer[0:2])[0]
        self.timestamp = unpack('I', buffer[2:6])[0]
        self.channel = self.timestamp % 10
        self.payload = buffer[8:]
        warnings.warn("not tested doe to lack of logs - relic of previous parser version")


class DvlPackageV31(DvlPackage):
    """
    ----------------------------------------------------------
    Bytes:     4       4     8             1
        TimeStamp - ID - Data - Channel & Length
    ----------------------------------------------------------
    The ID field contains this structure:
    Bits: MSB        1        1         1         29      LSB
             IsInfoFrame SPARE   IsStdFrame  CAN_ID
    ----------------------------------------------------------
    """

    def __init__(self, file_stream):
        super().__init__()
        buffer = file_stream.read(18)
        self.timestamp = unpack('I', buffer[0:4])[0]
        self.id = unpack('I', buffer[4:8])[0] & 0x1fffffff
        self.channel = buffer[16] >> 4
        self.length = buffer[16] & 0xf
        self.payload = buffer[8:16]


class DvlPackageV32(DvlPackage):
    """
     On V32, B0 contains the record type and fragment index
    ----------------------------------------------------------------
     Data     Tstamp - CanID - Length - Channel & flags - Data
     CAN        4        4       1             1            8
    ----------------------------------------------------------------
     Data     Tstamp - HdrFlags - Cha&ID - Cycle cnt - Length - Data
     FR Idx0    4         2         2          1         1        8
     FR Idx>0   0         0         0          0         0       18
    ----------------------------------------------------------------
    ----------------------------------------------------------------
     Data        Tstamp - CanID - Length - Channel & flags - Data
     CanFd Idx=0   4      4(brs)      1          1            8
     CanFd Idx>0   0      0           0          0            18
     Position of first data byte
     """
    HEADER_DATA_LENGTH = 8
    OTHER_DATA_MAX_LENGTH = 18
    RECORD_BYTE_SIZE = 19
    FORMATTING_SIZES = [8, 16, 32, 64]

    def __init__(self, file_stream):
        super().__init__()
        buffer = file_stream.read(self.RECORD_BYTE_SIZE)
        if len(buffer) < self.RECORD_BYTE_SIZE:
            self.finished = True
            return
        self.fragIndex = buffer[0] & 0x0F
        self.recType = (buffer[0] >> 4) & 0x0F
        self.timestamp = unpack('I', buffer[1:5])[0]
        self.id = unpack('I', buffer[5:9])[0] & 0x1fffffff
        self.length = buffer[9]
        self.flags = (buffer[10] >> 4)
        self.channel = (buffer[10]) & 0x0F
        self.payload = buffer[11:]
        to_read_length = self.length - self.HEADER_DATA_LENGTH
        if to_read_length > 0:
            self._read_split_data(file_stream, to_read_length)

    def _read_split_data(self, file_stream, to_read_length):
        """
        This function is called when the whole message haven't fit into one buffer stream; it completes payload
        with further streams
        :param file_stream: open dlv file stream
        :param to_read_length: record's payload length
        :return:
        """
        for _ in range(int(ceil(to_read_length / self.OTHER_DATA_MAX_LENGTH))):
            buffer = file_stream.read(19)
            self.payload += buffer[1:to_read_length + 1]
            to_read_length -= len(buffer[1:to_read_length + 1])
        self._format_output_payload_to_proper_size()

    def _format_output_payload_to_proper_size(self):
        """
        This function adds zeros suffix to self.payload to meet proper output size (FORMATTING_SIZES)
        :return:
        """
        payload_len = len(self.payload)
        if not (payload_len in self.FORMATTING_SIZES):
            for size in self.FORMATTING_SIZES:
                if payload_len < size:
                    sufix_zeros_size = size - payload_len
                    self.payload += bytearray(sufix_zeros_size)
                    break


class DvlStream:
    """
    DVL WALKER FUNCTION

        /-----^\
       /==     |
   +-o/   ==B) |
      /__/-----|
         =====
         ( \ \ \
          \ \ \ \
           ( ) ( )
           / /  \ \
         / /     | |
         /        |
       _^^oo    _^^oo

    """

    def __init__(self, file_stream):
        self.file_stream = file_stream

        self.header = DvlHeader(file_stream)

        file_length = os.fstat(file_stream.fileno()).st_size
        self.numberOfRecords = (file_length - 64) / self.header.record_length
        self.position = 0
        self.done = False
        self.current = None

    def next(self):
        if self.position >= self.numberOfRecords:
            return False

        if self.header.version == DVLFileVersion.V2_2:
            self.current = DvlPackageV22(self.file_stream)

        if self.header.version == DVLFileVersion.V3_1:
            self.current = DvlPackageV31(self.file_stream)

        if self.header.version == DVLFileVersion.V3_2:
            self.current = DvlPackageV32(self.file_stream)
            if self.current.finished:
                self.done = True

        self.position += 1
        return True

    def __iter__(self):
        return self

    def __next__(self):
        if self.next() and not self.done:
            return self.current
        else:
            raise StopIteration()


if __name__ == '__main__':
    # simple example of use
    dbc_config = {4: r'PATH\TO\DBC\FILE.dbc'}
    parser = DVLParser(dbc_config)
    dvl_path = (r"C:\logs\OXTS_RT-Range\RNA_SRR5\Opole_CW19\RCTA\rRf360t4060306v204p50"
                r"\FTP402_TC1_90_150938_001_rRf360t4060306v204p50.dvl")
    output = parser.parse(dvl_path)