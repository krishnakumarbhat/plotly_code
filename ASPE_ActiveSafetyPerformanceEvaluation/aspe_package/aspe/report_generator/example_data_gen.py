import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def get_simple_long_text():
    return """
            Lorem Ipsum is simply dummy text of the printing and typesetting industry. 
            Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, 
            when an unknown printer took a galley of type and scrambled it to make a type specimen book. 
            It has survived not only five centuries, but also the leap into electronic typesetting, 
            remaining essentially unchanged.
            Lorem Ipsum is simply dummy text of the printing and typesetting industry. 
            Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, 
            when an unknown printer took a galley of type and scrambled it to make a type specimen book. 
            It has survived not only five centuries, but also the leap asdas
            """


def get_simple_short_text():
    return """
            Lorem Ipsum is simply dummy text of the printing and typesetting industry. 
            It has been the industry's standard dummy text ever since the 1500s, 
            when an unknown printer took a galley of type and scrambled it to make a type specimen book. 
            """


def get_simple_df_data():
    mix_data = np.array([
        [1, 2, 3, 4],
        [1, 2, 3, 4],
        [1, 2, 3, 4],
        [1, 2, 3, 4],
        [1, 2, 3, 4],
        [1, 2, 3, 4],
    ])

    columns = ['a', 'b', 'c', 'd']

    return pd.DataFrame(mix_data, columns=columns)


def get_simple_exp_fig():
    fig1, ax1 = plt.subplots()
    eg_data = get_simple_df_data()
    eg_data.plot(ax=ax1)
    plt.close()
    return fig1
