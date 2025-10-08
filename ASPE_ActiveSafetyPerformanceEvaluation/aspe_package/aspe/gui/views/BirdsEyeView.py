from pyqtgraph import PlotWidget    # noqa: I001
from pyqtgraph.Qt import QtGui

class BirdsEyeView(PlotWidget):
    def __init__(self):
        super().__init__()
        self.disableAutoRange()
        self.showGrid(x=True, y=True, alpha=0.5)
        self.scene().sigMouseClicked.connect(self.on_sigMouseReleased)
        self.scene().setClickRadius(0.5)
        self.setXRange(-50, 50, padding=0)
        self.setYRange(-50, 50, padding=0)
        self.resizeEvent = self.resize_event
        # Replace default context menu in ViewBox
        self.plotItem.vb.menu = None
        self.set_custom_context_menu()
        self.main_controller = None
        
    def set_custom_context_menu(self):
        if self.plotItem.vb.menu is None:
            # velocity profile
            self.plotItem.vb.menu = QtGui.QMenu()
            self.plotItem.vb.velocityProfile = QtGui.QAction("Velocity profile", self.plotItem.vb.menu)
            self.plotItem.vb.velocityProfile.triggered.connect(self.plot_velocity_profile)
            self.plotItem.vb.menu.addAction(self.plotItem.vb.velocityProfile)

            # reset viewport
            self.plotItem.vb.resetGui = QtGui.QAction("Reset viewport", self.plotItem.vb.menu)
            self.plotItem.vb.resetGui.triggered.connect(self.reset_viewport)
            self.plotItem.vb.menu.addAction(self.plotItem.vb.resetGui)
            
            # fix viewport on object
            self.plotItem.vb.fixView = QtGui.QAction("Fix viewport", self.plotItem.vb.menu)
            self.plotItem.vb.fixView.triggered.connect(self.fix_view)
            self.plotItem.vb.menu.addAction(self.plotItem.vb.fixView)

    def plot_velocity_profile(self):
        self.main_controller.plot_velocity_profile()

    def reset_viewport(self):
        """ Resets axes ranges on the plotting canvas """
        self.setXRange(-50, 50, padding=0)
        self.setYRange(-10, 50, padding=0)
        self.resizeEvent = self.resize_event
        
    def fix_view(self):
        """ Fixes viewport on selected object """
        self.main_controller.fix_view_controller.toggle_fix_view_state()
        
    def resize_event(self, ev):
        """ Handle fixed ratio of bird eye view """
        super().resizeEvent(ev)  # call all stuff which is normally done while resizing
        width = self.size().width()
        height = self.size().height()

        size_ratio = width / height

        x_min, x_max = self.getAxis("bottom").range
        x_len = x_max - x_min

        expected_y_len = x_len / size_ratio
        y_min, y_max = self.getAxis("left").range
        y_center = y_min + (y_max - y_min)/2
        new_y_min, new_y_max = y_center - expected_y_len / 2, y_center + expected_y_len / 2
        self.setYRange(new_y_min, new_y_max)

    def on_sigMouseReleased(self, mouse_click_event):
        # trick for handling clicking events, LinesDrawer and PointsDrawer classes
        if hasattr(mouse_click_event.currentItem, "drawer"):
            mouse_click_event.currentItem.drawer.on_click(mouse_click_event)

    def remove_data(self):
        self.plotItem.vb.AllViews.data = {}
        self.plotItem.subMenus.clear()
        self.plotItem.clear()
        self.plotItem.clearPlots()
