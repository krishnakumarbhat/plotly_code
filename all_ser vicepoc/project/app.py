from flask import Flask, render_template
import plotly.graph_objs as go
import plotly.io as pio
import pandas as pd
import numpy as np
import os

app = Flask(__name__, template_folder='templates')


@app.route('/')
def index():
    # sample data
    x = np.linspace(0, 10, 200)
    y = np.sin(x)

    fig1 = go.Figure(go.Scatter(x=x, y=y, mode='lines', name='sin(x)'))
    fig1.update_layout(title='Sine Wave')
    plot1 = pio.to_html(fig1, full_html=False, include_plotlyjs='cdn')

    df = pd.DataFrame({'x': x, 'y': np.cos(x)})
    # downsample for bar chart
    fig2 = go.Figure(go.Bar(x=df['x'][::10], y=df['y'][::10]))
    fig2.update_layout(title='Cosine Samples')
    # second plot uses the same Plotly JS already included above
    plot2 = pio.to_html(fig2, full_html=False, include_plotlyjs=False)

    return render_template('index.html', plot1=plot1, plot2=plot2)


if __name__ == '__main__':
    host = os.environ.get('FLASK_HOST', '0.0.0.0')
    port = int(os.environ.get('FLASK_PORT', '5000'))
    # Listen on 0.0.0.0 so the app is reachable from other hosts on the network
    app.run(host=host, port=port)
