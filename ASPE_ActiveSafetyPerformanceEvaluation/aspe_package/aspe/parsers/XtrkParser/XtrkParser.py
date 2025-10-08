import os
import traceback

import _io
import numpy as np
import pandas as pd

from aspe.parsers.aptiv_data_parser import DataParser


class XtrkParser(DataParser):
    def __init__(self):
        self._init_constants()
        self.xtrk = None

    def parse(self, log_file_path: str):
        self._check_file(log_file_path)

        with open(log_file_path, 'rb') as fp:
            pre_header = self._read_pre_header(fp, log_file_path)
            sup_block = self._read_header(fp, pre_header)
            try:
                S, sup_block, L, L_partial = self._init_data(sup_block, pre_header)
            except Exception as e:
                fp.close()
                print('Exception caught in data initialization:')
                raise e
            sz_record = pre_header[5]
            num_records = pre_header[6]
            for m in range(0, num_records, L):
                if m + L > num_records:
                    f_last = True
                    mm = np.arange(m, num_records, 1)
                else:
                    f_last = False
                    mm = m + np.arange(0, L, 1)
                length = mm.size
                buf = np.fromfile(fp, dtype='uint8', count=L * sz_record)
                for SupBlock in sup_block:
                    name = SupBlock['name']
                    for blocks in SupBlock['blocks']:
                        field = blocks['field']
                        num_dims = blocks['numDims']
                        typeout = blocks['typeout']
                        sub_array_size = blocks['subArraySize']
                        sub_array_size[0] = length
                        if f_last and (L_partial != 0):
                            rd_idx = blocks['rdIdxLast']
                        else:
                            rd_idx = blocks['rdIdx']
                        try:
                            b = buf[rd_idx]
                            proc_buf = np.frombuffer(b, dtype=typeout)
                            if S[name][field] is not None:
                                if num_dims == 1:
                                    S[name][field][mm] = proc_buf.reshape(sub_array_size, order='F')
                                elif num_dims == 2:
                                    S[name][field][mm, :] = proc_buf.reshape(sub_array_size, order='F')
                                elif num_dims == 3:
                                    S[name][field][mm, :, :] = proc_buf.reshape(sub_array_size, order='F')
                                elif num_dims == 4:
                                    S[name][field][mm, :, :, :] = proc_buf.reshape(sub_array_size, order='F')
                                elif num_dims == 5:
                                    S[name][field][mm, :, :, :, :] = proc_buf.reshape(sub_array_size, order='F')
                                elif num_dims == 6:
                                    S[name][field][mm, :, :, :, :, :] = proc_buf.reshape(sub_array_size, order='F')
                                else:
                                    fp.close()
                                    raise Exception('Unusual high number of dimension. Check input or add another case to the method')
                        except Exception as e:
                            print('An error occured while casting reading content from {} with field {}.\nSkipping field. The error message was {}'.format(name, field, e))
                            traceback.print_exc()
        self.xtrk = S
        return self.xtrk

    def get_reduced_objects(self, use_ai_keys=False):
        if self.xtrk is not None:
            if use_ai_keys:
                return self._get_reduced_objects_AI()
            else:
                return self._get_reduced_objects()

    def get_processed_detections(self):
        if self.xtrk is not None:
            return self._get_processed_detections()

    def get_clusters(self):
        if self.xtrk is not None:
            return self._get_clusters()

    def get_all_objects(self):
        if self.xtrk is not None:
            return self._get_all_objects()

    def get_guardrails(self):
        if self.xtrk is not None:
            return self._get_guardrails()

    def _get_reduced_objects(self):
        reduced_id = self.xtrk['object_track']['reduced_id']
        idx = np.where((reduced_id > 0) & (reduced_id <= self.NUMBER_OF_REDUCED_OBJECT_TRACKS))
        data = np.array([])
        columns = list()
        for key in self.xtrk['object_track'].keys():
            this_data = self.xtrk['object_track'][key][idx[0], idx[1]]
            if len(this_data.shape) == 1:
                this_data = this_data[:, np.newaxis]
                columns.append(key)
            elif len(this_data.shape) == 2:
                this_data = this_data
                columns.extend([key + '_' + str(i) for i in range(0, this_data.shape[1])])
            elif len(this_data.shape) == 3:
                [x, y] = np.meshgrid(np.arange(0, this_data.shape[2], 1), np.arange(0, this_data.shape[1], 1))
                x = x.reshape(-1)
                y = y.reshape(-1)
                this_data = this_data.reshape(this_data.shape[0], -1)  # CAREFUL: this is rowmajor so C style order don't be confused that it's different then matlab
                columns.extend([key + '_' + str(row) + '_' + str(col) for row, col in zip(y, x)])
            else:
                raise Exception('Unexpected high dimensional array. Check your xtrk or adapt code to support higher dimensional arrays')
            data = np.concatenate((data, this_data), axis=1) if data.size else this_data
        df_cols = ['tracker_iter']
        df_cols.extend(columns)
        tracker_iter = (idx[0] + 1)[:, np.newaxis]
        data = np.concatenate((tracker_iter, data), axis=1)

        return pd.DataFrame(columns=df_cols, data=data)

    def _get_reduced_objects_AI(self):
        # This method is just to be compatible with the old data parsing stucture from AI
        reduced_id = self.xtrk['object_track']['reduced_id']
        idx = np.where((reduced_id > 0) & (reduced_id <= self.NUMBER_OF_REDUCED_OBJECT_TRACKS))
        keys_to_fetch = ['id',
                         'vcs_position.longitudinal',
                         'vcs_position.lateral',
                         'length',
                         'width',
                         'vcs_heading',
                         'vcs_pointing',
                         'speed',
                         'f_moving',
                         'f_moveable',
                         'time_last_detected',
                         'time_created',
                         'timestamp',
                         'tracker_iter',
                         'detgid_placeholder',
                         'object_class',
                         'confidenceLevel']

        ai_key_mapping = {'id': 'track_id',
                          'vcs_position.longitudinal': 'x',
                          'vcs_position.lateral': 'y',
                          'length': 'length',
                          'width': 'width',
                          'vcs_heading': 'vcs_heading',
                          'vcs_pointing': 'vcs_pointing',
                          'speed': 'speed',
                          'f_moving': 'moving',
                          'f_moveable': 'moveable',
                          'time_last_detected': 'time_last_detected',
                          'time_created': 'time_created',
                          'timestamp': 'tracker_timestamp',
                          'tracker_iter': 'tracker_iter',
                          'detgid_placeholder': 'det_gids',
                          'object_class': 'object_class',
                          'confidenceLevel': 'confidenceLevel'}
        data = np.array([])
        columns = list()
        for key in keys_to_fetch:
            if key == 'tracker_iter':
                this_data = (idx[0] + 1)[:, np.newaxis]
            elif key == 'detgid_placeholder':
                this_data = np.repeat('[]', idx[0].shape[0], axis=0)
            elif key == 'object_class':
                this_data = np.repeat('0', idx[0].shape[0], axis=0)
            elif key == 'timestamp':
                time_data = np.repeat(self.xtrk['trkrInfo']['time'], reduced_id.shape[1], axis=1)
                this_data = time_data[idx[0], idx[1]] * 1000
            else:
                this_data = self.xtrk['object_track'][key][idx[0], idx[1]]
            if len(this_data.shape) == 1:
                this_data = this_data[:, np.newaxis]
            else:
                this_data = this_data
            if this_data.shape[1] > 1:
                columns.extend([ai_key_mapping[key] + '_' + str(i) for i in range(0, this_data.shape[1])])
            else:
                columns.append(ai_key_mapping[key])
            data = np.concatenate((data, this_data), axis=1) if data.size else this_data
        return pd.DataFrame(columns=columns, data=data).apply(pd.to_numeric,errors='ignore')

    def _get_processed_detections(self):
        return None

    def _get_clusters(self):
        active_cluster_ids = self.xtrk['trkrInfo']['active_cluster_ids']
        idx = np.where(active_cluster_ids != 0)
        active_ids = active_cluster_ids[idx[0], idx[1]] - 1  # Python indexing starts from 0
        data = np.array([])
        columns = list()
        for key in self.xtrk['clusters'].keys():
            this_data = self.xtrk['clusters'][key][idx[0], active_ids]
            this_data = this_data if len(this_data.shape) > 1 else this_data[:, np.newaxis]
            data = np.concatenate((data, this_data), axis=1) if data.size else this_data
            if this_data.shape[1] > 1:
                columns.extend([key + '_' + str(i) for i in range(0, this_data.shape[1])])
            else:
                columns.append(key)
        df_cols = ['tracker_iter']
        df_cols.extend(columns)
        tracker_iter = (idx[0] + 1)[:, np.newaxis]
        data = np.concatenate((tracker_iter, data), axis=1)

        return pd.DataFrame(columns=df_cols, data=data)

    def _get_all_objects(self):
        active_object_ids = self.xtrk['trkrInfo']['active_obj_ids']
        idx = np.where(active_object_ids != 0)
        active_ids = active_object_ids[idx[0], idx[1]] - 1  # Python indexing starts from 0
        data = np.array([])
        columns = list()
        for key in self.xtrk['object_track'].keys():
            this_data = self.xtrk['object_track'][key][idx[0], active_ids]
            if len(this_data.shape) == 1:
                this_data = this_data[:, np.newaxis]
                columns.append(key)
            elif len(this_data.shape) == 2:
                this_data = this_data
                columns.extend([key + '_' + str(i) for i in range(0, this_data.shape[1])])
            elif len(this_data.shape) == 3:
                [x, y] = np.meshgrid(np.arange(0, this_data.shape[2], 1), np.arange(0, this_data.shape[1], 1))
                x = x.reshape(-1)
                y = y.reshape(-1)
                this_data = this_data.reshape(this_data.shape[0], -1)  # CAREFUL: this is rowmajor so C style order don't be confused that it's different then matlab
                columns.extend([key + '_' + str(row) + '_' + str(col) for row, col in zip(y, x)])
            else:
                raise Exception('Unexpected high dimensional array. Check your xtrk or adapt code to support higher dimensional arrays')
            data = np.concatenate((data, this_data), axis=1) if data.size else this_data
        df_cols = ['tracker_iter']
        df_cols.extend(columns)
        tracker_iter = (idx[0] + 1)[:, np.newaxis]
        data = np.concatenate((tracker_iter, data), axis=1)

        return pd.DataFrame(columns=df_cols, data=data)

    def _get_guardrails(self):
        print('To be implemented')
        return None

    def _init_data(self, sb, pre_head):
        S = {}
        sz_rec = pre_head[5]
        num_records = pre_head[6]
        L = np.int32(np.maximum(1, np.floor(np.float32(self.MAX_BUF_SIZE) / np.float32(sz_rec))))
        for SupBlock in sb:
            name = SupBlock['name']
            num_repeats = SupBlock['numRepeats']
            base = SupBlock['base']
            sz_type = SupBlock['szType']
            this_field = {}
            for blocks in SupBlock['blocks']:
                array_size = (num_records, num_repeats, blocks['rows'], blocks['cols'])
                array_size = [x for x in array_size if x > 1]  # remove singleton
                if len(array_size) == 1:
                    array_size.append(1)
                blocks['numDims'] = len(array_size)
                sub_array_size = array_size[1:]
                blocks['subArraySize'] = [L]
                blocks['subArraySize'].extend(sub_array_size)
                typeout = blocks['typeout']
                try:
                    this_field[blocks['field']] = np.zeros(shape=array_size, dtype=typeout)
                except Exception as e:
                    this_field[blocks['field']] = None
                    print('Warning unknown data type. Setting S[' + name + '][' + blocks['field'] + '] to None\nError message was {}'.format(e))
                    traceback.print_exc()
                num_bytes = blocks['numBytes']
                rows = blocks['rows']
                cols = blocks['cols']
                start = base + blocks['offset']  # Originally 1 +base +blocks['offset'] but parser was intended for MATLAB which uses one based indexing
                blocks['rdIdx'] = self._gen_index(L, num_bytes, num_repeats, start, sz_rec, sz_type, rows, cols)
                L_partial = num_records % L
                if 0 != L_partial:
                    blocks['rdIdxLast'] = self._gen_index(L_partial, num_bytes, num_repeats, start, sz_rec, sz_type, rows, cols)
            S[name] = this_field
        return S, sb, L, L_partial

    def _gen_index(self, l_records, n_bytes, n_repeats, n_start, sz_records, sz_type, rows, cols):
        if n_bytes == 1 and n_repeats == 1 and rows * cols == 1:
            idx = n_start + sz_records * np.arange(0, l_records, 1).reshape(-1, 1)
        elif n_bytes == 1 and n_repeats == 1 and rows * cols > 1:
            idx = n_start + sz_records * np.arange(0, l_records, 1).reshape(-1, 1) + (n_bytes * np.arange(0, rows * cols, 1)).reshape(1, -1)
        elif n_bytes == 1 and n_repeats > 1 and rows * cols == 1:
            idx = n_start + sz_records * np.arange(0, l_records, 1).reshape(-1, 1) + (sz_type * np.arange(0, n_repeats, 1)).reshape(1, -1)
        elif n_bytes == 1 and n_repeats > 1 and rows * cols > 1:
            first_dim = (sz_records * np.arange(0, l_records, 1)).reshape(-1, 1, 1)
            middle_dim = (sz_type * np.arange(0, n_repeats, 1)).reshape(1, -1, 1)
            last_dim = (n_bytes * np.arange(0, rows * cols, 1)).reshape((1, 1, -1))
            idx = n_start + first_dim + middle_dim + last_dim
        elif n_bytes > 1 and n_repeats == 1 and rows * cols == 1:
            idx = n_start + np.arange(0, n_bytes, 1).reshape(-1, 1) + sz_records * np.arange(0, l_records, 1).reshape(1, -1)
        elif n_bytes > 1 and n_repeats == 1 and rows * cols > 1:
            first_dim = np.arange(0, n_bytes, 1).reshape(-1, 1, 1)
            middle_dim = (sz_records * np.arange(0, l_records, 1)).reshape(1, -1, 1)
            last_dim = (n_bytes * np.arange(0, rows * cols, 1)).reshape((1, 1, -1))
            idx = n_start + first_dim + middle_dim + last_dim
        elif n_bytes > 1 and n_repeats > 1 and rows * cols == 1:
            first_dim = np.arange(0, n_bytes, 1).reshape(-1, 1, 1)
            middle_dim = (sz_records * np.arange(0, l_records, 1)).reshape(1, -1, 1)
            last_dim = (sz_type * np.arange(0, n_repeats, 1)).reshape((1, 1, -1))
            idx = n_start + first_dim + middle_dim + last_dim
        elif n_bytes > 1 and n_repeats > 1 and rows * cols > 1:
            first_dim = np.arange(0, n_bytes, 1).reshape(-1, 1, 1, 1)
            sec_dim = (sz_records * np.arange(0, l_records, 1)).reshape(1, -1, 1, 1)
            third_dim = (sz_type * np.arange(0, n_repeats, 1)).reshape((1, 1, -1, 1))
            last_dim = (n_bytes * np.arange(0, rows * cols, 1)).reshape((1, 1, 1, -1))
            idx = n_start + first_dim + sec_dim + third_dim + last_dim
        else:
            raise Exception('Unexpected case in member _gen_index()')
        return idx.reshape(-1, order='F')

    def _read_header(self, fp, pre_head):
        sz_hdr, magic_num, version, num_sup_blocks, sz_header, sz_record, num_records = pre_head
        if version == self.VERSION_RAWTRACK1:
            maxtypeidentifier = 15 + 1
        else:
            maxtypeidentifier = 63 + 1
        sz_int32 = 4
        buf = np.fromfile(fp, dtype='uint8', count=sz_header - sz_hdr * sz_int32)
        n = 0
        SupBlock = list()
        for k in range(0, num_sup_blocks):
            this_supblock = {}
            c = buf[n:n + self.MAXIDENTIFIER]
            n += self.MAXIDENTIFIER
            p_str = self._ascii2str(c)
            this_supblock['name'] = p_str
            this_supblock['numRepeats'] = self._bytes2int32(buf[n:n + sz_int32])
            n += sz_int32
            this_supblock['numBlocks'] = self._bytes2int32(buf[n:n + sz_int32])
            n += sz_int32
            blocks = list()
            for j in range(0, this_supblock['numBlocks']):
                this_block = {}
                this_block['szType'] = self._bytes2int32(buf[n:n + sz_int32])
                n += sz_int32
                c = buf[n:n + self.MAXIDENTIFIER]
                n += self.MAXIDENTIFIER
                this_block['field'] = self._ascii2str(c)
                c = buf[n:n + maxtypeidentifier]
                n += maxtypeidentifier
                p_str = self._ascii2str(c)
                if p_str not in self.INTYPES.keys():
                    raise Exception('Unknown input type: {}'.format(p_str))
                this_block['typein'] = p_str
                hijacked_typeout = self.INTYPES[p_str]
                # c = buf[n:n + maxtypeidentifier] #ABE: Unnecessary because we dont want to have matlab types
                n += maxtypeidentifier  # ABE: But dont forget to increase the counter and screw the matlab typeout
                # p_str = self._ascii2str(c) #ABE: Unnecessary because we dont want to have matlab types
                this_block['typeout'] = hijacked_typeout
                this_block['offset'] = self._bytes2int32(buf[n:n + sz_int32])
                n += sz_int32
                this_block['numBytes'] = self._bytes2int32(buf[n:n + sz_int32])
                n += sz_int32
                this_block['rows'] = self._bytes2int32(buf[n:n + sz_int32])
                n += sz_int32
                this_block['cols'] = self._bytes2int32(buf[n:n + sz_int32])
                n += sz_int32
                blocks.append(this_block)
            this_supblock['blocks'] = blocks
            SupBlock.append(this_supblock)
        SupBlock[0]['base'] = np.int32(0)
        SupBlock[0]['szType'] = SupBlock[0]['blocks'][0]['szType']
        for k in range(1, num_sup_blocks):
            SupBlock[k]['szType'] = SupBlock[k]['blocks'][0]['szType']
            SupBlock[k]['base'] = SupBlock[k - 1]['base'] + SupBlock[k - 1]['numRepeats'] * SupBlock[k - 1]['szType']
        return SupBlock

    @staticmethod
    def _ascii2str(np_ascii_arr):
        return "".join([chr(item) for item in np_ascii_arr[0:np.argmin(np_ascii_arr)]])

    @staticmethod
    def _bytes2int32(np_bytes):
        return np.int32(int.from_bytes(np_bytes.tolist(), byteorder='little', signed=True))

    def _read_pre_header(self, file: _io.BufferedReader, log_file_path: str):
        sz_hdr = 5
        buf = np.fromfile(file, dtype='int32', count=sz_hdr)
        magic_num = buf[0]
        version = buf[1]
        num_sup_blocks = buf[2]
        sz_header = buf[3]
        sz_record = buf[4]
        if magic_num != self.MAGIC_RAWTRACK:
            file.close()
            raise Exception('File not of excpected type')
        if version not in [self.VERSION_RAWTRACK1, self.VERSION_RAWTRACK2]:
            file.close()
            raise Exception('File version not supported')
        os_filesize = os.path.getsize(log_file_path)
        num_records = (float(os_filesize) - float(sz_header)) / float(sz_record)
        if not num_records.is_integer():
            file.close()
            raise Exception('Unexpected file size')
        return [sz_hdr, magic_num, version, num_sup_blocks, sz_header, sz_record, int(num_records)]

    def _init_constants(self):
        self.MAGIC_RAWTRACK = np.int32(1383552107)
        self.VERSION_RAWTRACK1 = np.int32(1)
        self.VERSION_RAWTRACK2 = np.int32(2)
        self.MAXIDENTIFIER = 63 + 1
        self.MAX_BUF_SIZE = 8*1024*1024
        self.INTYPES = {'int8_T': 'int8',
                        'uint8_T': 'uint8',
                        'int16_T': 'int16',
                        'uint16_T': 'uint16',
                        'int32_T': 'int32',
                        'uint32_T': 'uint32',
                        'int64_T': 'int64',
                        'uint64_T': 'uint64',
                        'int8_t': 'int8',
                        'uint8_t': 'uint8',
                        'int16_t': 'int16',
                        'uint16_t': 'uint16',
                        'int32_t': 'int32',
                        'uint32_t': 'uint32',
                        'int64_t': 'int64',
                        'uint64_t': 'uint64',
                        'boolean_T': 'bool',
                        'real32_T': 'float32',
                        'real64_T': 'float64',
                        'float': 'single',
                        'double': 'double'}
        self.N32BITTYPES = ['int32', 'uint32']
        self.N8BITTYPES = ['int8', 'uint8', 'bool']
        self.N16BITTYPES = ['int16', 'uint16']
        self.N64BITTYPES = ['int64', 'uint64']
        self.NFLOATTYPES = ['float32', 'single']
        self.NDOUBLETYPES = ['float64', 'double']
        self.NUMBER_OF_REDUCED_OBJECT_TRACKS = 125


if __name__ == '__main__':
    import time

    filename = r"C:\logs\BYK-627_DEX-690\SRR_DEBUG\rRf360t4150309v205p50_core_2_20\20200131T101426_20200131T101446_543078_LB36408_SRR_DEBUG_rRf360t4150309v205p50_core_2_20.xtrk"

    parser = XtrkParser()
    t1 = time.time()
    parsed = parser.parse(log_file_path=filename)
    print(f'{time.time() - t1}')
    print('Done')
