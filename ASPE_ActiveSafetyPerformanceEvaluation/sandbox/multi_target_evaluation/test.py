# -*- coding: utf-8 -*-
"""
Created on Fri Nov 23 13:32:15 2018

@author: mj2snl
"""
import os
import pandas as pd
import multi_target_evaluation as mte
import sys
import time

sys.path.insert(0, r'..\..\TCKVTV00_JLR_SRR3\Heading Normalization')
import HeadingNormalization as hn
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.patches import Rectangle
import json
import math

# seperation w 2 ref and 2 est
path_rt = r"\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\RNASUV_SRR5_K0402B_20181115_201_LCZ_TC1_20P_001_rear_association_RTData.pkl"
path_tr = r'\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\RearRight_Z7B_64SD_64T_STAND_CORE.json'
scan_idx = 55154

## seperation w 2 ref and 2 est
#path_rt = r"\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\RNASUV_SRR5_K0402B_20181115_201_LCZ_TC1_20P_001_front_association_RTData.pkl"
#path_tr = r'\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\FrontRight_Z7B_64SD_64T_STAND_CORE.json'
#scan_idx = 53515

## oclusion w 2 ref and 1 est
#path_rt = r"\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\lat_opposite\RNASUV_SRR5_K0402B_20181115_201_LCZ_TC3_P20_001_rear_association_RTData.pkl"
#path_tr = r'\\DL16JQ1L2\Users\mj2snl\Desktop\multi_target\lat_opposite\RearRight_Z7B_64SD_64T_STAND_CORE.json'
#scan_idx = 49717

with open(path_tr) as f:
    json_data = json.load(f)

track_dict = {}

for idx in range(0, len(json_data['extraction'])):  # jedziemy po wszystkich scan indeksach
    track_dict[json_data['extraction'][idx]['Entity']['Detections']['dets_info']['ScanIndex']] = {}
    i = 1
    for nr_track in range(0, 64):
        if json_data['extraction'][idx]['Entity']['CurviTracks'][nr_track]['id'] != 0:
            track_dict[json_data['extraction'][idx]['Entity']['Detections']['dets_info']['ScanIndex']].update(
                {'pred_object_{}'.format(i): json_data['extraction'][idx]['Entity']['CurviTracks'][nr_track]})
            i += 1


side = 'right_sens_rt'
rt_data = pd.read_pickle(path_rt).to_dict('indexes')

# tr_data = pd.read_pickle(path_tr)[side]


initial_multi_target_data = {}
for ts in rt_data:
    initial_multi_target_data[ts] = {}
    initial_multi_target_data[ts].update({'ego': rt_data[ts][side]['ego']})
    initial_multi_target_data[ts].update({'ref_object_1': rt_data[ts][side]['target1']})
    initial_multi_target_data[ts].update({'ref_object_2': rt_data[ts][side]['target2']})

    # initial_multi_target_data[ts].update({'pred_object_1': tr_data[ts]['track1']})
    # initial_multi_target_data[ts].update({'pred_object_2': tr_data[ts]['track2']})

for si in initial_multi_target_data:
    h_ego = initial_multi_target_data[si]['ego']['AngleHeading']
    # p_h_1 = initial_multi_target_data[si]['pred_object_1']['TrackAngleHeading']
    # p_h_2 = initial_multi_target_data[si]['pred_object_2']['TrackAngleHeading']
    r_h_1 = initial_multi_target_data[si]['ref_object_1']['TargetAngleHeading']
    r_h_2 = initial_multi_target_data[si]['ref_object_2']['TargetAngleHeading']

    # initial_multi_target_data[si]['pred_object_1'].update({'target_norm_heading': hn.heading_normalization(h_ego,p_h_1)})
    # initial_multi_target_data[si]['pred_object_2'].update({'target_norm_heading': hn.heading_normalization(h_ego,p_h_2)})
    initial_multi_target_data[si]['ref_object_1'].update(
        {'reference_norm_heading': hn.heading_normalization(h_ego, r_h_1)})
    initial_multi_target_data[si]['ref_object_2'].update(
        {'reference_norm_heading': hn.heading_normalization(h_ego, r_h_2)})

