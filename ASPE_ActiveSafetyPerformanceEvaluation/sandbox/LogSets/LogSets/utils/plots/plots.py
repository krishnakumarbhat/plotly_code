import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import pandas as pd
from LogSets.utils.pca.pca import get_princ_comps
from random import randint
import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import seaborn as sns
from sklearn.preprocessing import StandardScaler
from sklearn.decomposition import PCA
import warnings
from LogSets.utils.file_handling import load

warnings.filterwarnings("ignore", module="matplotlib")


def plot3d_dataset(full_set, custom_set=None, save_plot=True, path=None, plot_title=None,
                   fullset_color='black', subset_color='orangered', clusters=None):

    if clusters is not None:
        colors = []
        n_clusters = np.unique(clusters).shape[0]
        for i in range(n_clusters):
            colors.append('#' + '%06X' % randint(0, 0xFFFFFF))
    if plot_title is None:
        plot_title = 'Data set'
    if save_plot:
        if path is None:
            raise ValueError('Please provide save path (path=)')
    if custom_set is None:
        custom_set = pd.DataFrame(columns=['path'])
    plt.style.use('seaborn-white')

    names = full_set['path']
    full_set_num = full_set.drop('path', axis=1)
    full_set_num = StandardScaler().fit_transform(full_set_num)
    full_set_pca = get_princ_comps(full_set_num, n_components=3, method='kernel_pca')

    fig = plt.figure(figsize=(16, 8))
    plt.clf()
    ax = Axes3D(fig, rect=(0, 0, 1, 1), elev=20, azim=90)
    plt.cla()
    features = full_set_pca * 100

    targets = names.to_list()
    for i, name in enumerate(targets):
        if name in custom_set['path'].to_list():
            if clusters is not None:
                color = colors[clusters.iloc[i, 0]]
                size = full_set.shape[0] // 10
            else:
                color = subset_color
                size = full_set.shape[0] // 30
            alpha = 1
            marker = 'X'
            # marker = "$" + str(i) + "$"

        else:
            if clusters is not None:
                color = colors[clusters.iloc[i, 0]]
                alpha = 0.1
                size = full_set.shape[0] // 10
            else:
                color = fullset_color
                alpha = 0.5
                size = full_set.shape[0] // 40
            marker = 'o'
        ax.scatter(features[i, 0], features[i, 1], features[i, 2], c=color, s=size, cmap="hsv", alpha=alpha, marker=marker)

    ax.set_xlabel("PC1")
    ax.set_ylabel("PC2")
    ax.set_zlabel("PC3")
    full = mpatches.Patch(color=fullset_color, label='FullSet')
    subset = mpatches.Patch(color=subset_color, label='Subset')
    ax.legend(handles=[full, subset])
    ax.set_title(plot_title)

    if save_plot:
        plt.savefig(fname=path.replace('.pickle', '.png'), dpi=600)
    plt.show()


def plot2d_dataset(full_set, custom_set=None, save_plot=True, path=None, plot_title=None,
                   fullset_color='black', subset_color='orangered'):
    if plot_title is None:
        plot_title = 'Data set'
    if save_plot:
        if path is None:
            raise ValueError('Please provide save path (path=)')
    if custom_set is None:
        custom_set = pd.DataFrame(columns=['path'])
    plt.style.use('tableau-colorblind10')
    names = full_set['path']
    full_set_num = full_set.drop('path', axis=1)
    full_set_num = StandardScaler().fit_transform(full_set_num)
    full_set_pca = get_princ_comps(full_set_num, n_components=2, method='kernel_pca')
    plt.clf()
    plt.cla()
    fig, ax = plt.subplots()
    features = full_set_pca

    targets = names.to_list()
    for i, name in enumerate(targets):
        if name in custom_set['path'].to_list():
            color = subset_color
            size = full_set.shape[0] // 30
        else:
            color = fullset_color
            size = full_set.shape[0] // 50
        ax.scatter(features[i, 0], features[i, 1], c=color, s=size)

    ax.set_xlabel("PC1")
    ax.set_ylabel("PC2")
    full = mpatches.Patch(color=fullset_color, label='FullSet')
    subset = mpatches.Patch(color=subset_color, label='Subset')
    ax.legend(handles=[full, subset])
    ax.set_title(plot_title)

    if save_plot:
        plt.savefig(fname=path.replace('.pickle', '.png'), dpi=300)
    plt.show()
    print()


def dists_plot():
    plt.style.use('seaborn-white')
    # full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\ad_shared_demo\ad_shared_demo.pickle').drop('path', axis=1)
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_test.pickle').drop('path',
                                                                                                               axis=1)
    # alpha_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\ad_shared_demo\ad_shared_demo.pickle').drop('path', axis=1)
    alpha_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\AlphaSets\alpha_set_25.pickle').drop('path', axis=1)
    golden_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\GoldenSets\golden_set_100.pickle').drop('path',
                                                                                                          axis=1)
    # visual prob dist plots analysis
    col = 132
    from scipy.stats import norm
    fig = plt.figure(figsize=(20, 4))
    ax = sns.distplot(full_set.iloc[:, col], hist=False,label='full')
    ax1 = sns.distplot(golden_set.iloc[:, col], hist=False, label='golden - 100 logs')
    ax2 = sns.distplot(alpha_set.iloc[:, col],hist=False, label='alpha - 25 logs')

    ax.set_title(full_set.columns[col])
    plt.show()


def pca_plots():
    plt.style.use('seaborn-white')
    full_set = load(r'\\10.224.186.68\AD-Shared\F360\Tools\LogSets\FullSet\fullset_test.pickle')
    logs_paths = full_set.path
    full_set = full_set.drop('path', axis=1)
    x = StandardScaler().fit_transform(full_set)
    cum_explained_variance_ratio = []
    components = []
    for n_components in range(1, 40):
        columns = []
        for c in range(n_components):
            columns.append('PC' + str(c + 1))

        pca = PCA(n_components=n_components)
        principalComponents = pca.fit_transform(x)
        principalDf = pd.DataFrame(data=principalComponents, columns=columns)

        ratio = pca.explained_variance_ratio_
        explained_variance_ratio_df = pd.DataFrame(pca.explained_variance_ratio_, index=principalDf.columns).T
        cum_explained_variance_ratio.append(sum(pca.explained_variance_ratio_))
        components.append(n_components)
    fig = plt.figure(figsize=(20, 4))
    plt.plot(components, cum_explained_variance_ratio, 'o', color='orange')
    plt.show()


if __name__ == '__main__':
    # pca_plots()
    dists_plot()
