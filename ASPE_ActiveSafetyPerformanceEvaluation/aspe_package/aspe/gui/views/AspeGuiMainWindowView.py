import sys

from PyQt5 import QtCore
from PyQt5.QtWidgets import QApplication
from pyqtgraph import exporters

from aspe.gui.controllers.DataViewerController import DataViewerController
from aspe.gui.controllers.DrawersController import DrawersController
from aspe.gui.controllers.LoadDataPanelController import LoadDataPanelController
from aspe.gui.controllers.MainController import MainController
from aspe.gui.controllers.PlaybackController import PlaybackController
from aspe.gui.controllers.SelectionController import SelectionController
from aspe.gui.drawers.DrawersFactory import DrawersFactory
from aspe.gui.utilities.GifCreator import GifCreator
from aspe.gui.views.BirdsEyeView import BirdsEyeView
from aspe.gui.views.DataViewerWidget import DataViewerWidget
from aspe.gui.views.LoadDataPanelView import LoadDataPanelView
from aspe.gui.views.PlaybackPanelView import PlaybackPanelView
from aspe.gui.views.SelectionViewerWidget import SelectionViewerWidget
from aspe.gui.views.settings.DrawersSettingsWidget import DrawersSettingsWidget
from aspe.gui.views.uis.aspe_gui_main_window import Ui_AspeGuiMainWindow


class AspeGuiMainWindowView(Ui_AspeGuiMainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi()

        self.main_controller = MainController()
        self.colorPresetsMenu.aboutToShow.connect(self.connect_populateColorPresets)
        self.playback_panel = PlaybackPanelView()
        self.load_data_panel = LoadDataPanelView()
        self.birds_eye_view = BirdsEyeView()
        DrawersFactory.birds_eye_view = self.birds_eye_view
        self.birds_eye_view.main_controller = self.main_controller
        self.main_controller.bird_eye_view = self.birds_eye_view

        self.drawers_controller = DrawersController()
        self.drawers_controller.playback_panel = self.playback_panel

        self.playback_controller = PlaybackController()
        self.playback_controller.playback_panel_view = self.playback_panel
        self.playback_controller.main_controller = self.main_controller
        exporter = exporters.ImageExporter(DrawersFactory.birds_eye_view.plotItem)
        self.playback_controller.gif_recorder = GifCreator(exporter)
        self.playback_controller.gif_recorder.drawers_controller = self.drawers_controller
        self.playback_panel.controller = self.playback_controller

        self.load_data_panel_controller = LoadDataPanelController(self.load_data_panel)
        self.load_data_panel.load_data_panel_controller = self.load_data_panel_controller
        self.load_data_panel_controller.drawers_controller = self.drawers_controller
        self.load_data_panel_controller.main_controller = self.main_controller

        self.control_panel_layout.addWidget(self.load_data_panel)
        self.control_panel_layout.addWidget(self.playback_panel)
        self.bird_view_layout.addWidget(self.birds_eye_view)

        self.selection_viewer = SelectionViewerWidget("Selection table view", self.right_half_widget)
        self.drawers_settings_widget = DrawersSettingsWidget("Drawers settings", self.right_half_widget)
        self.data_viewer_widget = DataViewerWidget("DataViewer", self.right_half_widget)

        self.drawers_controller.drawers_setting_widget = self.drawers_settings_widget

        self.scroll_area_layout.addWidget(self.data_viewer_widget)
        self.scroll_area_layout.addWidget(self.drawers_settings_widget)
        self.scroll_area_layout.addWidget(self.selection_viewer)
        self.scroll_area_layout.addStretch()

        self.drawers_controller.main_controller = self.main_controller

        self.data_viewer_controller = DataViewerController(self.data_viewer_widget)

        self.selection_controller = SelectionController()
        self.selection_controller.selection_viewer = self.selection_viewer
        self.selection_viewer.controller = self.selection_controller
        self.selection_controller.main_controller = self.main_controller

        self.main_controller.drawers_controller = self.drawers_controller
        self.main_controller.playback_controller = self.playback_controller
        self.main_controller.selection_controller = self.selection_controller
        self.main_controller.data_viewer_controller = self.data_viewer_controller
        self.main_controller.load_state()

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Space:
            self.playback_controller.toggle_play()
        elif event.key() == QtCore.Qt.Key_R:
            self.playback_controller.toggle_reverse()
        elif event.key() == QtCore.Qt.Key_S:
            self.playback_controller.toggle_slow()
        elif event.key() == QtCore.Qt.Key_G:
            self.playback_controller.toggle_record()
        elif event.key() == QtCore.Qt.Key_D:
            self.playback_panel.increment_playback_slider()
        elif event.key() == QtCore.Qt.Key_A:
            self.playback_panel.decrement_playback_slider()
        event.accept()

    def closeEvent(self, event):
        pass

    def connect_populateColorPresets(self):
        self.main_controller.populateColorPresets(ui=self)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = AspeGuiMainWindowView()
    ui.show()
    sys.exit(app.exec_())
