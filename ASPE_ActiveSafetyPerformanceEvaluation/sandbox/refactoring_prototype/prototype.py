import pandas as pd
# import pickle
import timeit
import _pickle as pickle


class DataContainer:
    def __init__(self):
        self.name = str()
        self.metadata = dict()
        self.data = pd.DataFrame()

    def to_dict(self):
        return {'metadata': self.metadata,
                'data': self.data}


class ExtractedData:
    def __init__(self, extracted_data_fields=('Sensors', 'Detection', 'Host vehicle data', 'Objects', 'LCDA', 'Target Selection', 'Road model', 'AEB'),
                 metadata_fields=('Data Version', 'Extractor Version', 'Log name', 'Extractor type', 'SW version', 'HW version', 'Max possible scan index')):
        self.metadata = dict.fromkeys(metadata_fields, None)
        self.extracted_data = dict(zip(list(extracted_data_fields), [DataContainer() for _ in range(len(extracted_data_fields))]))

    def to_dict(self):
        extracted_data_dict = {'metadata': self.metadata,
                               'extracted_data': {}}
        for field, value in self.extracted_data.items():
            extracted_data_dict['extracted_data'][field] = value.to_dict()
        return extracted_data_dict

    # TEST METHOD
    def fill_df(self):
        df_path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\rRf360t3060304v202r1\rRf360t3060304v202r1_aggregated_data.pickle'
        with open(df_path, 'rb') as handle:
            example_df = pickle.load(handle)

        new_df = pd.concat([example_df['estimated_obj_data'].reset_index() for _ in range(2)])
        path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\refactoring_prototyping\big_df.pickle'
        with open(path, 'wb') as handle:
            pickle.dump(new_df, handle)
        for value in self.extracted_data.values():
            value.data = new_df.copy(deep=True)

    def save_as_obj(self):
        path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\refactoring_prototyping\data_as_obj.pickle'
        with open(path, 'wb') as handle:
            pickle.dump(self, handle)

    def save_as_dict(self):
        path = r'\\10.224.186.68\AD-Shared\F360\Logs\AIT-646_RNA_Basic_KPI_report\2_FTP\full_loop\SW201_SUV_V1\LSS\LSS_CURVES\refactoring_prototyping\data_as_dict.pickle'
        with open(path, 'wb') as handle:
            pickle.dump(self.to_dict(), handle)


def example():
    extracted_obj = ExtractedData()
    extracted_obj.fill_df()
    extracted_obj.save_as_dict()
    extracted_obj.save_as_obj()


if __name__ == '__main__':
    example()