for si in initial_multi_target_data:
    try:
        initial_multi_target_data[si].update(
            {'reference_objects': {'ref_object_1': initial_multi_target_data[si]['ref_object_1'],
                                   'ref_object_2': initial_multi_target_data[si]['ref_object_2']}})
        # initial_multi_target_data[si].update({'predicted_objects': {'pred_object_1': initial_multi_target_data[si]['pred_object_1'],
        # 'pred_object_2': initial_multi_target_data[si]['pred_object_2']}})
        if si in track_dict:
            initial_multi_target_data[si].update({'predicted_objects': track_dict[si]})
        else:
            initial_multi_target_data[si].update({'predicted_objects': {}})
    
        del initial_multi_target_data[si]['ego']
        # del initial_multi_target_data[si]['pred_object_1']
        # del initial_multi_target_data[si]['pred_object_2']
        del initial_multi_target_data[si]['ref_object_1']
        del initial_multi_target_data[si]['ref_object_2']
    except:
        initial_multi_target_data[si].update({'reference_objects': {}})
        initial_multi_target_data[si].update({'predicted_objects': {}})


def arc_lengh_calc(ang, ran):
    return ang * 2 * math.pi * ran / 360


def pol2cart(rho, phi):
    x = rho * np.cos(phi)
    y = rho * np.sin(phi)
    return (x, y)


fov_polar = [[x / 10 for x in range(0, 3600, 10)], [80 for x in range(0, 3600, 10)]]
fov_cart = []
for i in range(360):
    x, y = pol2cart(fov_polar[1][i], np.radians(fov_polar[0][i]))
    fov_cart.append([])
    fov_cart[i].append(x)
    fov_cart[i].append(y - 2.4)

start = time.time()

test_scenario = 'stage_test'
project_type = 'radars'
atributes = {'or': {'atribute_0': 'center_pos_lat', 'atribute_1': 'center_pos_long'},
             'oe': {'atribute_0': 'center_pos_lat', 'atribute_1': 'center_pos_long'}}

association_method = 'mahala_position_xy_only'
association_parameters = {'mahalanobis_distance_threshold': 2,
                          'var_0': 2,
                          'var_1': 5,
                          'heading': 'reference_norm_heading'}

binary_classification_parameters = {'FOV': fov_cart,
                                    'points': 'all',
                                    'or_velocity_long_param': 'RangeVelForward',
                                    'or_velocity_lat_param': 'RangeVelLateral',
                                    'velocity_treshold': 2,
                                    'cons_ti_short': 3,
                                    'cons_ti_long': 5,
                                    'separation_treshold': 0.5,
                                    'oclusion_treshold_rad': math.radians(3),
                                    'oclusion_point_lat': 0,
                                    'oclusion_point_long': 0}

reference_paramaters = {'front_bumper_center_pos_long': 'RangePosForward',
                        'front_bumper_center_pos_lat': 'RangePosLateral',
                        'front_bumper_center_vel_long': 'RangeVelForward',
                        'front_bumper_center_vel_lat': 'RangeVelLateral',
                        'width': 'RangeTargetVehicleWidth',
                        'length': 'RangeTargetVehicleLength',
                        'heading': 'reference_norm_heading'}

estimated_object_paramaters = {'center_pos_long': 'vcs_long_posn',
                        'center_pos_lat': 'vcs_lat_posn',
                        'front_bumper_center_vel_long': 'vcs_long_vel',
                        'front_bumper_center_vel_lat': 'vcs_lat_vel',
                        'width': 'width',
                        'length': 'length',
                        'heading': 'heading'}

