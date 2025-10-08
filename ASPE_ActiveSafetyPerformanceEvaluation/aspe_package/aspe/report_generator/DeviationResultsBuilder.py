from aspe.report_generator.SingleDeviationResultsBuilder import SingleDeviationResultsBuilder
from aspe.report_generator.BaseBuilder import BaseBuilder
import numpy as np


class DeviationResultsBuilder(BaseBuilder):
    SECTION_NAME_MAP = {
        'dev_position_x': 'Position VCS X deviation',
        'dev_position_y': 'Position VCS Y deviation',
        'dev_speed': 'Speed deviation',
        'dev_orientation': 'Orientation deviation',
        'dev_velocity_x': 'Velocity VCS X deviation',
        'dev_velocity_y': 'Velocity VCS Y deviation',
        'dev_velocity_rel_x': 'Relative velocity VCS X deviation',
        'dev_velocity_rel_y': 'Relative velocity VCS Y deviation',
        'dev_acceleration_otg_x': 'Acceleration VCS X deviation',
        'dev_acceleration_otg_y': 'Acceleration VCS Y deviation',
        'dev_acceleration_rel_x': 'Relative acceleration VCS X deviation',
        'dev_acceleration_rel_y': 'Relative acceleration VCS Y deviation',
        'dev_bounding_box_dimensions_x': 'Longitudinal dimension deviation',
        'dev_bounding_box_dimensions_y': 'Lateral dimension deviation',
        'dev_yaw_rate': 'Yaw rate deviation'
    }
    UNIT_NAME_MAP = {
        'dev_position_x': '[m]',
        'dev_position_y': '[m]',
        'dev_speed': '[m/s]',
        'dev_orientation': '[deg]',
        'dev_velocity_x': '[m/s]',
        'dev_velocity_y': '[m/s]',
        'dev_velocity_rel_x': '[m/s]',
        'dev_velocity_rel_y': '[m/s]',
        'dev_bounding_box_dimensions_x': '[m/s]',
        'dev_bounding_box_dimensions_y': '[m/s]',
        'dev_yaw_rate': '[deg/s]',
        'dev_acceleration_otg_x': '[m/s^2]',
        'dev_acceleration_otg_y': '[m/s^2]',
        'dev_acceleration_rel_x': '[m/s^2]',
        'dev_acceleration_rel_y': '[m/s^2]',
    }

    def __init__(self, data, template_path, plotter):
        super().__init__(template_path, plotter)
        self.data = data
        self.build_results()

    def build_results(self):
        features_signatures = self.data['kpis_pairs_features_aggregated']['feature_signature'].unique()
        features_signatures = [f for f in features_signatures if 'nees' not in f]
        sections_html = []
        for feature_sign in features_signatures:
            section_name = self._get_section_name(feature_sign)
            unit = self._get_unit_signature(feature_sign)
            if not np.all(np.isnan(self.data['pe_results_obj_pairs'].loc[:, feature_sign].to_numpy())):
                single_section_builder = SingleDeviationResultsBuilder(self.data, feature_sign, section_name, unit,
                                                                       self.template_name, self.plotter)
                sections_html.append(single_section_builder.html_out)
        self.html_out = ''.join(sections_html)[:-1]

    def _get_section_name(self, feature_signature):
        try:
            section_name = self.SECTION_NAME_MAP[feature_signature]
        except KeyError:
            section_name = feature_signature
        return section_name

    def _get_unit_signature(self, feature_signature):
        try:
            unit = self.UNIT_NAME_MAP[feature_signature]
        except KeyError:
            unit = '[n/a]'
        return unit


if __name__ == "__main__":
    import pickle
    in_path = r"C:\wkspaces\ASPE0000_00_Common\.private\ASPE_reports\ASPE_reporting_input.pickle"
    with open(in_path, 'rb') as f:
        data = pickle.load(f)
    drb = DeviationResultsBuilder(data)
