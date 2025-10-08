from scipy.io import loadmat

from aspe.parsers.aptiv_data_parser import DataParser


class MatParser(DataParser):
    def __init__(self):
        super(MatParser, self).__init__()
        self.parsed_data = None

    def parse(self, log_file_path: str) -> dict:
        self.parsed_data = loadmat(log_file_path, simplify_cells=True)
        return self.parsed_data


if __name__ == '__main__':
    path = r"PATH/TO/MATFILE"

    parser = MatParser()
    parsed = parser.parse(log_file_path=path)

    print('Done')
