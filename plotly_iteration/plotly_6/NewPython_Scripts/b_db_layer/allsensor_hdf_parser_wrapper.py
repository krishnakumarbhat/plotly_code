from b_db_layer.Persensor_hdf_parser_wrapper import PersensorHdfParserWrapper
class AllsensorHdfParserWrapper(PersensorHdfParserWrapper):
    def __init__(self, address):
        self.address = address

    def parse(self):
        parsed_data = self._parse_hdf_file(self.input_file)
        # Here you can save parsed_data to the output file if needed

    def _parse_hdf_file(self, file_name):
        with h5py.File(file_name, 'r') as f:
            data_dict = {}
            scan_index = f['Header/ScanIndex'][()]
            vehicle_data = f['data/veh']
            detection_data = f['data/det']

            for index in range(len(scan_index)):
                key = f'scanindex{index + 1}'
                data_dict[key] = {
                    'veh': [
                        np.array(vehicle_data['speed'][index]),
                        np.array(vehicle_data['yaw'][index])
                    ],
                    'det': [
                        np.array(detection_data['range'][index]),
                        np.array(detection_data['rangerate'][index]),
                        np.array(detection_data['azimuth'][index]),
                        np.array(detection_data['elevation'][index]),
                        np.array(detection_data['amplitude'][index])
                    ]
                }
            return data_dict