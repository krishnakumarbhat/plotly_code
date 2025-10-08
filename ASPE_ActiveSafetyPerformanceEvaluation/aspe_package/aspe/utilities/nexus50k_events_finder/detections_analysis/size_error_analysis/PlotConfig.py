class PlotConfig:
    def __init__(self, number_of_resims):
        self.number_of_resims = number_of_resims
        self.plot_functions = {}
        self.y_axis_units = 'm'


class OvertakingConfig(PlotConfig):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.event_name = 'Overtaking'
        self.csv_base_name = 'overtaking'
        self.y_axis_value = 'center_x'


class CrossTrafficConfig(PlotConfig):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.event_name = 'Crosstraffic'
        self.csv_base_name = 'cta'
        self.y_axis_value = 'center_y'


class StopAndGoConfig(PlotConfig):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.event_name = 'Stop & Go'
        self.csv_base_name = 'stop_and_go'
        self.y_axis_value = 'range'