import numpy as np
import pandas as pd
import pyqtgraph as pg
from PyQt5.Qt import QFont

from aspe.gui.drawers.abstract_drawers.DrawerLeaf import DrawerLeaf
from aspe.gui.views.settings.DrawerLabelsSettingsWidget import DrawerLabelsSettingsWidget


class DrawerLabels(DrawerLeaf):
    def __init__(self, parent, name: str, plot_item: pg.PlotItem, color: str):
        super().__init__(parent, name, plot_item, color)
        self.is_enabled = False
        self.labeled_signal = "position_x"
        self.show_signal_name = True
        self.graphic_object = []  # place for TextItems
        self.labels_text = {}
        self.font_size = 8

    def set_data(self, signals_df, raw_signals_df, x_signal_signature, y_signal_signature):
        self.reset_data()
        if raw_signals_df is not None:
            if x_signal_signature in raw_signals_df.columns:
                raw_signals_df = raw_signals_df.rename(columns={x_signal_signature: x_signal_signature+"2",
                                                                y_signal_signature: y_signal_signature+"2"})
            self.data_df = pd.concat([signals_df, raw_signals_df], axis=1)
            self.data_df = self.data_df.loc[:,~self.data_df.columns.duplicated()]
            self.data_df = pd.DataFrame(self.data_df, columns=self.data_df.columns.sort_values())
        else:
            self.data_df = signals_df
            self.data_df = pd.DataFrame(self.data_df, columns=self.data_df.columns.sort_values())
        self.set_labels_text()
        samples_per_scan = self.data_df.groupby(by="scan_index").count()
        max_count_per_scan = samples_per_scan.to_numpy().max()

        for _ in range(0, max_count_per_scan):
            text_item = pg.TextItem()
            self.plot_item.addItem(text_item)
            self.graphic_object.append(text_item)

        self.data_df["df_indexes"] = self.data_df.index
        data_df_grouped = self.data_df.groupby(by="scan_index")
        self.x_data = data_df_grouped[x_signal_signature].apply(np.array).to_dict()
        self.y_data = data_df_grouped[y_signal_signature].apply(np.array).to_dict()
        self.df_indexes = data_df_grouped["df_indexes"].apply(np.array).to_dict()

    def reset_data(self):
        for go in self.graphic_object:
            self.plot_item.removeItem(go)
        self.graphic_object = []

    def plot_scan_index(self, scan_index):
        if self.is_enabled:
            try:
                x = self.x_data[scan_index]
                y = self.y_data[scan_index]
                labels = self.labels_text[scan_index]
                for n in range(0, len(x)):
                    text_item = self.graphic_object[n]
                    text_item.setPos(y[n], x[n])
                    text_item.setText(labels[n])
                for n in range(len(x), len(self.graphic_object)):
                    self.graphic_object[n].setText("")
            except KeyError:
                pass

    def plot_empty_data(self):
        for text_item in self.graphic_object:
            text_item.setText("")

    def set_labels_text(self):
        if self.labeled_signal in self.data_df:
            text_array = self.data_df.loc[:, self.labeled_signal].to_numpy().astype(str)
            if len(text_array.shape) > 1:
                text_array = text_array[:, 0]  # handle repeated dataframe column
            if self.show_signal_name:
                signal_name_arr = np.full(len(text_array), f"{self.labeled_signal}: ")
                text_array = np.char.add(signal_name_arr, text_array)
                # display id and reduced id at the same time in the label if they are plotted only reduced objects
                if self.parent.name == "AllObjects" and self.parent.current_scan_index is not None and \
                        self.parent.only_reduced_objects and self.labeled_signal == "id":
                    additional_text_array = self.data_df.loc[:, "reduced_id"].to_numpy().astype(str)
                    additional_signal_name_arr = np.full(len(additional_text_array), " (reduced_id: ")
                    additional_text_array = np.char.add(additional_signal_name_arr, additional_text_array)
                    additional_text_array = np.char.add(additional_text_array, np.full(len(text_array), ")"))
                    text_array = np.char.add(text_array, additional_text_array)
            ser = pd.Series(text_array, index=self.data_df.scan_index)
            self.labels_text = ser.groupby(ser.index).apply(np.array).to_dict()

    def create_settings_widget(self):
        self.settings_widget = DrawerLabelsSettingsWidget(self.name, self)
        self.settings_widget.fill_signals_combo_box(self.data_df.columns)
        return self.settings_widget

    def set_color(self, color):
        self.color = color
        for text_item in self.graphic_object:
            text_item.setColor(color)

    def set_show_name(self, show_name):
        self.show_signal_name = show_name
        self.set_labels_text()
        self.plot_scan_index(self.parent.current_scan_index)

    def set_labeled_signal(self, signal_signature):
        self.labeled_signal = signal_signature
        self.set_labels_text()
        self.plot_scan_index(self.parent.current_scan_index)

    def set_font_size(self, font_size):
        self.font_size = font_size
        for text_item in self.graphic_object:
            text_item.setFont(QFont("", font_size))

    def get_state(self):
        return {
            "is_enabled": self.is_enabled,
            "color": self.color,
            "show_signal_name": self.show_signal_name,
            "font_size": self.font_size,
            "labeled_signal": self.labeled_signal,
        }

    def load_state(self, state):
        is_enabled = state["is_enabled"]
        if is_enabled:
            self.enable()
        else:
            self.disable()
        self.set_color(state["color"])
        self.set_labeled_signal(state["labeled_signal"])
        self.set_show_name(state["show_signal_name"])
        self.set_font_size(state["font_size"])
        self.settings_widget.load_state(state)