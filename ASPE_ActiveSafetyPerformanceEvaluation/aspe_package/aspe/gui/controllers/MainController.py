import fnmatch
import json
import os
from functools import partial
from pathlib import Path

import numpy as np
from PyQt5 import QtGui, QtWidgets

from aspe.evaluation.RadarObjectsEvaluation.DataStructures import PEObjectsEvaluationOutputSingleLog
from aspe.gui.controllers.DataModelsController import DataModelsManager
from aspe.gui.controllers.FixViewController import FixViewController
from aspe.gui.controllers.SignalPlotterController import SignalPlotterController
from aspe.gui.controllers.VelocityProfilePlotterController import VelocityProfilePlotterController
from aspe.gui.controllers.VelocityProfilePlotterControllerPE import VelocityProfilePlotterControllerPE

SETTINGS_STATE_FILE_NAME = "settings_state.json"


def get_user_data_dir():
    controller_file_dir = Path.resolve(Path(__file__).parent)
    project_dir = controller_file_dir.parent
    user_data_dir = project_dir / ".user_data"
    if not Path.is_dir(user_data_dir):
        Path.mkdir(user_data_dir)
    return user_data_dir


def find(pattern, path):
    result = []
    for root, _dirs, files in os.walk(path):
        for name in files:
            if fnmatch.fnmatch(name, pattern):
                result.append(Path(root) / name)
    return result


class MainController:
    def __init__(self):
        self.data_models_controller = DataModelsManager()
        self.bird_eye_view = None
        self.playback_controller = None
        self.drawers_controller = None
        self.selection_controller = None
        self.data_viewer_controller = None
        self.signal_plotter_controller = SignalPlotterController()
        self.velocity_profile_plotter_controller = None
        self.current_scan_index = None
        self.is_playing = False
        self.scan_indexes = None
        self.user_data_dir = get_user_data_dir()
        self.fix_view_controller = FixViewController()
        
    def update_current_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        self.selection_controller.update_current_scan_index(scan_index)
        self.signal_plotter_controller.update_current_scan_index(scan_index)
        self.velocity_profile_plotter_controller.update(self.selection_controller.selected_drawer)
        self.drawers_controller.plot_scan_index(scan_index)
        self.fix_view_controller.update(self.selection_controller.selected_drawer)
        self.update_fix_view()

    def on_play(self):
        self.is_playing = True

    def on_stop(self):
        self.is_playing = False

    def update_scan_index_values(self, scan_indexes):
        vals = scan_indexes[~np.isnan(scan_indexes.astype(float))]
        self.scan_indexes = vals
        self.playback_controller.update_scan_index_values(vals)

    def on_select(self, selected_data_source, selected_data_set):
        self.selection_controller.on_select(selected_data_set)
        self.velocity_profile_plotter_controller.update(self.selection_controller.selected_drawer)
        self.fix_view_controller.update(self.selection_controller.selected_drawer)

    def on_deselect(self):
        self.selection_controller.on_deselect()
        self.velocity_profile_plotter_controller.clear()
        self.fix_view_controller.clear()

    def save_state(self, ui):
        state = {
            "drawers_settings": self.drawers_controller.get_state(),
        }

        filename, _ = QtGui.QFileDialog.getSaveFileName(
            ui,
            "Save File As",
            "",
            "JSON File (*.json)",
        )

        state_file_path = Path(self.user_data_dir) / filename
        with Path.open(state_file_path, "w") as write_file:
            json.dump(state, write_file, indent=2)

        return state

    def load_state(self, filename=None):
        state_file_path = \
            Path(filename) if filename is not None else self.user_data_dir / SETTINGS_STATE_FILE_NAME

        if Path.is_file(state_file_path):
            with Path.open(state_file_path, "r") as read_file:
                state = json.load(read_file)
                state_drawers_settings = state["drawers_settings"]
                self.drawers_controller.load_state(state_drawers_settings)

    def load_data_using_source_info(self, source_info):
        data_model = self.data_models_controller.load_data_using_source_info(source_info)
        # Change velocity profile plotter controller for pe data
        if isinstance(data_model.extracted, PEObjectsEvaluationOutputSingleLog):
            self.velocity_profile_plotter_controller = VelocityProfilePlotterControllerPE()
        else:
            self.velocity_profile_plotter_controller = VelocityProfilePlotterController()
        self.drawers_controller.add_drawer(data_model)
        self.data_viewer_controller.add_data_model(data_model)

    def remove_data(self, source_info):
        data_model_to_remove = self.data_models_controller.get_model_using_source_info(source_info)
        self.drawers_controller.remove_drawer_using_data_model(data_model_to_remove)
        self.data_viewer_controller.remove_data_model(data_model_to_remove)
        self.data_models_controller.remove_data(data_model_to_remove)

    def remove_all_data(self):
        self.drawers_controller.remove_all_drawers()
        self.data_viewer_controller.remove_all_data()
        self.bird_eye_view.remove_data()
        self.data_models_controller.remove_all_data()

    def plot_velocity_profile(self):
        self.velocity_profile_plotter_controller.show(self.selection_controller.selected_drawer)

    def update_fix_view(self):
        if self.fix_view_controller.fix_view_state == True:
            current_plot_scale = self.bird_eye_view.viewRange() 
            dx = (current_plot_scale[0][1] - current_plot_scale[0][0]) * 0.5
            dy = (current_plot_scale[1][1] - current_plot_scale[1][0]) * 0.5
            self.bird_eye_view.setXRange(self.fix_view_controller.y - dx, self.fix_view_controller.y + dx,  padding=0)
            self.bird_eye_view.setYRange(self.fix_view_controller.x - dy, self.fix_view_controller.x + dy, padding=0)

    def populateColorPresets(self, ui):
        ui.colorPresetsMenu.clear()
        actions = []
        filenames = find("*.json", self.user_data_dir)
        action = QtWidgets.QAction("Save preset as ...", ui)
        action.triggered.connect(partial(self.save_state, ui))
        actions.append(action)
        action = QtWidgets.QAction("Load preset from ...", ui)
        action.triggered.connect(partial(self.load_state_from_file, ui))
        actions.append(action)
        for filename in filenames:
            action = QtWidgets.QAction(str(filename), ui)
            action.triggered.connect(partial(self.load_state, str(filename)))
            action.triggered.connect(self.drawers_controller.update_state)
            actions.append(action)
        ui.colorPresetsMenu.addActions(actions)

        return ui.colorPresetsMenu

    def load_state_from_file(self, ui):

        filename, _ = QtGui.QFileDialog.getOpenFileName(
            ui,
            "Open file ...",
            "",
            "JSON File (*.json)",
        )
        self.load_state(filename)
        self.drawers_controller.update_state()