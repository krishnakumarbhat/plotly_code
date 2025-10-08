class SelectionController:
    def __init__(self):
        self.main_controller = None
        self.is_selected = False
        self.current_scan_index = None
        self.selected_drawer = None
        self.selection_viewer = None
        self.selected_data = None

    def update_current_scan_index(self, scan_index):
        self.current_scan_index = scan_index
        if self.is_selected:
            self.selected_drawer.current_scan_index = self.current_scan_index
            self.update_selection_table()

    def on_select(self, selected_drawer):
        if selected_drawer is not self.selected_drawer:
            if self.selected_drawer is not None:
                self.selected_drawer.deselect()
            self.is_selected = True
            self.selected_drawer = selected_drawer
        self.selected_data = selected_drawer.selected_data
        self.update_selection_table()

    def update_selection_table(self):
        selected_index = \
            self.selected_data.loc[self.selected_data.loc[:, "scan_index"] == self.current_scan_index, :].index
        if len(selected_index) > 0:
            selected_row = self.selected_data.loc[selected_index[0], :].sort_index()
            self.selection_viewer.update_selection(self.selected_drawer.parent, self.selected_drawer, selected_row)
        else:
            self.on_deselect()

    def on_selection_viewer_context_menu_event(self, event, menu, selection_viewer):
        self.main_controller.signal_plotter_controller.on_selection_viewer_context_menu_event(event,
                                                                                              menu,
                                                                                              selection_viewer)

    def on_deselect(self):
        self.is_selected = False
        self.selected_drawer = None
        self.selection_viewer.on_deselect()
