import pandas as pd


class DataViewerController:
    def __init__(self, data_viewer_widget):
        self.data_viewer_widget = data_viewer_widget

    def add_data_model(self, data_model):
        data_sets_names = get_obj_attributes_names(data_model.extracted)
        data_model_dict = {}
        for data_set_name in data_sets_names:
            data_set = getattr(data_model.extracted, data_set_name)
            if data_set is not None:
                data_set_dict = {}
                data_set_atts = get_obj_attributes_names(data_set)
                for data_set_att in data_set_atts:
                    att = getattr(data_set, data_set_att)
                    if isinstance(att, pd.DataFrame):
                        data_set_dict[data_set_att] = att
                    # todo add properties
                if len(data_set_dict) > 0:
                    data_model_dict[data_set_name] = data_set_dict
        self.data_viewer_widget.add_data_model(data_model, data_model_dict)

    def remove_data_model(self, data_model):
        self.data_viewer_widget.remove_data_model(data_model)

    def remove_all_data(self):
        self.data_viewer_widget.remove_all_data()


def get_obj_attributes_names(obj):
    return [a for a in dir(obj) if not a.startswith("__") and not callable(getattr(obj, a))]
