from PyQt5.QtCore import QTimer

from aspe.utilities.SupportingFunctions import create_log_path


class PlaybackController:
    def __init__(self):
        self.min_timer_dt = 15  # [ms]
        self.current_timer_dt = self.min_timer_dt
        self.main_controller = None
        self.playback_panel_view = None
        self.current_scan_index = None
        self.is_playing = False
        self.is_slowed = False
        self.is_reversed = False
        self.is_recording = False
        self.scan_index_values = None
        self.timer = QTimer()
        self.timer.timeout.connect(self.on_timer_timeout)
        self.gif_recorder = None

    def play(self):
        self.is_playing = True
        self.timer.start(self.current_timer_dt)
        self.playback_panel_view.on_playing_changed(self.is_playing)

    def stop(self):
        self.is_playing = False
        self.timer.stop()
        self.playback_panel_view.on_playing_changed(self.is_playing)

    def update_current_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        self.main_controller.update_current_scan_index(scan_index)
        if self.is_recording:
            self.gif_recorder.save_current_scan_index(scan_index, self.main_controller.bird_eye_view.size())

    def on_timer_timeout(self):
        if self.is_reversed:
            self.playback_panel_view.decrement_playback_slider()
        else:
            self.playback_panel_view.increment_playback_slider()

    def divide_timer_dt_by_factor(self, factor):
        new_dt = self.min_timer_dt / factor
        self.current_timer_dt = new_dt
        self.timer.setInterval(new_dt)

    def slow(self):
        self.is_slowed = True
        self.timer.setInterval(150)
        self.playback_panel_view.on_slow_changed(self.is_slowed)

    def un_slow(self):
        self.is_slowed = False
        self.timer.setInterval(self.current_timer_dt)
        self.playback_panel_view.on_slow_changed(self.is_slowed)

    def update_scan_index_values(self, scan_index_values):
        self.scan_index_values = scan_index_values
        min_scan_index = scan_index_values.min()
        max_scan_index = scan_index_values.max()
        self.playback_panel_view.min_scan_index = min_scan_index
        self.playback_panel_view.max_scan_index = max_scan_index

    def reverse(self):
        self.is_reversed = True
        self.playback_panel_view.on_reversed_changed(self.is_reversed)

    def un_reverse(self):
        self.is_reversed = False
        self.playback_panel_view.on_reversed_changed(self.is_reversed)

    def record_gif(self):
        self.is_recording = True
        self.playback_panel_view.on_record_gif_changed(self.is_recording)

    def un_record_gif(self):
        self.is_recording = False
        self.stop()
        last_dir = self.main_controller.data_models_controller.data_models[0].source_info.log_file_path
        log_path = create_log_path(last_dir)
        self.gif_recorder.create_gif(log_path)
        self.playback_panel_view.on_record_gif_changed(self.is_recording)

    def toggle_play(self):
        if self.is_playing:
            self.stop()
        else:
            self.play()

    def toggle_reverse(self):
        if not self.is_reversed:
            self.reverse()
        else:
            self.un_reverse()

    def toggle_slow(self):
        if not self.is_slowed:
            self.slow()
        else:
            self.un_slow()

    def toggle_record(self):
        if not self.is_recording:
            self.record_gif()
        else:
            self.un_record_gif()
