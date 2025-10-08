'''
Created by jj81t8
'''
import pickle
import pandas as pd
from matplotlib import pyplot as plt
import numpy as np
from numpy import sin, cos
import sys
import os
sys.path.insert(0, r'..\track_generation')
from track_data_generation import tracksDataMaker

path_RT_pickle = r"C:\Users\mj2snl\Desktop\multi_target\RNASUV_SRR5_K0402B_20181115_201_LCZ_TC1_20P_001_rear_association_RTData.pkl"
RT_data = pd.read_pickle(path_RT_pickle)


choice = 'pkl' #you can chose: 'data' or 'pkl' : 'data' return dictionary: Track_data (every time generate new track),
                                                # 'pkl' generate pkl file and return path to this file
drive, path_and_file = os.path.splitdrive(path_RT_pickle)
path, file = os.path.split(path_and_file)
allname, extension = os.path.splitext(file)
name = allname[0:-6] + 'TrackData.pkl'
list_of_files = os.listdir(path)

if choice == 'data':
    Track_data = tracksDataMaker(path_RT_pickle, choice)

elif choice == 'pkl':
    if name in list_of_files:
        Track_data = pd.read_pickle(path + "\\" +name)
    else:
        path_Track_pickle = tracksDataMaker(path_RT_pickle, choice)
        Track_data = pd.read_pickle(path_Track_pickle)

new = False #is you want to create new track (overwriting pkl)
if new == True:
    path_Track_pickle = tracksDataMaker(path_RT_pickle, 'pkl')
    Track_data = pd.read_pickle(path_Track_pickle)

#host
h_length = 5
h_width = 2
h_FL = [-h_width/2, 0]
h_FR = [h_width/2, 0]
h_RR = [h_width/2, -h_length]
h_RL = [-h_width/2, -h_length]
angle = 0
t_rot = np.radians(angle)
R_rot = np.array([[cos(t_rot), sin(t_rot)], [-sin(t_rot), cos(t_rot)]])
h_FL_rot = np.dot(R_rot, h_FL)
h_FR_rot = np.dot(R_rot, h_FR)
h_RR_rot = np.dot(R_rot, h_RR)
h_RL_rot = np.dot(R_rot, h_RL)

h_x = [h_FL_rot[0], h_FR_rot[0], h_RR_rot[0], h_RL_rot[0], h_FL_rot[0]]
h_y = [h_FL_rot[1], h_FR_rot[1], h_RR_rot[1], h_RL_rot[1], h_FL_rot[1]]

side = 'right_sens_rt'
scan_idx = 54260

#target1
t1_FL = RT_data[side][scan_idx]['target1']['front left corner']
t1_FR = RT_data[side][scan_idx]['target1']['front right corner']
t1_RL = RT_data[side][scan_idx]['target1']['rear left corner']
t1_RR = RT_data[side][scan_idx]['target1']['rear right corner']
t1_RBC = RT_data[side][scan_idx]['target1']['rear bumper centre']
#target2
t2_FL = RT_data[side][scan_idx]['target2']['front left corner']
t2_FR = RT_data[side][scan_idx]['target2']['front right corner']
t2_RL = RT_data[side][scan_idx]['target2']['rear left corner']
t2_RR = RT_data[side][scan_idx]['target2']['rear right corner']
t2_RBC = RT_data[side][scan_idx]['target2']['rear bumper centre']
#track1
tr1_FL = Track_data[side][scan_idx]['track1']['front left corner']
tr1_FR = Track_data[side][scan_idx]['track1']['front right corner']
tr1_RL = Track_data[side][scan_idx]['track1']['rear left corner']
tr1_RR = Track_data[side][scan_idx]['track1']['rear right corner']

#track2
tr2_FL = Track_data[side][scan_idx]['track2']['front left corner']
tr2_FR = Track_data[side][scan_idx]['track2']['front right corner']
tr2_RL = Track_data[side][scan_idx]['track2']['rear left corner']
tr2_RR = Track_data[side][scan_idx]['track2']['rear right corner']

fig = plt.figure()
fig.patch.set_facecolor("lightgrey")
graph1 = fig.add_subplot(1, 1, 1)
line1 = graph1.plot(h_x, h_y, color='black',linewidth=0.7)
#graph1.fill_between(h_x, h_y, color='red')
graph1.scatter(0, 0, color='black', s=6)
line2 = graph1.plot([t1_FL['x'], t1_FR['x'], t1_RR['x'], t1_RL['x'], t1_FL['x']], [t1_FL['y'], t1_FR['y'], t1_RR['y'], t1_RL['y'], t1_FL['y']], color="deeppink", linewidth=0.7)
graph1.scatter((t1_FL['x']+t1_FR['x'])/2, (t1_FL['y']+t1_FR['y'])/2, color='deeppink', s=6)
line3 = graph1.plot([t2_FL['x'], t2_FR['x'], t2_RR['x'], t2_RL['x'], t2_FL['x']], [t2_FL['y'], t2_FR['y'], t2_RR['y'], t2_RL['y'], t2_FL['y']], color="deeppink", linewidth=0.7)
graph1.scatter((t2_FL['x']+t2_FR['x'])/2, (t2_FL['y']+t2_FR['y'])/2, color='deeppink', s=6)
line4 = graph1.plot([tr1_FL['x'], tr1_FR['x'], tr1_RR['x'], tr1_RL['x'], tr1_FL['x']], [tr1_FL['y'], tr1_FR['y'], tr1_RR['y'], tr1_RL['y'], tr1_FL['y']], color="deepskyblue", linewidth=0.7)
graph1.scatter((tr1_FL['x']+tr1_FR['x'])/2, (tr1_FL['y']+tr1_FR['y'])/2, color='deepskyblue', s=6)
line5 = graph1.plot([tr2_FL['x'], tr2_FR['x'], tr2_RR['x'], tr2_RL['x'], tr2_FL['x']], [tr2_FL['y'], tr2_FR['y'], tr2_RR['y'], tr2_RL['y'], tr2_FL['y']], color="deepskyblue", linewidth=0.7)
graph1.scatter((tr2_FL['x']+tr2_FR['x'])/2, (tr2_FL['y']+tr2_FR['y'])/2, color='deepskyblue', s=6)
graph1.grid(linestyle='--', alpha=0.5)
plot_center_x = (t1_RBC['x']+t2_RBC['x'])/2
plot_center_y = (t1_RBC['y']+t2_RBC['y'])/2
graph1.set_xlim(plot_center_x-10,plot_center_x + 10)
graph1.set_ylim(plot_center_y-10,plot_center_y + 10)
#graph1.set_facecolor('black')
fig.legend([line1[0], line2[0], line4[0]], ('Host', 'Target', 'Track'))
#plt.show()

plt.savefig(r'C:\Users\mj2snl\wkspaces\ASPE0000_00_Common\a.png',dpi=800)