obj1 = mte.multi_target_evaluation(initial_multi_target_data,
                                   test_scenario=test_scenario,
                                   project_type=project_type,
                                   atributes=atributes,
                                   association_method=association_method,
                                   association_parameters = association_parameters,
                                   binary_classification_parameters=binary_classification_parameters,
                                   reference_paramaters = reference_paramaters,
                                   estimated_object_paramaters = estimated_object_paramaters)

reference_object_eval_results = obj1.get_reference_object_eval_results()
bina_metric = obj1.get_binary_classification_metric()
association_metric = obj1.get_association_metric()

reference_additional_data = obj1.reference_additional_data
track_additional_data = obj1.track_additional_data
ocl_angles = obj1.oclusion_metric
#print(obj1.kwargs['association_parameters'])
end = time.time()
print('Calcualtion time (without data loading):', round(end - start,2), 'sec')
# visualization
#scan_idx = 49717#49717#53515
num_of_ref = len(initial_multi_target_data[scan_idx]['reference_objects'].keys())
num_of_tr = len(initial_multi_target_data[scan_idx]['predicted_objects'].keys())

# print(reference_object_eval_results[scan_idx]['ref_object_1'])
fov = []
vel = []
cons_ti = []
mah_dist = []
separation = []
oclusion = []

fig = plt.figure()
fig.patch.set_facecolor("white")
graph1 = fig.add_subplot(1, 1, 1)
graph1.grid(linestyle='--', alpha=0.5)

# Tracks
for i in range(1, num_of_tr + 1):
    TrFL_lat = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['front_left_corner_pos_lat']
    TrFR_lat = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['front_right_corner_pos_lat']
    TrRL_lat = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['rear_left_corner_pos_lat']
    TrRR_lat = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['rear_right_corner_pos_lat']
    TrFL_long = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['front_left_corner_pos_long']
    TrFR_long = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['front_right_corner_pos_long']
    TrRL_long = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['rear_left_corner_pos_long']
    TrRR_long = track_additional_data[scan_idx]['pred_object_{}'.format(i)]['rear_right_corner_pos_long']
    line1 = graph1.plot([TrFL_lat, TrFR_lat, TrRR_lat, TrRL_lat, TrFL_lat],
                        [TrFL_long, TrFR_long, TrRR_long, TrRL_long, TrFL_long], color='grey', linewidth=0.7, alpha=0.5)
    graph1.scatter((TrFL_lat + TrFR_lat) / 2, (TrFL_long + TrFR_long) / 2, color='grey', s=6, alpha=0.5)
    plt.text(TrFR_lat, TrFR_long, str(i))


# Targets
for i in range(1, num_of_ref + 1):
    TFL_lat = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['front_left_corner_pos_lat']
    TFR_lat = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['front_right_corner_pos_lat']
    TRL_lat = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['rear_left_corner_pos_lat']
    TRR_lat = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['rear_right_corner_pos_lat']
    TFL_long = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['front_left_corner_pos_long']
    TFR_long = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['front_right_corner_pos_long']
    TRL_long = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['rear_left_corner_pos_long']
    TRR_long = reference_additional_data[scan_idx]['ref_object_{}'.format(i)]['rear_right_corner_pos_long']
    line2 = graph1.plot([TFL_lat, TFR_lat, TRR_lat, TRL_lat, TFL_lat],
                        [TFL_long, TFR_long, TRR_long, TRL_long, TFL_long], color='deeppink', linewidth=0.7)
    graph1.scatter((TFL_lat + TFR_lat) / 2, (TFL_long + TFR_long) / 2, color='deeppink', s=6)
    plt.text(TFR_lat, TFR_long, str(i))
    # Association data
    fov.append('fov condition ref{}:'.format(i) + str(
        reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)]['fov_condition']))  # fov condition
    vel.append('vel condition ref{}:'.format(i) + str(
        reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)]['vel_condition']))  # vel condition
    cons_ti.append('cons_ti condition ref{}:'.format(i) + str(
        reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)]['cons_ti_condition']))  # cons_ti condition

    for key in reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)]:
        if 'separation_condition' in key:
            separation.append(str(key) + ' from ' + 'ref_object_{}'.format(i) + ':' + str(
                reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)][key]))
        if 'occlusion' in key:
            oclusion.append(str(key) + '_' + 'ref_object_{}'.format(i) + ':' + str(
                reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)][key]))

