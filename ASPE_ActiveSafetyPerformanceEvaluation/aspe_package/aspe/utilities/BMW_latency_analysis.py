import matplotlib.pyplot as plt

from aspe.utilities.SupportingFunctions import load_from_pkl

path = r'C:\logs\BYK-632_DEX-696\DFT_361_analysis\last_run.pickle'
data = load_from_pkl(path)

fig, ax = plt.subplots(nrows=2, sharex=True)

for key, value in data.items():
    ts = value.index.to_numpy()
    center_y = value.center_y.to_numpy()
    speed = value.speed.to_numpy()

    ax[0].plot(ts, center_y, label=key)
    ax[1].plot(ts, speed, label=key)
ax[0].set_title('BBox center lateral position')
ax[1].set_title('Speed')

ax[0].set_ylabel('position lateral [m]')
ax[1].set_ylabel('speed [m/s]')

ax[1].set_xlabel('time [s]')

for a in ax:
    a.grid()
    a.get_xaxis().get_major_formatter().set_useOffset(False)
    a.legend()
