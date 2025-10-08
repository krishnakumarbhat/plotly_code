import json
import math
from matplotlib import pyplot as plt
from pprint import pprint


dict ={   'ref1':{'pred1': {'mah_dist': 1},'pred2': {'mah_dist': 0.1},'pred3': {'mah_dist': 0.5}} ,
          'ref2': {'pred1': {'mah_dist': 3}, 'pred2': {'mah_dist': 5},'pred3': {'mah_dist': 5}},
          'ref3': {'pred1': {'mah_dist': 0.5}, 'pred2': {'mah_dist': 15}, 'pred3': {'mah_dist': 1}}}

dict ={   'ref1':{'pred1': {'mah_dist': 1},'pred2': {'mah_dist': 0.1},'pred3': {'mah_dist': 0.5}} ,
          'ref2': {'pred1': {'mah_dist': 3}, 'pred2': {'mah_dist': 5},'pred3': {'mah_dist': 5}}}


conflict_tab = {}
exclude_tab = []

rep = 1
while rep == 1:
    rep = 0

    if (len(conflict_tab.keys()) < len(dict.keys())): #musi być tyle kluczy ile jest referencji
        for ref_obj in dict:
            preds_tab = []
            for pred_obj in dict[ref_obj]:
    
                if ref_obj in exclude_tab or pred_obj in exclude_tab:
                    continue
    
                min_mah = dict[ref_obj][pred_obj]['mah_dist']
                preds_tab.append([min_mah, pred_obj])
    
            try:
                min_mah, min_pred_obj = min(preds_tab)
            except:
                continue
            try:
                conflict_tab[min_pred_obj].update({min_mah: ref_obj})
            except:
                conflict_tab[min_pred_obj] = {min_mah: ref_obj}
    
        
        for pred_obj in conflict_tab:
            if len(list(conflict_tab[pred_obj].keys())) > 1:
                min_dist = min(list(conflict_tab[pred_obj].keys()))
                min_ref = conflict_tab[pred_obj][min_dist]
                conflict_tab[pred_obj] = {min_dist: min_ref}
                exclude_tab.append(min_ref)
                exclude_tab.append(min_pred_obj)
                rep = 1

print(conflict_tab)

closest_tab = {}
for ref_obj in dict:
    preds_tab = []
    for pred_obj in dict[ref_obj]:

        min_mah = dict[ref_obj][pred_obj]['mah_dist']
        preds_tab.append([min_mah, pred_obj])

    min_mah, min_pred_obj = min(preds_tab)
    closest_tab.update({ref_obj: {min_mah: min_pred_obj}})

print(closest_tab)























                # with open(r'C:\Users\jj81t8\Desktop\Wizualizacja\Multi_target\lat_opposite\RearRight_Z7B_64SD_64T_STAND_CORE.json') as f:
