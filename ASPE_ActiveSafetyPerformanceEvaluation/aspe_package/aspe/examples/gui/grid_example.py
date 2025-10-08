import random

import numpy as np
import pyqtgraph as pg
from PyQt5.QtWidgets import QGraphicsPathItem
from pyqtgraph.Qt import QtCore, QtGui, QtWidgets
from tqdm import tqdm

from aspe.utilities.SupportingFunctions import add_alpha_to_rgb

COLOR_PALETTE = [
    '#1f77b4',
    '#ff7f0e',
    '#5FD35F',
    '#d62728',
    '#9467bd',
    '#bcbd22',
    '#7f7f7f',
    '#17becf',
    '#8c564b',
    '#008000',
    '#800000',
    '#FF00FF',
    '#FFFF00',
    '#0000FF',
]


class OccupancyGridGUI:
    def __init__(self):
        # set default grid parameters
        self.x_offset = 0
        self.y_offset = 0
        self.x_tick = 2
        self.y_tick = 2
        self.nb_cells_x = 2
        self.nb_cells_y = 8
        self.factor = 1
        self.curvature = 1
        self.angle = 0

    def setup_ui(self):
        # initialize Qt application
        self.app = QtWidgets.QApplication([])

        # define a top-level widget to hold everything
        self.window = QtWidgets.QWidget()
        self.window.setWindowTitle('Grid example')

        # create widgets to be placed inside window
        self.plot = pg.PlotWidget()
        self.plot.setAspectLocked()
        self.plot.showGrid(x=True, y=True)

        self.in_x_tick = QtWidgets.QLineEdit()
        self.in_x_tick.setValidator(QtGui.QIntValidator())
        self.in_x_tick.setMaxLength(2)
        self.in_x_tick.setText(str(self.x_tick))
        self.in_x_tick.setAlignment(QtCore.Qt.AlignRight)
        label_x_tick = QtWidgets.QLabel('y tick')

        self.in_y_tick = QtWidgets.QLineEdit()
        self.in_y_tick.setValidator(QtGui.QIntValidator())
        self.in_y_tick.setMaxLength(2)
        self.in_y_tick.setText(str(self.y_tick))
        self.in_y_tick.setAlignment(QtCore.Qt.AlignRight)
        label_y_tick = QtWidgets.QLabel('x tick')

        self.in_nb_cells_x = QtWidgets.QLineEdit()
        self.in_nb_cells_x.setValidator(QtGui.QIntValidator())
        self.in_nb_cells_x.setMaxLength(2)
        self.in_nb_cells_x.setText(str(self.nb_cells_x))
        self.in_nb_cells_x.setAlignment(QtCore.Qt.AlignRight)
        label_nb_cells_x = QtWidgets.QLabel('Number of y cells')

        self.in_nb_cells_y = QtWidgets.QLineEdit()
        self.in_nb_cells_y.setValidator(QtGui.QIntValidator())
        self.in_nb_cells_y.setMaxLength(2)
        self.in_nb_cells_y.setText(str(self.nb_cells_y))
        self.in_nb_cells_y.setAlignment(QtCore.Qt.AlignRight)
        label_nb_cells_y = QtWidgets.QLabel('Number of x cells')

        self.in_x_offset = QtWidgets.QLineEdit()
        self.in_x_offset.setValidator(QtGui.QIntValidator())
        self.in_x_offset.setMaxLength(2)
        self.in_x_offset.setText(str(self.x_offset))
        self.in_x_offset.setAlignment(QtCore.Qt.AlignRight)
        label_x_offset = QtWidgets.QLabel('y offset')

        self.in_y_offset = QtWidgets.QLineEdit()
        self.in_y_offset.setValidator(QtGui.QIntValidator())
        self.in_y_offset.setMaxLength(2)
        self.in_y_offset.setText(str(self.y_offset))
        self.in_y_offset.setAlignment(QtCore.Qt.AlignRight)
        label_y_offset = QtWidgets.QLabel('x offset')

        self.in_scale_factor = QtWidgets.QLineEdit()
        validator = QtGui.QDoubleValidator(-10, 10, 1)
        # set dot instead of comma separation
        validator.setLocale(QtCore.QLocale(QtCore.QLocale.English, QtCore.QLocale.UnitedStates))
        self.in_scale_factor.setValidator(validator)
        self.in_scale_factor.setMaxLength(3)
        self.in_scale_factor.setText(str(self.factor))
        self.in_scale_factor.setAlignment(QtCore.Qt.AlignRight)
        label_scale_factor = QtWidgets.QLabel('Scale factor')

        self.in_curvature = QtWidgets.QLineEdit()
        validator = QtGui.QDoubleValidator(-0.1, 0.001, 3)
        # set dot instead of comma separation
        validator.setLocale(QtCore.QLocale(QtCore.QLocale.English, QtCore.QLocale.UnitedStates))
        self.in_curvature.setValidator(validator)
        self.in_curvature.setMaxLength(5)
        self.in_curvature.setText(str(self.curvature))
        self.in_curvature.setAlignment(QtCore.Qt.AlignRight)
        label_curvature = QtWidgets.QLabel('Curvature (1/radius) (i.e. 0.01)')

        self.in_angle = QtWidgets.QLineEdit()
        self.in_angle.setValidator(QtGui.QIntValidator())
        self.in_angle.setMaxLength(3)
        self.in_angle.setText(str(self.angle))
        self.in_angle.setAlignment(QtCore.Qt.AlignRight)
        label_angle = QtWidgets.QLabel('Rotation angle [deg]')

        btn = QtWidgets.QPushButton('Generate grid.')
        btn.clicked.connect(self.print_grid)

        # create a grid layout to manage the widgets size and position
        layout = QtWidgets.QGridLayout()
        self.window.setLayout(layout)

        # add widgets to the layout
        pos_left_row = 0
        layout.addWidget(label_x_tick, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_x_tick, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_y_tick, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_y_tick, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_nb_cells_x, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_nb_cells_x, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_nb_cells_y, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_nb_cells_y, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_x_offset, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_x_offset, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_y_offset, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_y_offset, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_scale_factor, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_scale_factor, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_curvature, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_curvature, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(label_angle, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.in_angle, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(btn, pos_left_row, 0)
        pos_left_row += 1
        layout.addWidget(self.plot, 0, 1, pos_left_row, 1)

    def get_input_values(self):
        if self.in_x_offset.text():
            self.x_offset = int(self.in_x_offset.text())

        if self.in_y_offset.text():
            self.y_offset = int(self.in_y_offset.text())

        if self.in_x_tick.text():
            self.x_tick = int(self.in_x_tick.text())

        if self.in_y_tick.text():
            self.y_tick = int(self.in_y_tick.text())

        if self.in_nb_cells_x.text():
            self.nb_cells_x = int(self.in_nb_cells_x.text())

        if self.in_nb_cells_y.text():
            self.nb_cells_y = int(self.in_nb_cells_y.text())

        if self.in_scale_factor.text():
            self.factor = float(self.in_scale_factor.text())

        if self.in_curvature.text():
            self.curvature = float(self.in_curvature.text())

        if self.in_angle.text():
            self.angle = np.deg2rad(int(self.in_angle.text()))

    def print_grid(self):
        self.get_input_values()

        cs_origin = np.array([0, 0])
        x = np.array([cs_origin[0], self.x_tick, self.x_tick, cs_origin[0], cs_origin[0], np.nan])
        y = np.array([cs_origin[1], cs_origin[1], self.y_tick, self.y_tick, cs_origin[1], np.nan])

        x_result, y_result, state, confidence = self.create_polygon_path_coordinates(x, y, self.nb_cells_x,
                                                                                     self.nb_cells_y,
                                                                                     self.x_tick,
                                                                                     self.y_tick)

        x_result = self.change_cs_origin(x_result, self.x_tick, self.nb_cells_x)
        x_result, y_result = self.scale_grid_by_factor(x_result, y_result, self.factor, self.nb_cells_y, cs_origin)
        x_result = self.curvilinear_transformation(x_result, y_result, self.curvature)
        x_result, y_result = self.rotation(x_result, y_result, self.angle)
        x_result, y_result, cs_origin = self.translate_cords_by_offset(x_result, y_result, self.x_offset, self.y_offset,
                                                                       cs_origin)

        x_split = np.split(x_result, x_result.size / 6)
        y_split = np.split(y_result, y_result.size / 6)

        cords = zip(x_split, y_split)

        # clear plot before adding new plot items
        self.plot.clear()

        self.add_drawers(cords, state, confidence)

    def add_drawers(self, cords, state, confidence):
        for iterator, cell_cords in enumerate(tqdm(cords)):
            cell_list = list(cell_cords)
            x_to_draw = cell_list[0]
            y_to_draw = cell_list[1]
            path = pg.arrayToQPath(x_to_draw, y_to_draw)

            color = state[iterator]
            alpha = f'{confidence[iterator]:x}'

            color_to_set = add_alpha_to_rgb(color=color, alpha=alpha)

            graphic_object = QGraphicsPathItem()
            graphic_object.setPen(pg.mkPen(color=color_to_set))
            graphic_object.setBrush(pg.mkBrush(color=color_to_set))

            graphic_object.setPath(path)
            self.plot.addItem(graphic_object)

    @staticmethod
    def create_polygon_path_coordinates(x, y, nb_cells_x, nb_cells_y, x_tick, y_tick):
        x_result = np.array([])
        y_result = np.array([])
        state = np.array([])
        confidence = np.array([], dtype=int)
        x_temp = x
        y_temp = y
        for i in tqdm(range(0, nb_cells_y)):
            x_result = np.append(x_result, x_temp)
            y_result = np.append(y_result, y_temp)
            # adds random state and confidence color representation
            state = np.append(state, COLOR_PALETTE[random.randint(0, len(COLOR_PALETTE) - 1)])
            confidence = np.append(confidence, random.randint(16, 255))
            for j in range(1, nb_cells_x):
                x_temp = x_temp + x_tick
                x_result = np.append(x_result, x_temp)
                y_result = np.append(y_result, y_temp)
                state = np.append(state, COLOR_PALETTE[random.randint(0, len(COLOR_PALETTE) - 1)])
                confidence = np.append(confidence, random.randint(16, 255))
            x_temp = x
            y_temp = y_temp + y_tick

        return x_result, y_result, state, confidence

    @staticmethod
    def translate_cords_by_offset(x_result, y_result, x_off, y_off, cs_origin):
        x_result = x_result + x_off
        y_result = y_result + y_off
        cs_origin[0] = cs_origin[0] + x_off
        cs_origin[1] = cs_origin[1] + y_off

        return x_result, y_result, cs_origin

    @staticmethod
    def change_cs_origin(x_result, x_tick, nb_cells_x):
        offset = nb_cells_x * x_tick / 2
        x_result = x_result - offset

        return x_result

    @staticmethod
    def scale_grid_by_factor(x, y, factor, nb_cells_y, cs_origin):
        def create_scale_factor_vector(factor, nb_cells):
            diff = factor - 1
            factor_vector = np.array([])
            factor_step = diff / (nb_cells)
            if diff != 0:
                to_append = 1
                for i in range(0, nb_cells + 1):
                    factor_vector = np.append(factor_vector, to_append)
                    to_append += factor_step
            return factor_vector

        factor_vector = create_scale_factor_vector(factor, nb_cells_y)
        paired = np.concatenate((x.reshape(1, -1), y.reshape(1, -1)), axis=0)

        diff_x = np.array([])
        diff_y = np.array([])
        for j in range(0, paired.shape[1]):
            diff_x = np.append(diff_x, paired[0][j] - cs_origin[0])
            diff_y = np.append(diff_y, paired[1][j] - cs_origin[1])

        # get unique values and drop nans
        diffs_y = np.unique(diff_y)[:-1]

        if factor_vector.size == 0:
            return x, y

        for i in range(0, len(diffs_y)):
            x[diff_y == diffs_y[i]] *= factor_vector[i]
        return x, y

    @staticmethod
    def curvilinear_transformation(x, y, curvature):
        if curvature != 0:
            radius = 1 / curvature
            y_points = np.unique(y)[:-1]

            # transformed circle equation, (x - x0)^2 + (y - y0)^2 = R^2, where x0 = R and y0 = 0
            # after transformation equation is given in form: x = sqrt(r^2 - y^2) + r, but only when r > y
            # when the radius is positive: x = -sqrt(r^2 - y^2) + r
            # when the radius is negative: x = sqrt(r^2 - y^2) + r

            print('radius: ', radius, 'max y: ', np.max(y_points))
            if abs(radius) > np.max(y_points):
                if radius > 0:
                    x_circle = -np.sqrt(radius ** 2 - y_points ** 2) + radius
                elif radius < 0:
                    x_circle = np.sqrt(radius ** 2 - y_points ** 2) + radius
                for i in range(0, len(x_circle)):
                    x[y == y_points[i]] += x_circle[i]

        return x

    @staticmethod
    def rotation(x, y, angle):
        paired = np.concatenate((x.reshape(1, -1), y.reshape(1, -1)), axis=0)
        size = paired.shape[1]
        pos_mat = paired.reshape(2, size, 1).transpose(1, 2, 0)

        rot_mat = np.tile(np.array([[np.cos(angle), -np.sin(angle)],
                                    [np.sin(angle), np.cos(angle)]]).reshape(2, 2, 1).transpose(2, 1, 0), (size, 1, 1))

        result = pos_mat @ rot_mat

        split = np.vsplit(result.transpose(2, 0, 1).reshape(2, size), 2)

        x_result = split[0].reshape(-1)
        y_result = split[1].reshape(-1)

        return x_result, y_result

    def run_app(self):

        # setup user interface
        self.setup_ui()

        # display widget as a new window
        self.window.show()

        # run app in a loop
        self.app.instance().exec_()


if __name__ == '__main__':
    gui = OccupancyGridGUI()
    gui.run_app()
