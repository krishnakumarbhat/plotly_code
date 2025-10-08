
from aspe.evaluation.RadarObjectsEvaluation.BinaryClassification import BCType
from aspe.gui.drawers.abstract_drawers.DrawerComposite import DrawerComposite
from aspe.gui.drawers.DrawerPoints import DrawerPoints


class DrawerBinaryClassification(DrawerComposite):
    def __init__(self, parent, extracted_data_set, plot_item, name):
        super().__init__(parent, extracted_data_set, plot_item, name)

    def create_selection_drawer(self, plot_item):
        return None

    def create_drawers(self, plot_item):
        true_positives_drawer = DrawerPoints(self, "TruePositives", plot_item, symbol="o", color="#00ff80",
                                             symbol_size=8)
        false_positives_drawer = DrawerPoints(self, "FalsePositives", plot_item, symbol="o", color="#ff0000",
                                              symbol_size=8)
        false_negatives_drawer = DrawerPoints(self, "FalseNegatives", plot_item, symbol="o", color="#ffff00",
                                              symbol_size=8)
        over_segmentation_drawer = DrawerPoints(self, "OverSegmentation", plot_item, symbol="o", color="#ffA500",
                                                symbol_size=8)
        under_segmentation_drawer = DrawerPoints(self, "UnderSegmentation", plot_item, symbol="o", color="#0000ff",
                                                 symbol_size=8)

        true_positives_drawer.set_data(
            self.data_set.loc[self.data_set.binary_classification == BCType.TruePositive, :],
            "center_x", "center_y")
        false_positives_drawer.set_data(
            self.data_set.loc[self.data_set.binary_classification == BCType.FalsePositive, :],
            "center_x", "center_y")
        false_negatives_drawer.set_data(
            self.data_set.loc[self.data_set.binary_classification == BCType.FalseNegative, :],
            "center_x", "center_y")
        over_segmentation_drawer.set_data(
            self.data_set.loc[self.data_set.binary_classification == BCType.OverSegmentation, :],
            "center_x", "center_y")
        under_segmentation_drawer.set_data \
            (self.data_set.loc[self.data_set.binary_classification == BCType.UnderSegmentation, :],
             "center_x", "center_y")

        return [true_positives_drawer,
                false_positives_drawer,
                false_negatives_drawer,
                over_segmentation_drawer,
                under_segmentation_drawer]

    def select(self, df_index):
        pass

