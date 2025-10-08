import numpy as np
import pandas as pd

from aspe.gui.drawers.DrawerLines import DrawerLines


class DrawerContour(DrawerLines):
    def set_data(self, data_df: pd.DataFrame, vector_origin_x_signature: str, vector_origin_y_signature: str) -> None:
        data_df["df_indexes"] = data_df.index
        x_data = data_df[vector_origin_x_signature].to_numpy().astype(np.float32)
        y_data = data_df[vector_origin_y_signature].to_numpy().astype(np.float32)
        df_indexes = data_df["df_indexes"].to_numpy()
        scan_index = data_df["scan_index"].to_numpy()
        contour_changed = data_df["contour_id"].to_numpy().astype(np.int32)

        insert_indexes = np.where(contour_changed[:-1] != contour_changed[1:])[0] + 1
        x_data = np.insert(x_data, insert_indexes, np.nan)
        y_data = np.insert(y_data, insert_indexes, np.nan)
        df_indexes = np.insert(df_indexes, insert_indexes - 1, df_indexes[insert_indexes - 1])
        scan_index = np.insert(scan_index, insert_indexes, scan_index[insert_indexes])
        connect_mask = ~np.isnan(x_data)
        df = pd.DataFrame({"scan_index": scan_index,
                           "x_data": x_data,
                           "y_data": y_data,
                           "df_indexes": df_indexes,
                           "connect_mask": connect_mask})

        df_grouped = df.groupby(by="scan_index")
        self.x_data = df_grouped["x_data"].apply(np.array).to_dict()
        self.y_data = df_grouped["y_data"].apply(np.array).to_dict()
        self.is_visible = df_grouped["connect_mask"].apply(np.array).to_dict()
        self.df_indexes = df_grouped["df_indexes"].apply(np.array).to_dict()

    def append_legend_part(self, legend_df: pd.DataFrame, legend_dict: dict) -> pd.DataFrame:
        if self.is_enabled:
            legend_dict["subdrawer_name"] = [self.name]
            legend_dict["color"] = [self.color]
            legend_dict["style"] = [self.line_style]
            legend_df = legend_df.append(pd.DataFrame(legend_dict))
        return legend_df
