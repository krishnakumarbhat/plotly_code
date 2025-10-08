import h5py
from hdfhelper import HDFParsingHelper

if __name__ == '__main__':
    print("HDF parser")
    hdf_group_dataset_map = {}
    hdf_file = h5py.File(r"C:\Users\gjlkfw\Downloads\SampleHDF_File.h5", 'r')
    hdf_helper = HDFParsingHelper()

    hdf_group_list = hdf_helper.get_hdf_groups(hdf_file)


    for group in hdf_group_list:
        print("---------------------------")
        print("Group ",group)
        hdf_dataset_list = list(hdf_helper.get_hdf_datasets(hdf_file,group))
        for dataset in hdf_dataset_list:
            print("         Have Datasets ",dataset)
            hdf_group_dataset_map[group] = hdf_dataset_list

    print("------------------------------------------------------")
    print("Paresed [Available ---> HDF Group and dataset Map]")
    print("------------------------------------------------------")
    for key,value in hdf_group_dataset_map.items():
        print(f"{key} : {value}")
    print("------------------------------------------------------")


    for key,value in hdf_group_dataset_map.items():


        for val in value:
             print(f"{key}"+"/"+str(val))
             dset=hdf_helper.get_data_in_dataset(hdf_file,key+"/"+str(val))
             print("dset",dset)
