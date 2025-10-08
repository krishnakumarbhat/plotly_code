class HdfProcessor:
    def __init__(self, data):
        self.data_dict = data

    def process(self):
        print(f"Processing {self.data_dict}.")
