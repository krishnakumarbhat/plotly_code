from datetime import datetime
from typing import List

import pandas as pd
from psac.interface import PSACInterface

from aspe.evaluation.RadarObjectsEvaluation.Nexus50kEvaluation import NexusEvent
from aspe.utilities.nexus50k_events_finder.user_config import user_config


def save_event_list_to_csv(events_list: List[NexusEvent], csv_path: str):
    df = pd.DataFrame([ev.to_dict() for ev in events_list])
    df.to_csv(csv_path)


def get_events_list_from_csv(csv_p: str, only_n_first_events: int = None) -> List[NexusEvent]:
    df = pd.read_csv(csv_p, index_col=0)
    event_list = []
    for index, row in df.iterrows():
        if only_n_first_events and index > only_n_first_events:
            break
        event = NexusEvent(name=row['name'],
                           nexus_log_id=row['nexus_log_id'],
                           tmin=datetime.fromisoformat(row['tmin']),
                           tmax=datetime.fromisoformat(row['tmax']),
                           index=index)
        event_list.append(event)
    return event_list


def get_psac_interface():
    return PSACInterface(url="https://nexus.aptiv.com/api",
                         user=user_config.NEXUS_USER_NAME,
                         pass_word=user_config.NEXUS_PASSWORD)
