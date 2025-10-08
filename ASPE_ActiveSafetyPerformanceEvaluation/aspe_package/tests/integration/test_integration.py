import sys
import os
import unittest
import pytest
from pathlib import Path
from PyQt5.QtWidgets import QApplication
from PyQt5 import QtCore

from aspe.extractors import F360MudpExtractedData
from aspe.extractors.ENV.DataSets.ENVExtractedData import ENVExtractedData
from aspe.gui.drawers.EnvDrawer import EnvDrawer
from aspe.gui.drawers.F360MudpDrawer import F360MudpDrawer
from aspe.gui.views.AspeGuiMainWindowView import AspeGuiMainWindowView
from aspe.gui.drawers.F360XtrkDrawer import F360XtrkDrawer
from aspe.extractors.F360.DataSets.F360XtrkExtractedData import F360XtrkExtractedData


class IntegrationTest(unittest.TestCase):
    """Integration tests for ASPE."""
    @classmethod
    def setup_class(self):
        # Get dir path
        self.dir_path = os.getenv("ASPE_TEST_LOG_PATH")

    def setup_method(self, method):
        # Create a QApplication instance
        QApplication.setAttribute(QtCore.Qt.HighDpiScaleFactorRoundingPolicy.PassThrough)
        self.app = QApplication(sys.argv)

        # Create an instance of your main application window
        self.window = AspeGuiMainWindowView()
        self.window.setAttribute(QtCore.Qt.WA_DontShowOnScreen, True)
        self.window.show()

    def teardown_method(self, method):
        # Clean up the main window and QApplication
        self.window.close()
        self.app.quit()
        del self.window
        del self.app
        
        
        # Clean up extracted .pickle files
        extracted_path = Path(self.dir_path) / 'logs'
        extracted = extracted_path.glob('*.pickle')

        for f_name in extracted:
            if Path.is_file(f_name):
                Path.unlink(f_name)

    def test_gui_with_mudp(self):
        test_file = str(Path(self.dir_path) / 'logs' / 'sample_mudp.mudp')
        
        self._load_data(test_file)

        self._check_drawer(drawer_type=F360MudpDrawer,
                           extracted_type=F360MudpExtractedData)

    def test_gui_with_xtrk(self):
        test_file = str(Path(self.dir_path) / 'logs' / 'sample_xtrk.xtrk')
        self._load_data(test_file)

        self._check_drawer(drawer_type=F360XtrkDrawer,
                           extracted_type=F360XtrkExtractedData)
        
    def test_gui_with_ocg_keg(self):
        test_file = str(Path(self.dir_path) / 'logs' / 'sample_ocg.keg')
        self._load_data(test_file)

        self._check_drawer(drawer_type=EnvDrawer,
                           extracted_type=ENVExtractedData)

    def _load_data(self, test_file):
        # load text to data panel combo box
        self.window.load_data_panel.log_path_combo_box.setCurrentText(test_file)
        self.window.load_data_panel.on_log_path_changed()
        
        self.assertTrue(self.window.load_data_panel.current_log_path, 'Current log path not set.')

        # trigger loading data 
        self.window.load_data_panel.data_sources_table.selectRow(0)
        self.window.load_data_panel.data_sources_table.on_load_clicked()

    def _check_drawer(self, drawer_type, extracted_type):
        # check if drawers list is not empty, check if relevant drawer exists
        drawers_controller = self.window.main_controller.drawers_controller
        self.assertTrue(drawers_controller.drawers, 'Drawers list empty.')

        drawer = drawers_controller.drawers[0]
        self.assertIsInstance(drawer, drawer_type, 'Drawer is not an instance of {}'.format(drawer_type))

        # check if drawer has extracted data
        self.assertTrue(drawer.data_model is not None, 'Data model not present.')
        self.assertIsInstance(drawer.data_model.extracted, extracted_type, 'Extracted data is not an instance of {}'.format(extracted_type))


if __name__ == '__main__':
    pytest.main()
