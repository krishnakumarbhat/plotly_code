import pickle

data_path = r'private\point_x4_00_y4_00_vx4_00_vy_00.pickle'
with open(data_path, 'rb') as handle:
    sim_out = pickle.load(handle)

