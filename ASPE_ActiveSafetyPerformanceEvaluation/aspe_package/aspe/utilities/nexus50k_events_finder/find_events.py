from aspe.utilities.nexus50k_events_finder.event_finders.close_pedestrians_event_finder import (
    ClosePedestriansEventFinder,
)
from aspe.utilities.nexus50k_events_finder.user_config import user_config

if __name__ == '__main__':
    event_finder = ClosePedestriansEventFinder(config=user_config)
    event_finder.run()
    event_finder.save_events_to_csv()
