import sys

from PyQt5 import QtCore
from PyQt5.QtWidgets import QApplication

from aspe.gui.views.AspeGuiMainWindowView import AspeGuiMainWindowView


def run_aspe_gui():
    QApplication.setAttribute(QtCore.Qt.HighDpiScaleFactorRoundingPolicy.PassThrough)
    app = QApplication(sys.argv)
    ui = AspeGuiMainWindowView()
    ui.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    run_aspe_gui()
