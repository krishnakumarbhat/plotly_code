import numpy as np
import pandas as pd

from aspe.gui.controllers.VelocityProfilePlotterController import VelocityProfilePlotterController


class VelocityProfilePlotterControllerPE(VelocityProfilePlotterController):
    def __init__(self):
        super().__init__()

    def plot_velocity_profile(self):
        extracted_data = self.active_selected_drawer.get_parent().data_model.extracted
        selected_data = self.active_selected_drawer.selected_data
        scan_index = self.active_selected_drawer.current_scan_index
        name = self.active_selected_drawer.name

        obj_df = selected_data[selected_data.scan_index == scan_index]

        obj_ref = extracted_data.extracted_reference_objects.signals
        obj_est = extracted_data.extracted_estimated_objects.signals
        dets = extracted_data.extracted_radar_detections.signals
        pairs = extracted_data.pe_results_obj_pairs.signals

        # Handling missing f_selected_b_trk signal
        if "f_selected_by_trk" not in dets:
            f_selected_by_trk = pd.Series(np.zeros(dets.shape[0]), name="f_selected_by_trk")
            dets = dets.join(f_selected_by_trk)

        # There should always be only one active selected object
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
            if name == "Reference":
                index_ref = selected_data.loc[selected_data.scan_index == scan_index, :].index[0]
                index_est = pairs.sort_values(by="distance").loc[pairs.index_ref == index_ref, "index_est"]
                if index_est.size != 0:
                    index_est = index_est.iloc[0]
                all_relevant_est = obj_est.loc[index_est, :]

                if not all_relevant_est.empty:
                    azimuth_vec, velocity_based_range_rate, velocity_based_range_rate_refs = \
                        self.get_estimated_velocity_profile(
                            obj=all_relevant_est,
                            obj_ref=obj,
                            name=name)
                else:
                    azimuth_vec, velocity_based_range_rate, velocity_based_range_rate_refs = \
                        self.get_estimated_velocity_profile(
                            obj=obj,
                            name=name)
            elif name == "Estimated":
                index_est = selected_data.loc[selected_data.scan_index == scan_index, :].index[0]
                index_ref = pairs.sort_values(by="distance").loc[pairs.index_est == index_est, "index_ref"]
                if index_ref.size != 0:
                    index_ref = index_ref.iloc[0]
                all_relevant_refs = obj_ref.loc[index_ref, :]

                azimuth_vec, velocity_based_range_rate, velocity_based_range_rate_refs = \
                    self.get_estimated_velocity_profile(obj, all_relevant_dets, all_relevant_refs)

            self.velocity_profile_plotter_widget.plot(associated_dets=associated_dets,
                                                      not_associated_dets=not_associated_dets,
                                                      selected_dets=selected_dets,
                                                      est_azimuth_vec=azimuth_vec,
                                                      est_range_rate_vec=velocity_based_range_rate,
                                                      est_range_rate_vec_refs=velocity_based_range_rate_refs,
                                                      )
        else:
            self.velocity_profile_plotter_widget.clear()
