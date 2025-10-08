import sys


class Logger:
    def __init__(self):
        self.logged_info = list()
        self.log_info = str()
        self.separator = "\n \t \t"

    def log(self, log_idx, path, log_info, ok):
        if not ok:
            self.log_info = f'\n{str(log_idx)}. {path} \n \t' + self.log_info
            error_info = self.separator.join(list(map(str, sys.exc_info())))
            self.log_info += log_info + f'\n \t \t {error_info} \n'
            self.logged_info.append(self.log_info)
            print(self.log_info)
            self.log_info = str()
        else:
            self.log_info = log_info + ' \n \t'

    def save_logged_info(self, log_info_path, parsing_info, extracting_info):
        with open(log_info_path, 'w') as file:
            file.writelines(parsing_info + '\n')
            file.writelines(extracting_info + '\n')
            file.writelines('\n \n')
            for err in self.logged_info:
                file.writelines(err)
