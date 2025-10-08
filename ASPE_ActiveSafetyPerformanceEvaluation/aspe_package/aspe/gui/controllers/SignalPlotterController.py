from PyQt5.Qt import QAction

from aspe.gui.views.SignalPlotterWidget import SignalPlotterWidget


class SignalPlotterController:
    def __init__(self):
        self.signal_plotter_widget = None
        self.axes_count = 0
        self.current_scan_index = None

    def update_current_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if self.signal_plotter_widget is not None:
            self.signal_plotter_widget.update_signal_plotter(scan_index)

    def add_new_plot(self, source, data_set_name, selected_data, signature_to_plot):
        if self.signal_plotter_widget is None:
            self.signal_plotter_widget = SignalPlotterWidget(self)
            self.signal_plotter_widget.show()
        self.signal_plotter_widget.create_new_plot()
        self.axes_count = len(self.signal_plotter_widget.axes)
        self.signal_plotter_widget.plot_signal(source,
                                               data_set_name,
                                               selected_data,
                                               signature_to_plot,
                                               self.axes_count-1)
        self.signal_plotter_widget.update_signal_plotter(self.current_scan_index)

    def plot_on_existing_axes(self, axes_index, source, selected_data, data_set_name, signature_to_plot):
        self.signal_plotter_widget.plot_signal(source, data_set_name, selected_data, signature_to_plot, axes_index)
        self.signal_plotter_widget.update_signal_plotter(self.current_scan_index)

    def on_signal_plotter_widget_close(self):
        self.signal_plotter_widget = None
        self.axes_count = 0

    def on_selection_viewer_context_menu_event(self, event, menu, selection_viewer):
        if self.signal_plotter_widget is not None:
            self.axes_count = len(self.signal_plotter_widget.axes)
        signal_plotter_menu = menu.addMenu("SignalPlotter")
        new_plot_action = signal_plotter_menu.addAction("New plot")
        add_to_plot_actions = []
        for n in range(self.axes_count):
            add_to_plot_action = QAction(f"Plot on axes {n+1}")
            signal_plotter_menu.addAction(add_to_plot_action)
            add_to_plot_actions.append(add_to_plot_action)

        selected_signals = selection_viewer.get_selected_signals_signatures()
        action = menu.exec_(selection_viewer.mapToGlobal(event.pos()))
        selected_drawer = selection_viewer.currently_selected
        # TODO FZD-378: drawer should has method 'get_data_model' or smth, this is nasty
        data_root_folder = selected_drawer.parent.data_model.source_info.root_folder
        data_set_name = selected_drawer.name
        if action == new_plot_action:
            for signal_signature in selected_signals:
                self.add_new_plot(data_root_folder, data_set_name, selected_drawer.selected_data, signal_signature)
        else:
            for ax_idx, add_to_plot_action in enumerate(add_to_plot_actions):
                if action == add_to_plot_action:
                    for signal_signature in selected_signals:
                        self.plot_on_existing_axes(ax_idx,
                                                   data_root_folder,
                                                   selected_drawer.selected_data,
                                                   data_set_name,
                                                   signal_signature)

    def on_new_plot_clicked(self):
        selected_rows = self.selection_table.selectionModel().selectedRows()
        selected_rows_idxs = [r.row() for r in selected_rows]
        for row_idx in selected_rows_idxs:
            signal_signature = self.selection_table.item(row_idx, 0).text()
            selected_df = self.currently_selected.selected_data
            self.signal_plotter_controller.add_new_plot(None, selected_df, signal_signature)