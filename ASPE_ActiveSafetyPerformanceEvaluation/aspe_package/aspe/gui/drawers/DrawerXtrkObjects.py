
import numpy as np

from aspe.gui.drawers.DrawerBBoxes import DrawerBBoxes
from aspe.gui.drawers.DrawerCircle import DrawerCircle
from aspe.gui.drawers.DrawerInternalObjects import DrawerInternalObjects
from aspe.gui.drawers.DrawerPoints import DrawerPoints
from aspe.gui.drawers.DrawerVector import DrawerVector


class DrawerXtrkObjects(DrawerInternalObjects):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_drawers(self, plot_item):
        drawers = super().create_drawers(plot_item)
        if self._assoc_gates_can_be_drawed():
            assoc_gates_drawer = DrawerBBoxes(self,
                                              "AssocGatesMovable",
                                              plot_item,
                                              color="#008080",
                                              line_style="--",
                                              line_width=1)
            assoc_gates_non_mov_drawer = DrawerCircle(self,
                                                      "AssocGatesNonMovable",
                                                      plot_item,
                                                      color="#008080",
                                                      line_style="--",
                                                      line_width=1)
            predicted_position_drawer = DrawerPoints(self,
                                                     "PredictedPosition",
                                                     plot_item, symbol="x",
                                                     color="#ffff00",
                                                     symbol_size=8)
            predicted_velocity_drawer = DrawerVector(self,
                                                     "PredictedVelocity",
                                                     plot_item,
                                                     color="#ff8000",
                                                     line_style="-",
                                                     line_width=1)
            pseudo_position_drawer = DrawerPoints(self,
                                                  "PseudoMeasurement",
                                                  plot_item,
                                                  symbol="x",
                                                  color="#ff00ff",
                                                  symbol_size=8)

            assoc_gates_drawer.set_data(data_df=self.data_set.signals.loc[
                                                self.data_set.signals["f_moveable"] == True, :],
                                        center_x_signature="assoc_gates_center_x",
                                        center_y_signature="assoc_gates_center_y",
                                        orientation_signature="predicted_bounding_box_orientation",
                                        length_signature="assoc_gates_length",
                                        width_signature="assoc_gates_width")

            assoc_gates_non_mov_drawer.set_data(data_df=self.data_set.signals.loc[
                                                        self.data_set.signals["f_moveable"] == False, :],
                                                center_x_signature="assoc_gates_center_x",
                                                center_y_signature="assoc_gates_center_y",
                                                radius_signature="assoc_gates_radius",
                                                max_x_value=100,
                                                min_x_value=-100,
                                                line_points=20)

            predicted_position_drawer.set_data(self.data_set.signals,
                                               "predicted_position_x",
                                               "predicted_position_y")
            predicted_velocity_drawer.set_data(self.data_set.signals,
                                               "predicted_position_x",
                                               "predicted_position_y",
                                               "predicted_velocity_x",
                                               "predicted_velocity_y")
            pseudo_position_drawer.set_data(self.data_set.signals,
                                            "pseudo_position_x",
                                            "pseudo_position_y")

            drawers.append(assoc_gates_drawer)
            drawers.append(assoc_gates_non_mov_drawer)
            drawers.append(predicted_position_drawer)
            drawers.append(predicted_velocity_drawer)
            drawers.append(pseudo_position_drawer)
        return drawers

    def _assoc_gates_can_be_drawed(self):
        def _column_in_df_exists_and_is_not_full_nan(data_frame, column_name):
            try:
                return not np.all(np.isnan(data_frame.loc[:, column_name].to_numpy()))
            except KeyError:
                return False

        assoc_gates_signals = [
            "assoc_gates_center_x",
            "assoc_gates_center_y",
            "predicted_bounding_box_orientation",
            "assoc_gates_length",
            "assoc_gates_width",
        ]

        for signal in assoc_gates_signals:
            if not _column_in_df_exists_and_is_not_full_nan(self.data_set.signals, signal):
                return False
        return True
