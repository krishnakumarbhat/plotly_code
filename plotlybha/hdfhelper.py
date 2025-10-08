class HDFParsingHelper:

    def __init__(self):
        print("HDFPerSensorParsing")

    @staticmethod
    def get_hdf_groups(hdffile):
        print("get_hdf_groups")
        hdf_group = list(hdffile.keys())
        hdf_group_path = []
        hdf_dataset = []
        for group in hdf_group:
            # print("group", group)
            # print(" subgroups", hdffile[group].keys())
            for subgroup in hdffile[group].keys():
                group_path = hdffile[group + '/' + subgroup]
                # print(group_path)
                # print("  group_path", group_path.name)

                try:
                    if group_path.shape:
                        # print(" DS ", group_path.name)
                        hdf_dataset.append(group_path.name)

                except AttributeError:
                    # print(" Group Names", group_path.name)
                    hdf_group_path.append(group_path.name)

        return hdf_group_path

    @staticmethod
    def get_hdf_datasets(hdffile, group):
        # print(f"get_hdf_datasets under group {group}")
        hdf_datasets = hdffile[group]
        return hdf_datasets

    @staticmethod
    def get_data_in_dataset(hdffile, datasetname):
        print(f"get_dataset_data")
        data = hdffile[datasetname]
        result = data[()]
        #print(result)
        return result
