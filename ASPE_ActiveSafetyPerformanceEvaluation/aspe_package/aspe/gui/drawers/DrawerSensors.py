from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerSensorFoV import DrawerSensorFoV


class DrawerSensors(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return DrawerPoints(self, "RadarSelection", plot_item, symbol="o", color="#ff0000", symbol_size=12)

    def create_drawers(self, plot_item):
        radar_points_drawer = DrawerPoints(
            self, "Sensors positions", plot_item, symbol="o", color="#ffff00", symbol_size=8)
        signals = self.data_set.signals.drop(columns=["new_measurement_update"])

        per_look_exists = hasattr(self.data_set, 'per_look')
        per_look_empty = per_look_exists and self.data_set.per_look.empty

        if not per_look_exists or per_look_empty:
            self.data_df = signals.join(self.data_set.per_sensor.set_index(["sensor_id"]), on=["sensor_id"])
        else:
            sensor_properties = self.data_set.per_look.join(self.data_set.per_sensor.set_index("sensor_id"),
                                                            on="sensor_id")
            self.data_df = signals.join(sensor_properties.set_index(["sensor_id", "look_id"]),
                                        on=["sensor_id", "look_id"])

        radar_points_drawer.set_data(self.data_df, "position_x", "position_y")
        sensorList = [radar_points_drawer]

        if not per_look_empty: # prevent drawing FoV when per look isn't extracted
            for sensor_id, sensor_df in self.data_df.groupby(by="sensor_id"):
                sensorObject = DrawerSensorFoV(
                    self, f"Sensor {sensor_id} {sensor_df.loc[sensor_df.index[0], 'mount_location']}",
                    plot_item, color="#66ffff14")
                sensorObject.set_data(sensor_df)
                sensorList.append(sensorObject)

        return sensorList

    def select(self, df_index):
        selected_row = self.data_df.loc[df_index, :]
        if selected_row.unique_id != self.selected_unique_id:
            self.selected_unique_id = selected_row.unique_id
            selected_df = self.data_df.loc[self.data_df.loc[:, "unique_id"] == self.selected_unique_id, :]
            if self.data_set.raw_signals is not None:
                raw_df = self.data_set.raw_signals.loc[selected_df.index, :]
                selected_df = selected_df.join(raw_df, rsuffix="_raw")
            self.selected_data = selected_df
            self.selection_drawer.set_data(self.selected_data, "position_x", "position_y")
            self.selection_drawer.plot_scan_index(self.current_scan_index)
            self.parent.on_select(self)
        else:  #casewhenclickonsamedetection
            self.deselect()

