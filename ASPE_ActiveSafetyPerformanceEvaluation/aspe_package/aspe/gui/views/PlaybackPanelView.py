import numpy as np
from PyQt5 import QtCore
from PyQt5.QtWidgets import QMessageBox

from aspe.gui.views.uis.playback_panel_widget import Ui_PlaybackPanelWidget


class PlaybackPanelView(Ui_PlaybackPanelWidget):
    def __init__(self):
        super().__init__()
        self.controller = None  # move this init outside this class
        self.playback_factor_values = [(10 ** n) for n in np.linspace(-1, 0, 100)]
        self.setupUi()
        self.connect_signals()
        self.speed_playback_slider_value = 100
        self.scan_index_text_field.installEventFilter(self)

    @property
    def min_scan_index(self):
        return self.playback_slider.minimum()

    @min_scan_index.setter
    def min_scan_index(self, value):
        self.playback_slider.setMinimum(value)

    @property
    def max_scan_index(self):
        return self.playback_slider.maximum()

    @max_scan_index.setter
    def max_scan_index(self, value):
        self.playback_slider.setMaximum(value)

    def connect_signals(self):
        self.play_button.pressed.connect(self.on_play_button_click)
        self.reverse_button.pressed.connect(self.on_reverse_button_click)
        self.playback_speed_slider.valueChanged.connect(self.on_playback_speed_value_changed)
        self.playback_slider.valueChanged.connect(self.on_playback_slider_value_change)
        self.slow_button.pressed.connect(self.on_slow_button_click)
        self.next_button.pressed.connect(self.increment_playback_slider)
        self.record_gif_button.pressed.connect(self.on_record_gif_button_click)
        self.previous_button.pressed.connect(self.decrement_playback_slider)

    def on_play_button_click(self):
        self.controller.toggle_play()

    def on_playing_changed(self, is_playing):
        if is_playing:
            self.play_button.setText("Stop")
        else:
            self.play_button.setText("Play")

    def on_reverse_button_click(self):
        if self.reverse_button.isChecked():
            self.controller.un_reverse()
        else:
            self.controller.reverse()

    def on_reversed_changed(self, is_reversed):
        if is_reversed:
            self.reverse_button.setStyleSheet("font-weight: bold; color: blue")
        else:
            self.reverse_button.setStyleSheet("")

    def on_slow_button_click(self):
        self.controller.toggle_slow()

    def on_slow_changed(self, is_slow):
        if is_slow:
            self.slow_button.setStyleSheet("font-weight: bold; color: green")
            self.speed_playback_slider_value = self.playback_speed_slider.value()
            self.playback_speed_slider.setValue(1)
        else:
            self.slow_button.setStyleSheet("")
            self.playback_speed_slider.setValue(self.speed_playback_slider_value)

    def on_record_gif_button_click(self):
        self.controller.toggle_record()

    def on_record_gif_changed(self, is_recording):
        if is_recording:
            self.record_gif_button.setStyleSheet("font-weight: bold; color: red")
        else:
            self.record_gif_button.setStyleSheet("")

    def increment_playback_slider(self):
        next_value = self.playback_slider.value() + 1
        if next_value > self.max_scan_index:
            self.controller.stop()
        else:
            self.playback_slider.setValue(next_value)

    def decrement_playback_slider(self):
        next_value = self.playback_slider.value() - 1
        if next_value < self.min_scan_index:
            self.controller.stop()
        else:
            self.playback_slider.setValue(next_value)

    def on_playback_slider_value_change(self):
        scan_index = self.playback_slider.value()
        self.scan_index_text_field.setText(f"{scan_index}")
        self.controller.update_current_scan_index(scan_index)

    def on_playback_speed_value_changed(self):
        slider_value = self.playback_speed_slider.value()
        playback_factor = self.playback_factor_values[slider_value-1]
        self.playback_speed_value_text.setText(f"x{playback_factor:.2f}")
        self.controller.divide_timer_dt_by_factor(playback_factor)

    def on_scan_index_text_box_change(self):
        try:
            new_scan_index = int(self.scan_index_text_field.text())
            if self.min_scan_index < new_scan_index < self.max_scan_index:
                self.playback_slider.setValue(new_scan_index)
            else:
                self.scan_index_text_field.setText(str(self.playback_slider.value()))
                msg = QMessageBox()
                msg.setIcon(QMessageBox.Critical)
                msg.setText("Scan index error")
                msg.setInformativeText("Given scan index is not present in data")
                msg.setWindowTitle("Error")
                msg.exec_()
        except ValueError:
            self.scan_index_text_field.setText(str(self.playback_slider.value()))
            msg = QMessageBox()
            msg.setIcon(QMessageBox.Critical)
            msg.setText("Scan index error")
            msg.setInformativeText("Given scan index is not proper numeric format")
            msg.setWindowTitle("Error")
            msg.exec_()

    def eventFilter(self, obj, event):
        if event.type() == QtCore.QEvent.KeyPress and obj is self.scan_index_text_field and \
                event.key() == QtCore.Qt.Key_Return and self.scan_index_text_field.hasFocus():
            self.on_scan_index_text_box_change()
        return super().eventFilter(obj, event)
