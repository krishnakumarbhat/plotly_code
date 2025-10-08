from LogSets.events.event_classes.overtaking import Overtaking
from LogSets.utils.file_handling import load
from typing import List
import pandas as pd


class EventsCalculator:
    def __init__(self, extracted_data, components):
        """
        :param extracted_data: extracted object's signals
        :param components: data frame with already calculated components
        """
        self.extracted_log = extracted_data
        self.events = components
        self.available_events = dict()
        self._init_available_events()

    def calculate_all_available_events(self) -> pd.DataFrame:
        for event in self.available_events.values():
            try:
                event.calculate()
            except:
                print(f'Cant calculate: {str(event)}')
        return self.events

    def calculate_events(self, events: List[str]) -> pd.DataFrame:
        for event in events:
            if event in self.available_events.keys():
                try:
                    self.available_events[event].calculate()
                except:
                    print(f'Cant calculate: {str(event)}')
            else:
                print(f'Event: "{event}" is not a valid event name. Available events are: '
                      f'{" , ".join(list(self.available_events.keys()))} ')
        return self.events

    def _init_available_events(self):
        #   If you create new event ( based on IBaseEvent) it has to be added here to self.available_events dictionary
        self.available_events['overtaking'] = Overtaking(self.extracted_log, self.events)


def example():
    extracted_log_path = r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\Utils\example_extracted_logs' \
                         r'\example_extracted_log.pickle'

    extracted_log = load(extracted_log_path)['data']['objects']['signals']
    components = pd.DataFrame()
    components['path'] = pd.Series(extracted_log_path)
    events = EventsCalculator(extracted_log, components)

    # components = events.calculate_all_available_events()
    components = events.calculate_events(['overtaking', 'not_an_event'])
    print()


if __name__ == '__main__':
    example()