from copy import deepcopy

import matplotlib.pyplot as plt
from extractors.Interfaces.ExtractedData import ExtractedData
from matplotlib.widgets import Slider

from AptivPerformanceEvaluation.Visualization.extract_scan_index import ScanIndexExtraction
from AptivPerformanceEvaluation.Visualization.scan_index_visualization import visualize_scan_index
from AptivPerformanceEvaluation.Visualization.utilities import add_azimuth_vcs, find_position_limits


class GulaGui:
    """
    GUI for visualization reference and estimated data as an input for performance evaluation
    Hint: you can change scan index by left and right arrow

                                        __.-._
                                        '-._"7'
                                         /'.-c
                                         |  /T
    In memory of Szymon Guła jjgr36     _)_/LI
    """

    def __init__(self, f_internal_objects=False, velocity_scale=0.3, f_axis_equal=True):
        """

        :param f_internal_objects: Flag indicating if internal or reduced object should be used for visualization
        :param velocity_scale: Scale of velocity vectors visualization, default 0.3
        """
        self.fig, ax = plt.subplots(2, 1, gridspec_kw={'height_ratios': [1, 19]}, num='Guła GUI')
        self.ax_main_view = ax[1]
        self.ax_scan_index = ax[0]
        self.velocity_scale = velocity_scale
        self.f_internal_objects = f_internal_objects
        self.f_axis_equal = f_axis_equal
        self.xlim = [-10, 10]
        self.ylim = [-10, 10]

        # Disable two keys - handling navigating bye arrows
        plt.rcParams['keymap.back'] = ''
        plt.rcParams['keymap.forward'] = ''

    def load_data(self, est_data: ExtractedData, ref_data: ExtractedData):
        """
        Load data into GUI, data should be synchronized by Scan Index

        :param est_data: estimated data
        :type est_data: ExtractedData
        :param ref_data: reference data
        :type ref_data: ExtractedData
        :return:
        """
        self.est_data = deepcopy(est_data)
        self.ref_data = deepcopy(ref_data)
        if est_data.detections is not None:
            add_azimuth_vcs(self.est_data)

        self._find_limits()
        self.ax_main_view.set_xlim(self.xlim)
        self.ax_main_view.set_ylim(self.ylim)

        self.min_scan_index = ScanIndexExtraction.get_min_scan_index(est_data)
        self.max_scan_index = ScanIndexExtraction.get_max_scan_index(est_data)
        self.scan_index = self.min_scan_index

        self.slider_scan_index = Slider(self.ax_scan_index, 'SI',
                                        self.min_scan_index, self.max_scan_index,
                                        valstep=1, valinit=self.scan_index)
        self.on_key_event = self.fig.canvas.mpl_connect('key_press_event', self._on_key)

        self._draw()
        self.slider_scan_index.on_changed(self._on_slider_update)

    def set_scan_index(self, value):
        self.slider_scan_index.set_val(value)

    def _find_limits(self):
        limits = find_position_limits(self.est_data.objects)
        # adjust ranges
        x_range = max(limits['ylim']) - min(limits['ylim'])
        y_range = max(limits['xlim']) - min(limits['xlim'])
        x_mid = (max(limits['ylim']) + min(limits['ylim'])) * 0.5
        y_mid = (max(limits['xlim']) + min(limits['xlim'])) * 0.5
        extend = max([x_range, y_range]) * 0.5 + 10.0

        self.xlim = [x_mid - extend, x_mid + extend]
        self.ylim = [y_mid - extend, y_mid + extend]

    def _draw(self):
        """
        Main drawing method
        :return:
        """
        # save previous limits
        self.xlim = self.ax_main_view.get_xlim()
        self.ylim = self.ax_main_view.get_ylim()

        visualize_scan_index(self.ax_main_view, self.est_data, self.ref_data, self.scan_index,
                             f_internal_objects=self.f_internal_objects, velocity_scale=self.velocity_scale)
        if self.f_axis_equal:
            self.ax_main_view.axis('equal')

        # set previous limits
        self.ax_main_view.set_xlim(self.xlim)
        self.ax_main_view.set_ylim(self.ylim)

    def _on_slider_update(self, val):
        """
        Action to be executed when slider is changed

        :param val: just magic to make slider working
        :return:
        """
        self.scan_index = int(val)
        self._draw()

    def _on_key(self, event):
        if event.key == 'right':
            value = self.slider_scan_index._value_in_bounds(self.scan_index + 1)
            self.slider_scan_index.set_val(value)
        elif event.key == 'left':
            value = self.slider_scan_index._value_in_bounds(self.scan_index - 1)
            self.slider_scan_index.set_val(value)
