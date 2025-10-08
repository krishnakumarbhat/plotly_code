import re
import os
class LogFile():
    def __init__(self, log_address: str):
        self.address = log_address if '\\' in log_address else None
        self.name = log_address.split('\\')[-1]
        self.ds = re.findall('_[d|D]?[s|S]?\d?\d_', self.name)[0][1:-1]
        self.triger_time = re.findall('_[0-2]\d\d\d\d\d_', self.name)[0][1:-1]
        self.triger_date = re.findall('_\d\d\d\d\d\d\d\d_', self.name)[0][1:-1]
        self.log_index = re.findall('_\d\d\d\d', self.name)[-1][1:]

def reduce_loglist(log_list, path):
    reduced_loglist = {}
    for file in os.listdir(path):
        log_file = LogFile(file)
        ds_key = f'DS_{log_file.ds}'
        if os.path.splitext(file)[-1] == '.MF4':
            if ds_key not in reduced_loglist:
                reduced_loglist.update({ds_key: []})
            reduced_loglist[ds_key].append(file)#.replace('SRR_DEBUG', 'BN_FASETH').replace('deb', 'fas'))
    new_loglist = {}
    for ds_name, logs in log_list.items():
        if ds_name not in new_loglist.keys():
            new_loglist.update({ds_name: []})
        for single_filename in log_list[ds_name]:
            if os.path.basename(single_filename) in reduced_loglist[ds_name]:
                new_loglist[ds_name].append(os.path.join(path, single_filename))

    return new_loglist
