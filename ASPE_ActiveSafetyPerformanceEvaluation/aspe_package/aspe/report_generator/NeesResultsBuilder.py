from aspe.report_generator.BaseBuilder import BaseBuilder
from aspe.report_generator.SingleNeesResultsBuilder import SingleNeesResultsBuilder


class NeesResultsBuilder(BaseBuilder):
    SECTION_NAME_MAP = {
        'nees_value_position_x': 'Position VCS X NEES value',
        'nees_value_position_y': 'Position VCS Y NEES value',
        'nees_value_position_xy': 'Position VCS XY NEES value',
        'nees_value_velocity_x': 'Velocity VCS X NEES value',
        'nees_value_velocity_y': 'Velocity VCS Y NEES value',
        'nees_value_velocity_xy': 'Velocity VCS XY NEES value',
    }

    DIMENSIONS_MAP = {
        'nees_value_position_x': 1,
        'nees_value_position_y': 1,
        'nees_value_position_xy': 2,
        'nees_value_velocity_x': 1,
        'nees_value_velocity_y': 1,
        'nees_value_velocity_xy': 2,
    }

    def __init__(self, data, template_path, plotter):
        super().__init__(template_path, plotter)
        self.data = data
        self.build_results()

    def build_results(self):
        features_signatures = self.SECTION_NAME_MAP.keys()
        sections_html = []
        for feature_sign in features_signatures:
            section_name = self.SECTION_NAME_MAP[feature_sign]
            dimensions = self.DIMENSIONS_MAP[feature_sign]

            single_section_builder = SingleNeesResultsBuilder(self.data, self.plotter, self.template_name,
                                                              section_name, feature_sign, dimensions)
            sections_html.append(single_section_builder.html_out)
        self.html_out = ''.join(sections_html)[:-1]
