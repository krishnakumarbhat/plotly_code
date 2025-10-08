class FixViewController():
    """Class that controls fix viewport functionality.

    Selected drawer position x and y are extracted and updated with
    scan index change to set current plot x and y ranges. If the user
    selects the option from the context menu the camera will "track" 
    selected object.

    Attributes:
        selected_drawer: Dataframe containing properties of selected object
    """

    def __init__(self):
        self.fix_view_state = False
        self.x, self.y = 0, 0

    def update(self, selected_drawer):
        if (self.fix_view_state and selected_drawer):
            try:
                self.x, self.y = self.__get_selected_drawer_coordinates(selected_drawer)
            except KeyError:
                print('Selected item is not an object, fix viewport is turned off')
                self.clear()

    def clear(self):
        if self.fix_view_state:
            self.toggle_fix_view_state()

    def toggle_fix_view_state(self):
        self.fix_view_state ^= 1

    def __get_selected_drawer_coordinates(self, selected_drawer):
        selected_data = selected_drawer.selected_data
        scan_index = selected_drawer.current_scan_index
        obj_df = selected_data[selected_data.scan_index == scan_index]
        x = obj_df['center_x'].values[0] 
        y = obj_df['center_y'].values[0]
        return x, y
