from typing import List, Tuple
from warnings import warn

from psac.interface import PSACInterface
from psac.items import Event, EventName
from tqdm import tqdm

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluationConfig import Nexus50kEvaluationConfig
from aspe.utilities.nexus50k_events_finder.user_config import user_config
from aspe.utilities.nexus50k_events_finder.utils import get_events_list_from_csv, get_psac_interface


def upload_events_from_csv_to_nexus(config: Nexus50kEvaluationConfig, event_name_to_create: str,
                                    event_description: str, events_csv_path: str) -> Tuple[EventName, List[Event]]:
    """
    Upload events which are stored in .csv file to Nexus. Csv file is produced by
    BaseNexusEventFinder.save_events_to_csv method, however it can be specified manually.

    :param config: config which is used to connect to Nexus. Only config.NEXUS_USER_NAME attribute needs to be set
    :param event_name_to_create: event name, which will be associated to uploaded events. Should be interpreted as type
    of events. If event_name already exists in Nexus, new events will be simply added to existing events' set
    :param event_description: event description which will be associated to uploaded events type. If events_name already
    exists this parameter is not used.
    :param events_csv_path: path to csv file containing info about found events. Csv file is produced by
    BaseNexusEventFinder.save_events_to_csv method, but it can be also specified manually. Required columns:
        - name: str - name of event
        - nexus_log_id: str - Nexus log id for log where event occurs
        - tmin: str - start timestamp of log, given in form YYYY-mm-dd HH:MM:SS.ff
        - tmax: str - end timestamp of log, given in form YYYY-mm-dd HH:MM:SS.ff
    :return: tuple: (created EventName record, list of Event records uploaded to Nexus)
    """

    pi = get_psac_interface(config)
    ev_name = pi.createEventName(
        name=event_name_to_create,
        description=event_description,
        return_existing=True,
    )[0]

    uploaded_events = []
    for event in tqdm(get_events_list_from_csv(events_csv_path)):
        new_event = pi.createEvent(
            event_name=ev_name.id,
            log=event.nexus_log_id,
            start=event.tmin.isoformat() + 'Z',
            end=event.tmax.isoformat() + 'Z',
            emitter='manual',
            return_existing=True)
        uploaded_events.append(new_event[0])
    return ev_name, uploaded_events


def download_events_from_nexus(event_name_list: str = None, event_id: str = None, pi: PSACInterface = None)\
        -> List[Event]:
    if not (event_id or event_name_list):
        raise AttributeError("One of arguments: event_name_list or event_id needs to be specified.")

    if pi is None:
        pi = get_psac_interface()

    if event_name_list is not None:
        event_name_list = pi.getEventNameList(name=event_name_list)
        if len(event_name_list) > 0:
            events_list = pi.getEventList(query={'name': event_name_list[0].id})
        else:
            warn(f'Event name {event_name_list} was not found in Nexus. Returned events list will be empty.')
            events_list = []
    else:
        events_list = pi.getEventList(event_id=event_id)
    return events_list


if __name__ == '__main__':
    # Example usage of upload_events_from_csv_to_nexus function

    name = 'f360_close_trucks'
    description = 'Auto-gt objects classified as tracks reported in close range to host'
    csv_path = r"E:\logfiles\nexus_50k_eval_data\close_trucks_events.csv"

    event_name, uploaded_events = upload_events_from_csv_to_nexus(
        config=user_config,
        event_name_to_create=name,
        event_description=description,
        events_csv_path=csv_path,
    )

    events_list = download_events_from_nexus(config=user_config, event_name_list=name)