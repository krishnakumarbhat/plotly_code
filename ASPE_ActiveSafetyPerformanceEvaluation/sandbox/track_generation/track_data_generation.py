'''
Created by jj81t8
'''

import pickle
import pandas as pd
import os
from random import uniform

# choice: 'pkl', 'data'
def tracksDataMaker(path_RT_pickle,choice):
    RT_data = pd.read_pickle(path_RT_pickle)
    scan_keys = pd.DataFrame.keys(RT_data['right_sens_rt'])

    drive, path_and_file = os.path.splitdrive(path_RT_pickle)
    path, file = os.path.split(path_and_file)
    allname, extension = os.path.splitext(file)
    name = allname[0:-6]+'TrackData'
    path_to_save = path + "\\" + '{}.pkl'.format(name)

    list_scan_idx = []
    for scan_idx in scan_keys:
        list_scan_idx.append(scan_idx)

    list_tracks = ['track1', 'track2']

    Track_data = {'right_sens_rt': {el: {el2: {'TrackAngleHeading': 0,
                                                        'TrackVelForward': 0,
                                                        'TrackVelLateral': 0,
                                                        'RangePosLateral': 0,
                                                        'RangePosForward': 0,
                                                        'RangeVelLateral': 0,
                                                        'RangeVelForward': 0,
                                                        'RangeTrackVehicleLength': 0,
                                                        'RangeTrackVehicleWidth': 0,
                                                        'front left corner': {'x': 0, 'y': 0},
                                                        'front right corner': {'x': 0, 'y': 0},
                                                        'rear left corner': {'x': 0, 'y': 0},
                                                        'rear right corner': {'x': 0, 'y': 0}
                                            }for el2 in list_tracks
                                      }for el in list_scan_idx}}

    for scan_idx in list_scan_idx:
        i = 1
        for track in list_tracks:
            pos_offset = round(uniform(0.1, 0.5), 4)
            vel_offset = round(uniform(0.1, 0.5), 4)
            heading_offset = round(uniform(0.1, 0.5), 4)

            Track_data['right_sens_rt'][scan_idx][track]['TrackAngleHeading'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['TargetAngleHeading'] + heading_offset
            Track_data['right_sens_rt'][scan_idx][track]['TrackVelForward'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['TargetVelForward'] + vel_offset
            Track_data['right_sens_rt'][scan_idx][track]['TrackVelLateral'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['TargetVelLateral'] + vel_offset
            Track_data['right_sens_rt'][scan_idx][track]['RangePosLateral'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangePosLateral'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['RangePosForward'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangePosForward'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['RangeVelLateral'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangeVelLateral'] + vel_offset
            Track_data['right_sens_rt'][scan_idx][track]['RangeVelForward'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangeVelForward'] + vel_offset
            Track_data['right_sens_rt'][scan_idx][track]['RangeTrackVehicleLength'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangeTargetVehicleLength']
            Track_data['right_sens_rt'][scan_idx][track]['RangeTrackVehicleWidth'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['RangeTargetVehicleWidth']


            Track_data['right_sens_rt'][scan_idx][track]['front left corner']['x'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['front left corner']['x'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['front left corner']['y'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['front left corner']['y'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['front right corner']['x'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['front right corner']['x'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['front right corner']['y'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['front right corner']['y'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['rear left corner']['x'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['rear left corner']['x'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['rear left corner']['y'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['rear left corner']['y'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['rear right corner']['x'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['rear right corner']['x'] + pos_offset
            Track_data['right_sens_rt'][scan_idx][track]['rear right corner']['y'] = RT_data['right_sens_rt'][scan_idx]['target{}'.format(i)]['rear right corner']['y'] + pos_offset
            i += 1
    if choice == 'pkl':
        with open(path_to_save, 'wb') as handle:
            pickle.dump(Track_data, handle, protocol=pickle.HIGHEST_PROTOCOL)
        return path_to_save
    elif choice == 'data':
        return Track_data


if __name__ =='__main__':
    path_dict = r'C:\Users\jj81t8\Desktop\Wizualizacja\Multi_target'
    list_of_files = os.listdir(path_dict)
    for file in list_of_files:
        if file[-10:] == 'RTData.pkl':
            path_RT_pickle = path_dict + '\\' + file
            choice = 'pkl'
            drive, path_and_file = os.path.splitdrive(path_RT_pickle)
            path, file = os.path.split(path_and_file)
            allname, extension = os.path.splitext(file)
            name = allname[0:-6] + 'TrackData.pkl'
            if choice == 'pkl':
                if name not in list_of_files:
                    path_Track_pickle = tracksDataMaker(path_RT_pickle, choice)
