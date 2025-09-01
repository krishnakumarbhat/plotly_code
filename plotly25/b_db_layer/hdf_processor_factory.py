from b_db_layer.allsensor_hdf_parser_wrapper import AllsensorHdfParserWrapper
from b_db_layer.Persensor_hdf_parser_wrapper import PersensorHdfParserWrapper

class HdfProcessorFactory:
    def __init__(self, file_dict):
        self.file_dict = file_dict

    def process(self):
        address_map , name =  self.file_dict[0] , self.file_dict[1]
        if "multi"== name:
            PersensorHdfParserWrapper(address_map).parse()

        else:
            PersensorHdfParserWrapper(address_map).parse()
                # AllsensorHdfParserWrapper(input_file, output_file).parse()
                
                
                
# import h5py
# import numpy as np

# class HdfProcessor:
#     def __init__(self, data_dict):
#         self.data_dict = data_dict

#     def process(self):
#         position_map = {}
#         dictt , naem = self.data_dict[0] ,self.data_dict[1]
#         print(dictt,naem)
#         if naem == "multi":
#             multiwrapper_hdf.process()
#         for i,j  in self.data_dict.items():
#             print(f"hisdfhid : {i,j}")

        
#         inputs = list(self.data_dict.keys())
#         outputs = list(self.data_dict.values())
        
#         # Create a matrix to store inputs and outputs
#         matrix_size = (len(inputs), 2)  # 2 columns for input and output
#         data_matrix = np.empty(matrix_size, dtype=object)

#         # Fill the matrix with inputs and outputs
#         for i in range(len(inputs)):
#             data_matrix[i, 0] = inputs[i]   # Input file path
#             data_matrix[i, 1] = outputs[i]  # Output file path

#         # Create a hashmap to store positions
        
#         for i in range(len(inputs)):
#             position_map[(i, 0)] = inputs[i]
#             position_map[(i, 1)] = outputs[i]

#         print("Data Matrix:")
#         print(data_matrix)

#         print("Position Map:")
#         print(position_map)

#         # Save to HDF5 file (example filename)
#         output_file_name = 'processed_data.hdf5'
#         with h5py.File(output_file_name, 'w') as f:
#             f.create_dataset('data_matrix', data=data_matrix)