for i in range(1, num_of_ref + 1):
    for j in range(1, num_of_tr + 1):
        mah_dist.append('mah dist ref{}, pred{} :'.format(i, j) + str(
            round(reference_object_eval_results[scan_idx]['ref_object_{}'.format(i)]['pred_object_{}'.format(j)]['mahalanobis_distance'],3)))

txt = Rectangle((0, 0), 1, 1, fc="w", fill=False, edgecolor='none', linewidth=0)
if num_of_ref ==0 and num_of_tr == 0:
    legend_data = [txt]
    legend_labels = [str(scan_idx)]
elif num_of_ref == 0 and num_of_tr != 0:
    legend_data = [line1[0], txt]
    legend_labels = ['Estimated object', 'scan idx: ' + str(scan_idx)]
elif num_of_ref != 0 and num_of_tr == 0:
    legend_data = [line2[0], txt]
    legend_labels = ['Reference object', 'scan idx: ' + str(scan_idx)]
else:
    legend_data = [line1[0], line2[0], txt]
    legend_labels = ['Estimated object', 'Reference object', 'scan idx: ' + str(scan_idx)]


for i in range(0, num_of_tr * num_of_ref):  # adding mah dist info to legend
    legend_labels.append(mah_dist[i])
    legend_data.append(txt)

for i in range(0, num_of_ref):  # adding fov info to legend
    legend_labels.append(fov[i])
    legend_data.append(txt)

for i in range(0, num_of_ref):  # adding vel info to legend
    legend_labels.append(vel[i])
    legend_data.append(txt)

for i in range(0, num_of_ref):  # adding cons ti info to legend
    legend_labels.append(cons_ti[i])
    legend_data.append(txt)

for el in separation:  # adding separation info to legend
    legend_labels.append(el)
    legend_data.append(txt)

for el in oclusion:  # adding oclusion info to legend
    legend_labels.append(el)
    legend_data.append(txt)

'''       
for si in reference_object_eval_results:
    for ref in reference_object_eval_results[si]:
        for key, val in reference_object_eval_results[si][ref].items():
            if 'occlusion' in key and val == True:
                print(si, key, val, ocl_angles[si][ref]['oclusion_counter'], ocl_angles[si][ref]['covered_flag'])
'''

plt.xlim(-15, 15)
plt.ylim(-15, 15)
fig2 = plt.figure()
fig2.patch.set_facecolor("white")
legend = fig2.legend(legend_data, legend_labels, loc='center', prop={'size': 10}, frameon=1)
frame = legend.get_frame()
frame.set_facecolor('white')
frame.set_edgecolor('deeppink')
plt.show()


a = {'a': {}}
columns = ['ref1', 'ref2']
indexes = ['pred1', 'pred2']
d = np.array([[1, 2], [3, 4]])
df = pd.DataFrame()
df['ref1'] = pd.Series([2, 3], index=['pred2', 'pred3'])
df = df.drop_duplicates()
a['a'] = df




# Get a given data from a dictionary with position provided as a list
def getFromDict(dataDict, mapList):    
    for k in mapList: dataDict = dataDict[k]
    return dataDict

# Set a given data in a dictionary with position provided as a list
def setInDict(dataDict, mapList, value): 
    for k in mapList[:-1]: dataDict = dataDict[k]
    dataDict[mapList[-1]] = value


d = {"a" : {"b": {"c" : 1}},
     "1" : {"2": {"3" : 2}}}

k = getFromDict(d, ["a", "b", "c"])










