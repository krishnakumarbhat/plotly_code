import unittest
from aspe.gui.controllers.FixViewController import FixViewController
from aspe.gui.controllers.MainController import MainController
import pandas as pd

class TestFixViewController(unittest.TestCase):
    def setUp(self):
        self.fix_view_controller = FixViewController()
        self.main_controller = MainController()

    def test_init(self):
        """ Check if variables are initialized properly """
        self.assertFalse(self.fix_view_controller.fix_view_state)
        self.assertEqual(self.fix_view_controller.x, 0)
        self.assertEqual(self.fix_view_controller.y, 0)

    def test_clear(self):
        self.fix_view_controller.fix_view_state = True
        self.fix_view_controller.clear()

        self.assertFalse(self.fix_view_controller.fix_view_state)
        self.fix_view_controller.clear()
        self.assertFalse(self.fix_view_controller.fix_view_state)

    def test_update_with_selected_drawer(self):
        """ Check if values are updated correctly with active selected object """
        class SelectedDrawer:
            def __init__(self):
                self.selected_data = pd.DataFrame.from_dict({"scan_index": [1], "center_x": [1.2], "center_y": [2.3]})
                self.current_scan_index = 1
        selected_drawer = SelectedDrawer()
        self.fix_view_controller.fix_view_state = True
        self.fix_view_controller.update(selected_drawer)
        self.assertEqual(self.fix_view_controller.x, 1.2)
        self.assertEqual(self.fix_view_controller.y, 2.3)
    
    def test_update_with_selected_drawer_not_obj(self):
        """ Check if values are not updated with active selected object which does not have required data """
        class SelectedDrawer:
            def __init__(self):
                self.selected_data = pd.DataFrame.from_dict({"scan_index": [1]})
                self.current_scan_index = 1
        selected_drawer = SelectedDrawer()
        self.fix_view_controller.fix_view_state = True
        self.fix_view_controller.update(selected_drawer)
        self.assertEqual(self.fix_view_controller.x, 0)
        self.assertEqual(self.fix_view_controller.y, 0)

    def test_update_without_selected_drawer(self):
        """ Check if values are updated correctly with active selected object """
        selected_drawer = None
        self.fix_view_controller.fix_view_state = True
        self.fix_view_controller.update(selected_drawer)
        self.assertEqual(self.fix_view_controller.x, 0)
        self.assertEqual(self.fix_view_controller.y, 0)


if __name__ == '__main__':
    unittest.main()