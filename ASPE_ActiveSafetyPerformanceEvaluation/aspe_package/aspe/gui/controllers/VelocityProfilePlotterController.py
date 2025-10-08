import numpy as np
import pandas as pd

from aspe.gui.drawers.DrawerInternalObjects import DrawerInternalObjects
from aspe.gui.drawers.DrawerObjects import DrawerObjects
from aspe.gui.utilities.calc_bbox_corners import calc_bbox_corners
from aspe.gui.views.VelocityProfilePlotterWidget import VelocityProfilePlotterWidget
from aspe.utilities.MathFunctions import normalize_angle_vector


class VelocityProfilePlotterController:
    def __init__(self):
        self.velocity_profile_plotter_widget = None
        self.active_selected_drawer = None

    def show(self, selected_drawer):
        if self.drawer_suitable(selected_drawer):
            if self.velocity_profile_plotter_widget is None:
                self.velocity_profile_plotter_widget = VelocityProfilePlotterWidget(self)
                self.velocity_profile_plotter_widget.show()
                self.velocity_profile_plotter_widget.create_new_plot()

            self.active_selected_drawer = selected_drawer
            self.plot_velocity_profile()

    def update(self, selected_drawer):
        if self.drawer_suitable(selected_drawer) and self.velocity_profile_plotter_widget is not None:
            self.active_selected_drawer = selected_drawer
            self.plot_velocity_profile()

    def clear(self):
        if self.velocity_profile_plotter_widget is not None:
            self.velocity_profile_plotter_widget.clear()

    def on_signal_plotter_widget_close(self):
        self.velocity_profile_plotter_widget = None

    def drawer_suitable(self, selected_drawer):
        return selected_drawer is not None and selected_drawer.selected_data is not None and \
               (isinstance(selected_drawer, DrawerInternalObjects) | isinstance(selected_drawer, DrawerObjects))

    def plot_velocity_profile(self):
        extracted_data = self.active_selected_drawer.get_parent().data_model.extracted
        selected_data = self.active_selected_drawer.selected_data
        scan_index = self.active_selected_drawer.current_scan_index

        obj_df = selected_data[selected_data.scan_index == scan_index]

        dets = extracted_data.detections.signals.join(extracted_data.detections.raw_signals, rsuffix="_raw")

        # Handling missing f_selected_b_trk signal
        if "f_selected_by_trk" not in dets:
            f_selected_by_trk = pd.Series(np.zeros(dets.shape[0]), name="f_selected_by_trk")
            dets = dets.join(f_selected_by_trk)

        if obj_df.shape[0] == 1:
            obj = obj_df.iloc[0]

            # Detection filters
            scan_index_filter = dets.scan_index == scan_index
            associated_object_filter = dets.assigned_obj_id == selected_data.id.iloc[0]
            position_filter = self.get_position_filter(dets, obj_df)
            all_relevant_dets = dets.loc[scan_index_filter & (associated_object_filter | position_filter), :]

            associated_dets, not_associated_dets, selected_dets, all_relevant_dets = self.detection_calculation(
                all_relevant_dets, obj, selected_data)

            # Calculate predicted range rate based on object velocity for azimuth angles around existing detections
            azimuth_vec, velocity_based_range_rate, _ = self.get_estimated_velocity_profile(obj=obj,
                                                                                            dets=all_relevant_dets)

            self.velocity_profile_plotter_widget.plot(associated_dets=associated_dets,
                                                      not_associated_dets=not_associated_dets,
                                                      selected_dets=selected_dets,
                                                      est_azimuth_vec=azimuth_vec,
                                                      est_range_rate_vec=velocity_based_range_rate,
                                                      est_range_rate_vec_refs=None,
                                                      )
        else:
            self.velocity_profile_plotter_widget.clear()

    def get_estimated_velocity_profile(self, obj, dets=None, obj_ref=None, name=None):
        # Set the velocity rr for reference data as None
        velocity_based_range_rate_refs = None

        # Azimuth vector boundaries
        if dets is not None and dets.shape[0] != 0:
            azimuth_margin = np.deg2rad(10.0)
            lower_azimuth_lim = np.min(dets["azimuth_vcs_norm"]) - azimuth_margin
            upper_azimuth_lim = np.max(dets["azimuth_vcs_norm"]) + azimuth_margin
        else:
            azimuth_margin = np.deg2rad(30.0)
            object_angular_position = np.arctan2(obj.center_y,
                                                 obj.center_x)
            lower_azimuth_lim = object_angular_position - azimuth_margin
            upper_azimuth_lim = object_angular_position + azimuth_margin

        # Calculate azimuth vector
        azimuth_vec = np.linspace(lower_azimuth_lim, upper_azimuth_lim, 100)
        velocity_based_range_rate = self.estimate_rr_from_velocity(azimuth_vec, obj)

        if name is not None and name == "Reference":
            velocity_based_range_rate_refs = self.estimate_rr_from_velocity(azimuth_vec, obj)

        # Calculate velocity rr if reference for estimated object exist
        if obj_ref is not None and not obj_ref.empty:
            velocity_based_range_rate_refs = self.estimate_rr_from_velocity(azimuth_vec, obj_ref)
        elif name is not None and not "Estimated":
            velocity_based_range_rate = None

        return azimuth_vec, velocity_based_range_rate, velocity_based_range_rate_refs

    def detection_calculation(self, all_relevant_dets, obj, selected_data):
        # Calculate normalized azimuth angle for filtered detection subset
        azimuth_vcs_norm = normalize_angle_vector(all_relevant_dets["azimuth_vcs"] % (2.0 * np.pi))
        all_relevant_dets = all_relevant_dets.assign(azimuth_vcs_norm=azimuth_vcs_norm)

        # Calculate range rate error based on object velocity
        estimated_range_rate = self.estimate_rr_from_velocity_curvature_compensated(all_relevant_dets, obj)
        est_range_rate_error = np.abs(all_relevant_dets["range_rate_comp"] - estimated_range_rate)
        all_relevant_dets = all_relevant_dets.assign(est_range_rate_error=est_range_rate_error)

        # Divide relevant detections into subsets for plotting
        associated_filter = all_relevant_dets.assigned_obj_id == selected_data.id.iloc[0]
        associated_dets = all_relevant_dets[associated_filter]
        not_associated_dets = all_relevant_dets[~associated_filter]
        selected_dets = associated_dets[associated_dets.f_selected_by_trk == 1]

        return associated_dets, not_associated_dets, selected_dets, all_relevant_dets

    @staticmethod
    def get_position_filter(dets, obj_df, position_margin=2.0):
        corners_x, corners_y = calc_bbox_corners(obj_df.center_x.to_numpy(),
                                                 obj_df.center_y.to_numpy(),
                                                 obj_df.bounding_box_orientation.to_numpy(),
                                                 obj_df.bounding_box_dimensions_x.to_numpy(),
                                                 obj_df.bounding_box_dimensions_y.to_numpy())
        return (dets.position_x > np.nanmin(corners_x) - position_margin) & \
            (dets.position_x < np.nanmax(corners_x) + position_margin) & \
            (dets.position_y > np.nanmin(corners_y) - position_margin) & \
            (dets.position_y < np.nanmax(corners_y) + position_margin)

    @staticmethod
    def estimate_rr_from_velocity(azimuth_vec, obj):
        current_vel_otg_x = obj.velocity_otg_x
        current_vel_otg_y = obj.velocity_otg_y
        return current_vel_otg_x * np.cos(azimuth_vec) + current_vel_otg_y * np.sin(azimuth_vec)

    @staticmethod
    def estimate_rr_from_velocity_curvature_compensated(dets, obj):
        # Handling missing curvature signal
        if "curvature" not in obj:
            curvature = pd.Series(np.zeros(1), name="curvature", index=["curvature"])
            obj = pd.concat([obj, curvature])

        # Handling missing vcs heading
        if "vcs_heading" not in obj:
            vcs_heading = pd.Series(np.arctan2(obj.velocity_otg_y, obj.velocity_otg_x),
                                    name="vcs_heading", index=["vcs_heading"])
            obj = pd.concat([obj, vcs_heading])

        dx = dets.position_x - obj.position_x
        dy = dets.position_y - obj.position_y

        return obj.speed * (((np.cos(obj.vcs_heading) - (obj.curvature * dy)) * np.cos(dets.azimuth_vcs)) +
                            ((np.sin(obj.vcs_heading) - (obj.curvature * dx)) * np.sin(dets.azimuth_vcs)))
