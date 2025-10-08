import numpy as np
import pandas as pd

from aspe.gui.drawers.DrawerExtractedDataFactory import DrawersExtractedDataFactory
from aspe.utilities.SupportingFunctions import remove_log_extension


class DrawersController:
    def __init__(self):
        self.main_controller = None
        self.scan_indexes = None
        self.drawers_setting_widget = None
        self.current_scan_index = None
        self.drawers = []
        self.drawers_state = []

    def add_drawer(self, data_model):
        drawer = DrawersExtractedDataFactory.create_extracted_data_drawer(self, data_model)
        self.drawers_setting_widget.add_data_source_settings(drawer.settings_widget, data_model.source_info.root_folder)
        self.insert_data_source(drawer)
        drawer_index = self.drawers.index(drawer)

        for _index, state in enumerate(self.drawers_state):
            if drawer.name == state["name"] and state["log_index"] == drawer_index:
                drawer.load_state(state)
                break

        if self.drawers[drawer_index].log_index is None:
            self.drawers[drawer_index].log_index = drawer_index
            self.drawers_state.append(drawer.get_state())

        self.update_scan_indexes()
        drawer.plot_scan_index(self.current_scan_index)

    def remove_drawer_using_data_model(self, data_model_to_remove):
        for index, drawer in enumerate(self.drawers):
            if drawer is not None and drawer.data_model == data_model_to_remove:
                drawer.clear()
                self.drawers[index] = None
                self.drawers_setting_widget.remove_source_from_combo_box(data_model_to_remove.source_info)
                self.update_scan_indexes()
                break

    def remove_all_drawers(self):
        for index, drawer in enumerate(self.drawers):
            if drawer is not None:
                self.drawers_setting_widget.remove_source_from_combo_box(drawer.data_model.source_info)
                drawer.clear()
                self.drawers[index] = None
                self.update_scan_indexes()

    def clear_sources(self):
        self.drawers = []

    def plot_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        for drawer in self.drawers:
            if drawer is not None:
                drawer.plot_scan_index(scan_index)

    def update_scan_indexes(self):
        scan_indexes_list = [ds.scan_indexes for ds in self.drawers if ds is not None]
        if len(scan_indexes_list) > 0:
            self.scan_indexes = np.sort(np.unique(np.hstack(scan_indexes_list)))
        else:
            self.scan_indexes = np.arange(1, 10)  # some defaults
        self.main_controller.update_scan_index_values(self.scan_indexes)

    def on_select(self, selected_drawer, selected_data_set):
        self.main_controller.on_select(selected_drawer, selected_data_set)

    def on_deselect(self):
        self.main_controller.on_deselect()

    def get_state(self):
        self.save_state()
        return self.drawers_state

    def load_state(self, state):
        self.drawers_state = state

    def update_state(self):
        for index, drawer in enumerate(self.drawers):
            if drawer is not None:
                for _index_state, state in enumerate(self.drawers_state):
                    if drawer.name == state["name"] and state["log_index"] == index:
                        drawer.load_state(state)
                        self.update_scan_indexes()
                        drawer.plot_scan_index(self.current_scan_index)
                        break

    def save_state(self):
        for index, drawer in enumerate(self.drawers):
            if drawer is not None:
                for index_state, state in enumerate(self.drawers_state):
                    if drawer.name == state["name"] and state["log_index"] == index:
                        self.drawers_state[index_state] = drawer.get_state()

    def insert_data_source(self, data_source):
        inserted = False
        for index, drawer_slot in enumerate(self.drawers):
            if drawer_slot is None:
                self.drawers[index] = data_source
                inserted = True
                break
        if not inserted:
            self.drawers.append(data_source)

    def get_legend(self):
        # preparing the data
        title_log_name = ""
        resim_suffix = "rRf360"
        legend_dict = {"log_name": [],
                       "drawer_name": [],
                       "subdrawer_name": [],
                       "color": [],
                       "style": []}
        legend_df = pd.DataFrame(legend_dict)
        # appending legend from every drawer
        for maindrawer in self.drawers:
            legend_df = maindrawer.append_legend_part(legend_df, legend_dict)
        legend_df = legend_df.sort_values("log_name").reset_index().drop(columns="index")
        # check if legend is not empty
        if not legend_df.empty:
            # check which logs are resimulated
            for idx, _row in legend_df.iterrows():
                log_name = legend_df.loc[idx, "log_name"]
                if resim_suffix in log_name:
                    legend_df.loc[idx, "log_nick_name"] = remove_log_extension(log_name[log_name.find(resim_suffix)::])
                    legend_df.loc[idx, "is_resim"] = True
                else:
                    legend_df.loc[idx, "log_nick_name"] = log_name
                    legend_df.loc[idx, "is_resim"] = False
            # protection against log names changed by users
            # if original log is detected then take its nick name as title
            # else take first name in dataframe without resim suffix
            if not legend_df["is_resim"].any():
                if legend_df["is_resim"].value_counts()[False] == 1:
                    title_log_name = legend_df[legend_df["is_resim"] == False]["log_nick_name"].item()
                    title_log_index = legend_df.index[legend_df["is_resim"] == False].to_list()
                    legend_df.loc[title_log_index, "log_nick_name"] = "Original log"
            else:
                log_name = legend_df.loc[0, "log_name"]
                title_log_name = log_name[0:log_name.find(legend_df.loc[0, "log_nick_name"]) - 1]
                if not title_log_name:
                    title_log_name = "no-name log"
            # count characters per legend line to scale font size
            for idx in range(len(legend_df)):
                legend_df.loc[idx, "characters_per_legend_line"] = len(legend_df.loc[idx, "style"] + " " +
                                                                       legend_df.loc[idx, "log_nick_name"] + " - " +
                                                                       legend_df.loc[idx, "drawer_name"] + " - " +
                                                                       legend_df.loc[idx, "subdrawer_name"])
        return legend_df, title_log_name