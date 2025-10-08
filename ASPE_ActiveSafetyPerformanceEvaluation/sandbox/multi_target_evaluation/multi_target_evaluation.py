# -*- coding: utf-8 -*-
"""
Created on Fri Nov 23 13:32:15 2018

@author: mj2snl
"""
import math
import numpy as np
from scipy.spatial import distance
from matplotlib import pyplot as plt
from tqdm import tqdm
import pandas as pd


class multi_target_evaluation():
    def __str__(self):
        "input as dict with reference objects and predicted objects, data structure like:"
        "data[time_instance][reference_objects][object_n][or_position_x]"
        "                                                [or_position_y]"
        "                                                [or_velocity_x]"
        "                                                [or_velocity_y]"
        "                   [predicted_objects][object_n][oe_position_x]"
        "                                                [oe_position_y]"
        "                                                [oe_velocity_x]"
        "                                                [oe_velocity_y] ect..."
        "output data as 2 dicts: association metric and binary classification metric"

    def __init__(self, multi_target_data=None, **kwargs):
        self.kwargs = kwargs

        if isinstance(multi_target_data, dict) is False:  # check if input data exist(dict)
            raise ValueError('no data file attached or incorrect format (excepted: dict)')
        else:
            self.multi_target_data = multi_target_data

        if 'test_scenario' in kwargs:  # check if test_scenario exist
            self.test_scenario = kwargs['test_scenario']
        else:
            raise ValueError("please select test scenario (e.g test_scenario = 'stage_test')")
        if 'project_type' in kwargs:  # check if projet type exist
            self.project_type = kwargs['project_type']
        else:
            raise ValueError("please select project type (e.g project_type = 'radars')")

        if 'atributes' in kwargs:  # check if atributes exist
            self.atributes_length = int(len(kwargs['atributes']['or']))
            self.oe_matrix = np.zeros(self.atributes_length)  # oe matrix
            self.or_matrix = np.zeros(self.atributes_length)  # or matrix
            self.cov_matrix = np.zeros((self.atributes_length, self.atributes_length))  # cov matrix
        else:
            raise ValueError("please select atributes")

        if 'association_method' in kwargs:  # check if association method exist
            self.association_method = kwargs['association_method']
            if self.association_method == 'default':
                self.association_method = 'mahala_position_xy_only'
        else:
            raise ValueError("please select association method")

        if 'association_parameters' in kwargs:  # check if association_parameters exist
            if kwargs['association_parameters'] == 'default':  # default association_parameters
                self.association_method = 'mahala_position_xy_only'
                self.kwargs['association_parameters'] = {'mahalanobis_distance_threshold': 1.5,
                                             'var_0': 2,
                                             'var_1': 2,
                                             'cov_01': 0,
                                             'cov_10': 0}
            elif self.association_method == 'mahala_position_xy_only':
                pass
        else:
            raise ValueError("please select association parameters")


    def reference_object_eval_calc(self):  # calculate association metric and check binary clssification association_parameters
        if self.association_method == 'mahala_position_xy_only':  #assoc method: mahalanobis_pos_xy_only
            self.reference_object_eval_results = {}         # association metric
            self.reference_additional_data = {}  # reference additional data
            self.track_additional_data = {}      # track additional data
            self.ref_pred_mah_dist_tab = {}
            for ti in self.multi_target_data:  # for every scan idx
                self.reference_object_eval_results[ti] = {}
                self.reference_additional_data[ti] = {}
                self.track_additional_data[ti] = {}
                self.ref_pred_mah_dist_tab[ti] = {}

                for pr_obj in self.multi_target_data[ti]['predicted_objects']:  # filling track additional data with pos points
                    oe_pred_points = multi_target_evaluation.track_points_calculation(self.multi_target_data[ti]['predicted_objects'][pr_obj], self.kwargs['estimated_object_paramaters'])
                    self.track_additional_data[ti].update({pr_obj: oe_pred_points})

                for ref_obj in self.multi_target_data[ti]['reference_objects']:  # for every ref objects
                    or_ref_points = multi_target_evaluation.rt_ref_points_calculation(self.multi_target_data[ti]['reference_objects'][ref_obj], self.kwargs['reference_paramaters'])
                    self.reference_additional_data[ti].update({ref_obj: or_ref_points})

                    self.reference_object_eval_results[ti][ref_obj] = {}
                    self.ref_pred_mah_dist_tab[ti][ref_obj] = {}

                    for pred_obj in self.multi_target_data[ti]['predicted_objects']:  # for every pred objects
                        self.reference_object_eval_results[ti][ref_obj][pred_obj] = {}
                        self.ref_pred_mah_dist_tab[ti][ref_obj][pred_obj] = {}

                        for x in range(self.atributes_length):  # fill or, oe and cov matrix on diagonal

                            try: # if not find atribute in ref_additional_data, search in multi_target_data
                                self.or_matrix[x] = self.reference_additional_data[ti][ref_obj][self.kwargs['atributes']['or'][f'atribute_{x}']]
                            except:
                                self.or_matrix[x] = self.multi_target_data[ti]['reference_objects'][ref_obj][self.kwargs['atributes']['or'][f'atribute_{x}']]
                            try:
                                self.oe_matrix[x] = self.track_additional_data[ti][pred_obj][self.kwargs['atributes']['oe'][f'atribute_{x}']]
                            except:
                                self.oe_matrix[x] = self.multi_target_data[ti]['predicted_objects'][pred_obj][self.kwargs['atributes']['oe'][f'atribute_{x}']]

                            if isinstance(self.kwargs['association_parameters'][f'var_{x}'], int) or isinstance(self.kwargs['association_parameters'][f'var_{x}'], float):
                                self.cov_matrix[x, x] = self.kwargs['association_parameters'][f'var_{x}']

                            elif isinstance(self.kwargs['association_parameters'][f'var_{x}'], str):
                                self.cov_matrix[x, x] = self.multi_target_data[ti]['reference_objects'][self.kwargs['association_parameters'][f'var_{x}']]

                        if 'heading' in self.kwargs['association_parameters']:
                            heading = self.kwargs['association_parameters']['heading']  # rotate covariance matrix
                            self.cov_matrix_n = multi_target_evaluation.var_cov_rotation_matrix(self.multi_target_data[ti]['reference_objects'][ref_obj][heading], self.cov_matrix)
                        else:
                            for i in range(self.atributes_length): #filling
                                for j in range(self.atributes_length):
                                    if i != j:
                                        cov_val = self.kwargs['association_parameters']['cov_' + f'{i}' + f'{j}']
                                        self.cov_matrix[i, j] = cov_val
                            self.cov_matrix_n = self.cov_matrix  #rotated cov matrix

                        # calculate mahalanobis distance
                        mah_dist = multi_target_evaluation.mahalanobis_distance_calc(self.or_matrix, self.oe_matrix,self.cov_matrix_n)
                        self.ref_pred_mah_dist_tab[ti][ref_obj][pred_obj].update({'mahalanobis_distance': mah_dist})
                        self.reference_object_eval_results[ti][ref_obj][pred_obj].update({'mahalanobis_distance': mah_dist})

                #calculate closest no conflict pred to ref
                closest_no_conflict_tab = self.calc_closest_no_conflict_tab_pred_to_ref(ti)

                for pred_obj in closest_no_conflict_tab:
                    for mah_dist, ref_obj in closest_no_conflict_tab[pred_obj].items():
                        self.reference_object_eval_results[ti][ref_obj].update({'closest_no_conflict_eo': pred_obj})

                #calculate closest pred to ref with repeats
                closest_tab = {}  # closest eo to ref with repeats (unique ref) (ref main key)
                for ref_obj in self.ref_pred_mah_dist_tab[ti]:
                    preds_tab = []
                    for pred_obj in self.ref_pred_mah_dist_tab[ti][ref_obj]:
                        min_mah = self.ref_pred_mah_dist_tab[ti][ref_obj][pred_obj]['mahalanobis_distance']
                        preds_tab.append([min_mah, pred_obj])

                    try:
                        min_mah, min_pred_obj = min(preds_tab)
                        closest_tab.update({ref_obj: {min_mah: min_pred_obj}})
                        self.reference_object_eval_results[ti][ref_obj].update({'closest_w_repeats_eo': min_pred_obj})
                    except:
                        pass

                for ref_obj in self.reference_object_eval_results[ti]: # if for ref doesn't exsist closest_no_conflict_eo/ closest_w_repeats_eo
                    if 'closest_no_conflict_eo' not in self.reference_object_eval_results[ti][ref_obj]:
                        self.reference_object_eval_results[ti][ref_obj].update({'closest_no_conflict_eo': None})
                    if 'closest_w_repeats_eo' not in self.reference_object_eval_results[ti][ref_obj]:
                        self.reference_object_eval_results[ti][ref_obj].update({'closest_w_repeats_eo': None})
                        

        # binary classification association_parameters calculation
        if 'binary_classification_parameters' in self.kwargs and self.kwargs['project_type'] == 'radars':
            oclusion_metric = {}

            for i in tqdm(range(len(list(self.multi_target_data.keys())))):
                ti = list(self.multi_target_data.keys())[i]
                oclusion_tab = {}
                oclusion_metric[ti] = {}
                for ref_obj in self.multi_target_data[ti]['reference_objects']:
                    oclusion_tab[ref_obj] = {}

                    #FOV condition
                    fov_condition, lowest_pos_lat, lowest_pos_long = multi_target_evaluation.default_fov_condition(self.kwargs['binary_classification_parameters']['FOV'],
                                                                                        self.kwargs['binary_classification_parameters']['points'],
                                                                                        self.reference_additional_data[ti][ref_obj],
                                                                                        self.multi_target_data[ti]['reference_objects'][ref_obj])

                    self.reference_object_eval_results[ti][ref_obj].update({'fov_condition': fov_condition})

                    #velocity condition
                    vel_condition = multi_target_evaluation.default_vel_condition(self.kwargs['binary_classification_parameters']['or_velocity_lat_param'],
                                                                                  self.kwargs['binary_classification_parameters']['or_velocity_long_param'],
                                                                                  self.kwargs['binary_classification_parameters']['velocity_treshold'],
                                                                                  self.multi_target_data[ti]['reference_objects'][ref_obj])
                    self.reference_object_eval_results[ti][ref_obj].update({'vel_condition': vel_condition})

                    #consecutive time instances condition
                    cons_ti_cond = multi_target_evaluation.default_cons_ti_condition(i,
                                                                                     ref_obj,
                                                                                     lowest_pos_lat,
                                                                                     lowest_pos_long,
                                                                                     self.kwargs['binary_classification_parameters']['cons_ti_short'],
                                                                                     self.kwargs['binary_classification_parameters']['cons_ti_long'],
                                                                                     self.reference_object_eval_results,
                                                                                     self.multi_target_data)
                    self.reference_object_eval_results[ti][ref_obj].update({'cons_ti_condition': cons_ti_cond})


                    #separation condition
                    for ref_obj2 in self.multi_target_data[ti]['reference_objects']:
                        if int(ref_obj[-1]) < int(ref_obj2[-1]):  # if references are different (without repetition)
                            dist = []  # distance between references (each pair of points)
                            for point in self.reference_additional_data[ti][ref_obj]:
                                for point2 in self.reference_additional_data[ti][ref_obj]:
                                    if 'lat' in point and 'lat' in point2:
                                        pos_lat = self.reference_additional_data[ti][ref_obj][point] - \
                                                  self.reference_additional_data[ti][ref_obj2][point2]
                                        pos_long = self.reference_additional_data[ti][ref_obj][point[:-3:] + 'long'] - \
                                                   self.reference_additional_data[ti][ref_obj2][point2[:-3:] + 'long']
                                        dist.append(math.hypot(pos_lat, pos_long))

                            # TODO change sepration conditions as in book
                            dist = min(dist)  # min distance between references
                            if dist <= self.kwargs['binary_classification_parameters']['separation_treshold']:
                                self.reference_object_eval_results[ti][ref_obj].update(
                                    {'separation_condition_to_' + ref_obj2: False})
                                self.reference_object_eval_results[ti][ref_obj2].update(
                                    {'separation_condition_to_' + ref_obj: False})
                            else:
                                self.reference_object_eval_results[ti][ref_obj].update(
                                    {'separation_condition_to_' + ref_obj2: True})
                                self.reference_object_eval_results[ti][ref_obj2].update(
                                    {'separation_condition_to_' + ref_obj: True})
                            self.reference_object_eval_results[ti][ref_obj].update({'min_dist_to_' + ref_obj2: dist})
                            self.reference_object_eval_results[ti][ref_obj2].update({'min_dist_to_' + ref_obj: dist})

                    # Oclusion
                    #  finding oclusion_point_lat and oclusion_point_long
                    if isinstance(self.kwargs['binary_classification_parameters']['oclusion_point_lat'], str):
                        oclusion_point_lat = self.multi_target_data['reference_objects'][self.kwargs['binary_classification_parameters']['oclusion_point_lat']]
                    else:
                        oclusion_point_lat = self.kwargs['binary_classification_parameters']['oclusion_point_lat']

                    if isinstance(self.kwargs['binary_classification_parameters']['oclusion_point_long'], str):
                        oclusion_point_long = self.multi_target_data['reference_objects'][self.kwargs['binary_classification_parameters']['oclusion_point_long']]
                    else:
                        oclusion_point_long = self.kwargs['binary_classification_parameters']['oclusion_point_long']

                    # polar points for oclusion condition - changing cartesian to polar
                    angle_ref1 = []  # angles of reference 1 points
                    range_ref1 = []
                    oclusion_metric[ti][ref_obj] = {'oclusion_counter': 0, 'covered_flag': False}
                    for point in self.reference_additional_data[ti][ref_obj]:
                        # polar points for oclusion condition
                        if 'lat' in point:

                            # point for first reference (with oclusion point corection (depend on sensor))
                            point1_pos_lat_cart = self.reference_additional_data[ti][ref_obj][point] + oclusion_point_lat
                            point1_pos_long_cart = self.reference_additional_data[ti][ref_obj][point[:-3:] + 'long'] + oclusion_point_long
                            point1_pos_radius, point1_pos_angle = multi_target_evaluation.cart2pol(point1_pos_lat_cart,
                                                                                                   point1_pos_long_cart)

                            if point1_pos_angle < 0:  # normalization of angle (from (-179 - 180) to (0 - 360))
                                point1_pos_angle = math.pi + (math.pi - abs(point1_pos_angle))
                            angle_ref1.append(point1_pos_angle)
                            range_ref1.append(point1_pos_radius)

                    min_ang_ref1 = 0.0
                    max_ang_ref1 = 0.0
                    diff = 0.0
                    # calculate min and max angle and range for reference
                    for ang1 in angle_ref1:
                        for ang2 in angle_ref1:
                            temp_diff = abs(ang1 - ang2)
                            if temp_diff >= math.pi:
                                temp_diff = 360 % temp_diff
                            if diff < temp_diff:
                                diff = temp_diff
                                min_ang_ref1 = min(ang1, ang2)
                                max_ang_ref1 = max(ang1, ang2)
                                min_ang_range = range_ref1[angle_ref1.index(min_ang_ref1)]
                                max_ang_range = range_ref1[angle_ref1.index(max_ang_ref1)]
                    # discretize angles
                    x_min, y_min = multi_target_evaluation.pol2cart(min_ang_range, min_ang_ref1)
                    x_max, y_max = multi_target_evaluation.pol2cart(max_ang_range, max_ang_ref1)
                    disc_x = np.linspace(x_min, x_max, 10)
                    disc_y = np.linspace(y_min, y_max, 10)
                    for x in range(len(disc_x)):
                        disc_ran, disc_ang = multi_target_evaluation.cart2pol(disc_x[x], disc_y[x])
                        if disc_ang < 0:  # normalization of angle (from (-179 - 180) to (0 - 360))
                            disc_ang = math.pi + (math.pi - abs(disc_ang))
                        oclusion_tab[ref_obj].update({disc_ang: disc_ran})

                for x in range(0, 3600, 5):
                    x = math.radians(x / 10)
                    lower_ang = x
                    higher_ang = x + 0.005
                    references_comparision = []
                    for ref in oclusion_tab:
                        for ang, ran in oclusion_tab[ref].items():
                            if (ang >= lower_ang and ang <= higher_ang):
                                # print(ang, ran, lower_ang, higher_ang)
                                references_comparision.append([ran, ref])
                    if references_comparision:
                        min_ran, min_ref = min(references_comparision)
                        for ran, ref in references_comparision:
                            if ref != min_ref:
                                oclusion_metric[ti][ref]['covered_flag'] = True

                        oclusion_metric[ti][min_ref]['oclusion_counter'] += 1
                        if oclusion_metric[ti][ref]['covered_flag'] != True:
                            oclusion_metric[ti][min_ref]['covered_flag'] = False

                for ref in self.reference_object_eval_results[ti]:
                    self.reference_object_eval_results[ti][ref].update({'occlusion_sector_counter': oclusion_metric[ti][ref]['oclusion_counter']})
                    self.reference_object_eval_results[ti][ref].update({'occlusion_coverage': oclusion_metric[ti][ref]['covered_flag']})
                    if oclusion_metric[ti][ref]['covered_flag'] == True:
                        if oclusion_metric[ti][ref]['oclusion_counter'] * 0.5 < math.degrees(
                                self.kwargs['binary_classification_parameters']['oclusion_treshold_rad']):
                            self.reference_object_eval_results[ti][ref].update({'occlusion_condition': True})
                            self.reference_object_eval_results[ti][ref].update(
                                {'cons_ti_condition': False})  # when oclusion is occured cons ti is reseted
                        else:
                            self.reference_object_eval_results[ti][ref].update({'occlusion_condition': False})
                    else:
                        self.reference_object_eval_results[ti][ref].update({'occlusion_condition': False})

            self.oclusion_metric = oclusion_metric

                
    def get_association_metric(self):  #association metric calculation
        try:
            self.reference_object_eval_results
        except:
            self.reference_object_eval_calc()
            
        association_metric = {}
        for ti in self.reference_object_eval_results:
            association_metric[ti] = {}
            df = pd.DataFrame()
            for ref_obj in self.reference_object_eval_results[ti]:
                pred_obj_list = [x for x in self.reference_object_eval_results[ti][ref_obj] if 'pred' in x] # all pred objcts name in particular reference
                pred_obj_dists_list = [round(self.reference_object_eval_results[ti][ref_obj][x]['mahalanobis_distance'],2) for x in self.reference_object_eval_results[ti][ref_obj] if 'pred' in x] # all pred objcts mahalanobis distance in particular reference
                        
                df[ref_obj] = pd.Series(pred_obj_dists_list, index=pred_obj_list)
                
            association_metric[ti] = df
        self.association_metric = association_metric
        return self.association_metric
        

    def bianary_classification_metric_calc(self):  # calculate binary classification metric
        if 'binary_classification_parameters' not in self.kwargs:
            raise ValueError("please select binary classification parameters")
        try:
            self.reference_object_eval_results
        except:
            self.reference_object_eval_calc()

        if self.kwargs['project_type'] == 'radars':
            self.binary_classification_metric = {}
            for ti in self.reference_object_eval_results:
                self.binary_classification_metric[ti] = {}
                self.binary_classification_metric[ti]['TP'] = {}
                self.binary_classification_metric[ti]['FP'] = {}
                self.binary_classification_metric[ti]['FN'] = {}
                self.binary_classification_metric[ti]['NR'] = {}
                self.binary_classification_metric[ti]['DCO'] = {}

                pred_obj_list = [x for x in self.multi_target_data[ti]['predicted_objects']]
                for ref_obj in self.reference_object_eval_results[ti]:
                    cons_ti_condition = self.reference_object_eval_results[ti][ref_obj]['cons_ti_condition']
                    fov_condition = self.reference_object_eval_results[ti][ref_obj]['fov_condition']
                    try:
                        occlusion_condition = self.reference_object_eval_results[ti][ref_obj]['occlusion_condition']
                    except:
                        print(ti)
                    vel_condition = self.reference_object_eval_results[ti][ref_obj]['vel_condition']
                    for key in self.reference_object_eval_results[ti][ref_obj]:
                        if 'separation' in key and self.reference_object_eval_results[ti][ref_obj][key] == False:
                            separation_condition = self.reference_object_eval_results[ti][ref_obj][key]
                            break
                        else:
                            separation_condition = True
                    closest_no_conflict_eo = self.reference_object_eval_results[ti][ref_obj]['closest_no_conflict_eo']
                    closest_w_repeats_eo = self.reference_object_eval_results[ti][ref_obj]['closest_w_repeats_eo']

                    if cons_ti_condition == True and fov_condition == True and vel_condition == True:
                        track_fromulation = True
                    else:
                        track_fromulation = False

                    if closest_no_conflict_eo != None:
                        if track_fromulation == True and self.reference_object_eval_results[ti][ref_obj][closest_no_conflict_eo][
                            'mahalanobis_distance'] <= self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                            self.binary_classification_metric[ti]['TP'].update({ref_obj: closest_no_conflict_eo})
                            del pred_obj_list[pred_obj_list.index(closest_no_conflict_eo)]

                        if track_fromulation == True and self.reference_object_eval_results[ti][ref_obj][closest_no_conflict_eo][
                            'mahalanobis_distance'] > self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                            if occlusion_condition == False and separation_condition == True:
                                self.binary_classification_metric[ti]['FN'].update({ref_obj: None})
                                self.binary_classification_metric[ti]['FP'].update({closest_no_conflict_eo: ref_obj})
                                del pred_obj_list[pred_obj_list.index(closest_no_conflict_eo)]
                            else:
                                self.binary_classification_metric[ti]['NR'].update({ref_obj: None})

                        if track_fromulation == False and self.reference_object_eval_results[ti][ref_obj][closest_no_conflict_eo][
                            'mahalanobis_distance'] <= self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                            self.binary_classification_metric[ti]['TP'].update({ref_obj: closest_no_conflict_eo})
                            del pred_obj_list[pred_obj_list.index(closest_no_conflict_eo)]

                        if track_fromulation == False and self.reference_object_eval_results[ti][ref_obj][closest_no_conflict_eo]['mahalanobis_distance'] > self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                            self.binary_classification_metric[ti]['NR'].update({ref_obj: None})

                    else:
                        #TODO need to be reformulate and verificate
                        if track_fromulation == True and occlusion_condition == False and separation_condition == True:
                            self.binary_classification_metric[ti]['FN'].update({ref_obj: None})
                        else:
                            if closest_w_repeats_eo == None and track_fromulation == True:
                                self.binary_classification_metric[ti]['FN'].update({ref_obj: None})
                            elif track_fromulation == True and self.reference_object_eval_results[ti][ref_obj][closest_w_repeats_eo][
                            'mahalanobis_distance'] <= self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                                self.binary_classification_metric[ti]['FN'].update({ref_obj: None})
                            else:
                                self.binary_classification_metric[ti]['NR'].update({ref_obj: None})

                for pred_obj in pred_obj_list:
                    temp_pred_tab = []
                    for ref_obj in self.reference_object_eval_results[ti]:
                        mah_dist = self.reference_object_eval_results[ti][ref_obj][pred_obj]['mahalanobis_distance']
                        temp_pred_tab.append([mah_dist, ref_obj, pred_obj])
                    try:
                        mah_dist, ref_obj, pred_obj = min(temp_pred_tab)
                    except:
                        self.binary_classification_metric[ti]['FP'].update({pred_obj: None})
                        continue
                    if mah_dist <= self.kwargs['association_parameters']['mahalanobis_distance_threshold']:
                        self.binary_classification_metric[ti]['DCO'].update({pred_obj: ref_obj})
                    else:
                        self.binary_classification_metric[ti]['FP'].update({pred_obj: None})

    def var_cov_rotation_matrix(angle, variance_matrix, clockwise=True):  # rotate cov matrix
        '''
        angle in degrees
        var-s as variances of position x and y
        '''
        theta = math.radians(angle)
        if clockwise == True:
            rotation_matrix = np.array([[np.cos(theta), np.sin(theta)],
                                        [-np.sin(theta), np.cos(theta)]])
        else:
            rotation_matrix = np.array([[np.cos(theta), -np.sin(theta)],
                                        [np.sin(theta), np.cos(theta)]])

        transpose_rotation_matrix = np.matrix.transpose(rotation_matrix)

        variance_matrix_const = variance_matrix

        var_cov_matrix_rotated = np.matmul(np.matmul(rotation_matrix, variance_matrix_const), transpose_rotation_matrix)
        return var_cov_matrix_rotated

    def mahalanobis_distance_calc(or_matrix, oe_matrix, covariance_matrix):  # calculate mahalanobis dist
        return distance.mahalanobis(or_matrix, oe_matrix, np.linalg.inv(covariance_matrix))

    def get_reference_object_eval_results(self):  # return association metric
        try:
            self.reference_object_eval_results
        except:
            self.reference_object_eval_calc()
        return self.reference_object_eval_results

    def get_binary_classification_metric(self):
        try:
            self.binary_classification_metric
        except:
            self.bianary_classification_metric_calc()
        return self.binary_classification_metric

    def point_inside_polygon(x, y, poly):  # check if point is inside polygon
        plen = len(poly)
        inside = False
        p1x, p1y = poly[0]
        for i in range(plen + 1):
            p2x, p2y = poly[i % plen]
            if y > min(p1y, p2y):
                if y <= max(p1y, p2y):
                    if x <= max(p1x, p2x):
                        if p1y != p2y:
                            xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                        if p1x == p2x or x <= xinters:
                            inside = not inside
            p1x, p1y = p2x, p2y
        return inside

    def cart2pol(x, y):  # change cartesian to polar
        rho = np.sqrt(x ** 2 + y ** 2)
        phi = np.arctan2(y, x)
        return (rho, phi)

    def pol2cart(rho, phi):
        x = rho * np.cos(phi)
        y = rho * np.sin(phi)
        return (x, y)

    def rt_ref_points_calculation(data, reference_parameters):  # calculation of ref points for reference
        data_output = {}

        RT_len = data[reference_parameters['length']]
        RT_width = data[reference_parameters['width']]

        RT_pos_forw = data[reference_parameters['front_bumper_center_pos_long']]  # 1.front_bumper center
        RT_pos_lat = data[reference_parameters['front_bumper_center_pos_lat']]

        RT_head = data[reference_parameters['heading']]

        h_rad_rt = math.radians(RT_head)

        data_output.update({'front_bumper_center_pos_long': RT_pos_forw, 'front_bumper_center_pos_lat': RT_pos_lat})

        lat = RT_width / 2 * math.cos(h_rad_rt)  # 2.front_left_corner
        long = RT_width / 2 * math.sin(h_rad_rt)
        new_c_lat = RT_pos_lat - lat
        new_c_long = RT_pos_forw + long
        data_output.update({'front_left_corner_pos_long': new_c_long, 'front_left_corner_pos_lat': new_c_lat})

        lat = RT_width / 2 * math.cos(h_rad_rt)  # 3.front_right_corner
        long = RT_width / 2 * math.sin(h_rad_rt)
        new_c_lat = RT_pos_lat + lat
        new_c_long = RT_pos_forw - long
        data_output.update({'front_right_corner_pos_long': new_c_long, 'front_right_corner_pos_lat': new_c_lat})

        lat = RT_len / 2 * math.sin(h_rad_rt)  # 4.left side center
        long = RT_len / 2 * math.cos(h_rad_rt)
        new_c_lat = data_output['front_left_corner_pos_lat'] - lat
        new_c_long = data_output['front_left_corner_pos_long'] - long
        data_output.update({'left_side_center_pos_long': new_c_long, 'left_side_center_pos_lat': new_c_lat})

        lat = RT_len / 2 * math.sin(h_rad_rt)  # 5.right side center
        long = RT_len / 2 * math.cos(h_rad_rt)
        new_c_lat = data_output['front_right_corner_pos_lat'] - lat
        new_c_long = data_output['front_right_corner_pos_long'] - long
        data_output.update({'right_side_center_pos_long': new_c_long, 'right_side_center_pos_lat': new_c_lat})

        lat = RT_len * math.sin(h_rad_rt)  # 6.rear left corner
        long = RT_len * math.cos(h_rad_rt)
        new_c_lat = data_output['front_left_corner_pos_lat'] - lat
        new_c_long = data_output['front_left_corner_pos_long'] - long
        data_output.update({'rear_left_corner_pos_long': new_c_long, 'rear_left_corner_pos_lat': new_c_lat})

        lat = RT_len * math.sin(h_rad_rt)  # 7.rear right corner
        long = RT_len * math.cos(h_rad_rt)
        new_c_lat = data_output['front_right_corner_pos_lat'] - lat
        new_c_long = data_output['front_right_corner_pos_long'] - long
        data_output.update({'rear_right_corner_pos_long': new_c_long, 'rear_right_corner_pos_lat': new_c_lat})

        lat = RT_len * math.sin(h_rad_rt)  # 8.rear bumper center
        long = RT_len * math.cos(h_rad_rt)
        new_c_lat = RT_pos_lat - lat
        new_c_long = RT_pos_forw - long
        data_output.update({'rear_bumper_center_pos_long': new_c_long, 'rear_bumper_center_pos_lat': new_c_lat})

        # 9.center
        lat = (data_output['front_bumper_center_pos_lat'] + data_output['rear_bumper_center_pos_lat']) / 2
        long = (data_output['front_bumper_center_pos_long'] + data_output['rear_bumper_center_pos_long']) / 2
        data_output.update({'center_pos_long': long, 'center_pos_lat': lat})

        return data_output

    def track_points_calculation(data, estimated_obj_parameters):  # calculation track points
        data_output = {}

        Tr_len = data[estimated_obj_parameters['length']]
        Tr_width = data[estimated_obj_parameters['width']]

        Tr_pos_center_lat = data[estimated_obj_parameters['center_pos_lat']] # 0. center point
        Tr_pos_center_long = data[estimated_obj_parameters['center_pos_long']]

        Tr_head = data[estimated_obj_parameters['heading']]

        h_rad_tr = Tr_head  # math.radians(Tr_head)

        data_output.update({'center_pos_long': Tr_pos_center_long, 'center_pos_lat': Tr_pos_center_lat})

        lat = Tr_len * math.sin(h_rad_tr)  # 1.front_bumper_center
        long = Tr_len * math.cos(h_rad_tr)
        Tr_front_bump_long = Tr_pos_center_long + 0.5 * long
        Tr_front_bump_lat = Tr_pos_center_lat + 0.5 * lat
        data_output.update(
            {'front_bumper_center_pos_long': Tr_front_bump_long, 'front_bumper_center_pos_lat': Tr_front_bump_lat})

        lat = Tr_width / 2 * math.cos(h_rad_tr)  # 2.front_left_corner
        long = Tr_width / 2 * math.sin(h_rad_tr)
        new_c_lat = Tr_front_bump_lat - lat
        new_c_long = Tr_front_bump_long + long
        data_output.update({'front_left_corner_pos_long': new_c_long, 'front_left_corner_pos_lat': new_c_lat})

        lat = Tr_width / 2 * math.cos(h_rad_tr)  # 3.front_right_corner
        long = Tr_width / 2 * math.sin(h_rad_tr)
        new_c_lat = Tr_front_bump_lat + lat
        new_c_long = Tr_front_bump_long - long
        data_output.update({'front_right_corner_pos_long': new_c_long, 'front_right_corner_pos_lat': new_c_lat})

        lat = Tr_len / 2 * math.sin(h_rad_tr)  # 4.left side center
        long = Tr_len / 2 * math.cos(h_rad_tr)
        new_c_lat = data_output['front_left_corner_pos_lat'] - lat
        new_c_long = data_output['front_left_corner_pos_long'] - long
        data_output.update({'left_side_center_pos_long': new_c_long, 'left_side_center_pos_lat': new_c_lat})

        lat = Tr_len / 2 * math.sin(h_rad_tr)  # 5.right side center
        long = Tr_len / 2 * math.cos(h_rad_tr)
        new_c_lat = data_output['front_right_corner_pos_lat'] - lat
        new_c_long = data_output['front_right_corner_pos_long'] - long
        data_output.update({'right_side_center_pos_long': new_c_long, 'right_side_center_pos_lat': new_c_lat})

        lat = Tr_len * math.sin(h_rad_tr)  # 6.rear left corner
        long = Tr_len * math.cos(h_rad_tr)
        new_c_lat = data_output['front_left_corner_pos_lat'] - lat
        new_c_long = data_output['front_left_corner_pos_long'] - long
        data_output.update({'rear_left_corner_pos_long': new_c_long, 'rear_left_corner_pos_lat': new_c_lat})

        lat = Tr_len * math.sin(h_rad_tr)  # 7.rear right corner
        long = Tr_len * math.cos(h_rad_tr)
        new_c_lat = data_output['front_right_corner_pos_lat'] - lat
        new_c_long = data_output['front_right_corner_pos_long'] - long
        data_output.update({'rear_right_corner_pos_long': new_c_long, 'rear_right_corner_pos_lat': new_c_lat})

        lat = Tr_len * math.sin(h_rad_tr)  # 8.rear bumper center
        long = Tr_len * math.cos(h_rad_tr)
        new_c_lat = Tr_front_bump_lat - lat
        new_c_long = Tr_front_bump_long - long
        data_output.update({'rear_bumper_center_pos_long': new_c_long, 'rear_bumper_center_pos_lat': new_c_lat})

        return data_output

    # FOV condition - return value of fov condition(T/F) and the nearest point to host (from points which user chose)
    def default_fov_condition(poly, points, reference_additional_data, multi_target_data):
        lowest_pos_lat = []
        lowest_pos_long = []
        num_of_points = points #all, 1/2/3/4
        counter = 0
        for point in reference_additional_data:
            if 'lat' in point:
                lat_val = reference_additional_data[point]
                long_val = reference_additional_data[point[:-3:] + 'long']

                lowest_pos_lat.append(lat_val)  # lists of positions from host
                lowest_pos_long.append(long_val)

                if isinstance(num_of_points, str) and num_of_points == 'all':
                        if multi_target_evaluation.point_inside_polygon(lat_val, long_val, poly):
                            fov_condition = True
                        else:
                            fov_condition = False
                            break

                elif isinstance(num_of_points, int):
                        if multi_target_evaluation.point_inside_polygon(lat_val, long_val, poly):
                            counter += 1
                            if counter == num_of_points:
                                fov_condition = True
                                break
                            else:
                                fov_condition = False
                        else:
                            fov_condition = False

        return fov_condition, lowest_pos_lat, lowest_pos_long

    #vel condition - return value of velocity condition(T/F)
    def default_vel_condition(vel_lat_name, vel_long_name, velocity_treshold,multi_target_data):
        vel_lat_val = multi_target_data[vel_lat_name]
        vel_long_val = multi_target_data[vel_long_name]
        res_vel_val = math.hypot(vel_lat_val, vel_long_val)
        if res_vel_val >= velocity_treshold:
            vel_condition = True
        else:
            vel_condition = False
        return vel_condition

    #cons_ti condition - return value of cons_ti_condition
    def default_cons_ti_condition(i, ref_obj, lowest_pos_lat, lowest_pos_long, cons_ti_short, cons_ti_long, reference_object_eval_results, multi_target_data):
        lowest_pos_lat = min(lowest_pos_lat)  # the nearest point to host
        lowest_pos_long = min(lowest_pos_long)
        if math.hypot(lowest_pos_lat, lowest_pos_long) < 31:  # check how far the nearest point is
            cons_ti = cons_ti_short
        else:
            cons_ti = cons_ti_long
        try:
            for back_ti in range(cons_ti - 1, -1, -1):
                ti_back_iter = list(multi_target_data.keys())[i - back_ti]
                if reference_object_eval_results[ti_back_iter][ref_obj]['fov_condition'] and \
                    reference_object_eval_results[ti_back_iter][ref_obj]['vel_condition']:
                    cons_ti_condition = True
                    # when oclusion is occured cons ti is reseted
                else:
                    cons_ti_condition = False
                    break
        except:
            cons_ti_condition = False

        return cons_ti_condition

    #calculate closest no conflict tab (unique pred to unique ref)
    def calc_closest_no_conflict_tab_pred_to_ref(self, ti):
        closest_no_conflict_tab = {}  # tab with closest eo to ref with no repeats(unique ref to uniqe eo) (eo main key)
        exclude_tab = []
        rep = 1
        while rep == 1:  # filling closest_no_conflict_tab
            rep = 0
            if len(closest_no_conflict_tab.keys()) < len(self.ref_pred_mah_dist_tab[ti].keys()):  # every pred should have unique ref
                for ref_obj in self.ref_pred_mah_dist_tab[ti]:
                    preds_tab = []
                    for pred_obj in self.ref_pred_mah_dist_tab[ti][ref_obj]:

                        if ref_obj in exclude_tab or pred_obj in exclude_tab:
                            continue

                        mah_dist = self.ref_pred_mah_dist_tab[ti][ref_obj][pred_obj]['mahalanobis_distance']
                        preds_tab.append([mah_dist, pred_obj])

                    try:
                        min_mah, min_pred_obj = min(preds_tab)
                    except:
                        continue
                    try:
                        closest_no_conflict_tab[min_pred_obj].update({min_mah: ref_obj})
                    except:
                        closest_no_conflict_tab[min_pred_obj] = {min_mah: ref_obj}

                for pred_obj in closest_no_conflict_tab:
                    if len(list(closest_no_conflict_tab[pred_obj].keys())) > 1:  # if for one pred we have several references
                        min_dist = min(list(closest_no_conflict_tab[pred_obj].keys()))
                        min_ref = closest_no_conflict_tab[pred_obj][min_dist]
                        closest_no_conflict_tab[pred_obj] = {min_dist: min_ref}
                        exclude_tab.append(min_ref)
                        exclude_tab.append(min_pred_obj)
                        rep = 1
        return closest_no_conflict_tab
    
    def save_binary_class(path, filename, name, data):
        try:
            data = pd.DataFrame.from_dict(data, orient='index')
        except Exception:
            pass
        data.to_pickle(path + '\\' + filename[:-4] + name + '_binary_classification.pkl')