#     json_data = json.load(f)
#
# track_dict = {}
#
# for idx in range(0, len(json_data['extraction'])): #jedziemy po wszystkich scan indeksach
#     track_dict[json_data['extraction'][idx]['Entity']['Detections']['dets_info']['ScanIndex']]= {}
#     i = 1
#     for nr_track in range(0,64):
#         if json_data['extraction'][idx]['Entity']['CurviTracks'][nr_track]['id'] != 0:
#             track_dict[json_data['extraction'][idx]['Entity']['Detections']['dets_info']['ScanIndex']].update({'pred_object_{}'.format(i):json_data['extraction'][idx]['Entity']['CurviTracks'][nr_track]})
#             i+=1
#
# # for el in track_dict[49481]:
# #     print(el)
# # print(track_dict[49481]['pred_object_1']['heading'])
# # list =[]
# # for key in track_dict:
# #     list.append(key)
# #print(list)
#     #pprint(data['extraction'][i]['Entity']['Detections']['dets_info']['ScanIndex'])
#
# data = track_dict[49531]['pred_object_2']
# print(data)
#
# def track_points_calculation(data):  # calculation track points
#     data_output = {}
#
#     Tr_len = data['length']
#     Tr_width = data['width']
#
#     Tr_pos_center_lat = data['vcs_lat_posn']  #0. center point
#     Tr_pos_center_long = data['vcs_long_posn']
#
#     Tr_head = data['heading']
#
#     h_rad_tr = math.radians(Tr_head)
#
#     data_output.update({'center_pos_long': Tr_pos_center_long, 'center_pos_lat': Tr_pos_center_lat})
#
#                                               #1.front_bumper_center
#     lat = Tr_len * math.sin(h_rad_tr)
#     long = Tr_len * math.cos(h_rad_tr)
#     Tr_front_bump_long = Tr_pos_center_long + 0.5*long
#     Tr_front_bump_lat = Tr_pos_center_lat + 0.5*lat
#     data_output.update({'front_bumper_center_pos_long': Tr_front_bump_long, 'front_bumper_center_pos_lat': Tr_front_bump_lat})
#
#     lat = Tr_width / 2 * math.cos(h_rad_tr)  # 2.front_left_corner
#     long = Tr_width / 2 * math.sin(h_rad_tr)
#     new_c_lat =  Tr_front_bump_lat - lat
#     new_c_long = Tr_front_bump_long + long
#     data_output.update({'front_left_corner_pos_long': new_c_long, 'front_left_corner_pos_lat': new_c_lat})
#
#     lat = Tr_width / 2 * math.cos(h_rad_tr)  # 3.front_right_corner
#     long = Tr_width / 2 * math.sin(h_rad_tr)
#     new_c_lat = Tr_front_bump_lat + lat
#     new_c_long = Tr_front_bump_long - long
#     data_output.update({'front_right_corner_pos_long': new_c_long, 'front_right_corner_pos_lat': new_c_lat})
#
#     lat = Tr_len / 2 * math.sin(h_rad_tr)  # 4.left side center
#     long = Tr_len / 2 * math.cos(h_rad_tr)
#     new_c_lat = data_output['front_left_corner_pos_lat'] - lat
#     new_c_long = data_output['front_left_corner_pos_long'] - long
#     data_output.update({'left_side_center_pos_long': new_c_long, 'left_side_center_pos_lat': new_c_lat})
#
#     lat = Tr_len / 2 * math.sin(h_rad_tr)  # 5.right side center
#     long = Tr_len / 2 * math.cos(h_rad_tr)
#     new_c_lat = data_output['front_right_corner_pos_lat'] - lat
#     new_c_long = data_output['front_right_corner_pos_long'] - long
#     data_output.update({'right_side_center_pos_long': new_c_long, 'right_side_center_pos_lat': new_c_lat})
#
#     lat = Tr_len * math.sin(h_rad_tr)  # 6.rear left corner
#     long = Tr_len * math.cos(h_rad_tr)
#     new_c_lat = data_output['front_left_corner_pos_lat'] - lat
#     new_c_long = data_output['front_left_corner_pos_long'] - long
#     data_output.update({'rear_left_corner_pos_long': new_c_long, 'rear_left_corner_pos_lat': new_c_lat})
#
#     lat = Tr_len * math.sin(h_rad_tr)  # 7.rear right corner
#     long = Tr_len * math.cos(h_rad_tr)
#     new_c_lat = data_output['front_right_corner_pos_lat'] - lat
#     new_c_long = data_output['front_right_corner_pos_long'] - long
#     data_output.update({'rear_right_corner_pos_long': new_c_long, 'rear_right_corner_pos_lat': new_c_lat})
#
#     lat = Tr_len * math.sin(h_rad_tr)  # 8.rear bumper center
#     long = Tr_len * math.cos(h_rad_tr)
#     new_c_lat = Tr_front_bump_lat - lat
#     new_c_long = Tr_front_bump_long - long
#     data_output.update({'rear_bumper_center_pos_long': new_c_long, 'rear_bumper_center_pos_lat': new_c_lat})
#
#     return data_output
#
# tr_points = track_points_calculation(data)
# print(tr_points)
#
# fl_lat = tr_points['front_left_corner_pos_lat']
# fr_lat = tr_points['front_right_corner_pos_lat']
# rr_lat = tr_points['rear_right_corner_pos_lat']
# rl_lat = tr_points['rear_left_corner_pos_lat']
#
# fl_long = tr_points['front_left_corner_pos_long']
# fr_long = tr_points['front_right_corner_pos_long']
# rr_long = tr_points['rear_right_corner_pos_long']
# rl_long = tr_points['rear_left_corner_pos_long']
#
# fig = plt.figure()
# fig.patch.set_facecolor("mistyrose")
# graph1 = fig.add_subplot(1, 1, 1)
# graph1.grid(linestyle='--', alpha=0.5)
# graph1.plot([fl_lat, fr_lat, rr_lat, rl_lat,fl_lat], [fl_long, fr_long, rr_long, rl_long, fl_long])
# plt.show()

