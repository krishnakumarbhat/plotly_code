import numpy as np
import pandas as pd
from io import BytesIO
import base64
import plotly.graph_objects as go

# try:
import datashader as ds
import datashader.transfer_functions as tf
from datashader.colors import inferno, viridis, Greys9
    # DATASHADER_AVAILABLE = True
# except ImportError:
#     DATASHADER_AVAILABLE = False
    # print("Datashader not available. Using Plotly Scattergl instead.")


def render_datashader_plot(df, x_column, y_column, width=800, height=500, 
                         cmap=None, x_range=None, y_range=None, categorical=False,
                         plot_width=None, plot_height=None):
    """
    Render a datashader plot for memory-efficient visualization of large datasets.
    
    Args:
        df: Pandas DataFrame containing the data
        x_column: Name of the column to use for x-axis
        y_column: Name of the column to use for y-axis
        width: Width of the resulting image in pixels
        height: Height of the resulting image in pixels
        cmap: Color map to use (default: inferno)
        x_range: Tuple of (min, max) for x-axis range
        y_range: Tuple of (min, max) for y-axis range
        categorical: Whether the plot represents categorical data
        plot_width: Output width for the plot (if different from render width)
        plot_height: Output height for the plot (if different from render height)
        
    Returns:
        Plotly figure with the datashader rendering
    """
    # if not DATASHADER_AVAILABLE:
    #     return None
    
    # Set default colormap if none provided
    if cmap is None:
        cmap = inferno
    
    # Set up the canvas
    plot_width = plot_width or width
    plot_height = plot_height or height
    
    # Determine ranges if not provided
    if x_range is None:
        x_range = (df[x_column].min(), df[x_column].max())
    if y_range is None:
        y_range = (df[y_column].min(), df[y_column].max())
    
    # Ensure ranges have some width
    x_range = (x_range[0], x_range[1]) if x_range[0] != x_range[1] else (x_range[0]-0.5, x_range[1]+0.5)
    y_range = (y_range[0], y_range[1]) if y_range[0] != y_range[1] else (y_range[0]-0.5, y_range[1]+0.5)
    
    # Create the canvas
    canvas = ds.Canvas(plot_width, plot_height, x_range=x_range, y_range=y_range)
    
    # Aggregate the data
    if categorical:
        agg = canvas.points(df, x_column, y_column, ds.count_cat('category'))
        img = tf.shade(agg, color_key=cmap, how='eq_hist')
    else:
        agg = canvas.points(df, x_column, y_column)
        img = tf.shade(agg, cmap=cmap, how='eq_hist')
    
    # Convert the image to a base64 string
    img_io = BytesIO()
    img.to_pil().save(img_io, format='PNG')
    img_io.seek(0)
    img_base64 = base64.b64encode(img_io.read()).decode('ascii')
    
    # Create a plotly figure with the image
    fig = go.Figure()
    
    # Add the image
    fig.add_layout_image(
        dict(
            source=f'data:image/png;base64,{img_base64}',
            x=x_range[0],
            y=y_range[1],
            sizex=x_range[1] - x_range[0],
            sizey=y_range[1] - y_range[0],
            sizing="stretch",
            opacity=1.0,
            layer="below"
        )
    )
    
    # Set the axes ranges
    fig.update_xaxes(range=x_range)
    fig.update_yaxes(range=y_range)
    
    return fig


def create_datashader_scatter(x_data, y_data, categories=None, width=800, height=500, 
                            colormap=None, title=None, x_label=None, y_label=None):
    """
    Create a datashader scatter plot without downsampling.
    
    Args:
        x_data: Array or list of x values
        y_data: Array or list of y values
        categories: Optional categories/groups for the points
        width: Width of the plot in pixels
        height: Height of the plot in pixels
        colormap: Colormap to use
        title: Title for the plot
        x_label: Label for x-axis
        y_label: Label for y-axis
        
    Returns:
        Plotly figure with datashader rendering
    """
    # if not DATASHADER_AVAILABLE:
        # Fall back to scattergl
    fig = go.Figure()
    fig.add_trace(go.Scattergl(
        x=x_data,
        y=y_data,
        mode='markers',
        marker=dict(
            color='blue',
            size=5,
            opacity=0.7
        )
    ))
    
    if title:
        fig.update_layout(title=title)
    if x_label:
        fig.update_layout(xaxis_title=x_label)
    if y_label:
        fig.update_layout(yaxis_title=y_label)
        
    return fig

    # Create DataFrame from the data
    if categories is not None:
        df = pd.DataFrame({
            'x': x_data,
            'y': y_data,
            'category': categories
        })
        fig = render_datashader_plot(df, 'x', 'y', width, height, 
                                   cmap=colormap, categorical=True)
    else:
        df = pd.DataFrame({
            'x': x_data,
            'y': y_data
        })
        fig = render_datashader_plot(df, 'x', 'y', width, height, cmap=colormap)
    
    # Update layout
    if title:
        fig.update_layout(title=title)
    if x_label:
        fig.update_layout(xaxis_title=x_label)
    if y_label:
        fig.update_layout(yaxis_title=y_label)
        
    # Add hover functionality
    fig.update_layout(hovermode='closest')
    
    return fig


def create_datashader_categorized_scatter(data_dict, width=800, height=500, 
                                        title=None, x_label=None, y_label=None):
    """
    Create a datashader scatter plot for categorized data without downsampling.
    
    Args:
        data_dict: Dictionary of category: {x: [...], y: [...]} pairs
        width: Width of the plot in pixels
        height: Height of the plot in pixels
        title: Title for the plot
        x_label: Label for x-axis
        y_label: Label for y-axis
        
    Returns:
        Plotly figure with layered datashader renderings
    """
    # if not DATASHADER_AVAILABLE:
    # Fall back to scattergl
    fig = go.Figure()
    
    for category, values in data_dict.items():
        if not values.get('x') or not values.get('y'):
            continue
            
        fig.add_trace(go.Scattergl(
            x=values['x'],
            y=values['y'],
            mode='markers',
            name=category,
            marker=dict(
                size=5,
                opacity=0.7
            )
        ))
    
    if title:
        fig.update_layout(title=title)
    if x_label:
        fig.update_layout(xaxis_title=x_label)
    if y_label:
        fig.update_layout(yaxis_title=y_label)
        
    return fig

    # Create a master DataFrame with a category column
    x_all = []
    y_all = []
    cat_all = []
    
    for category, values in data_dict.items():
        if not values.get('x') or not values.get('y'):
            continue
            
        x_all.extend(values['x'])
        y_all.extend(values['y'])
        cat_all.extend([category] * len(values['x']))
    
    if not x_all:
        # No data to plot
        fig = go.Figure()
        fig.update_layout(
            title="No data to display" if not title else title,
            annotations=[{
                'text': 'No data available for visualization',
                'showarrow': False,
                'font': {'size': 16}
            }]
        )
        return fig
    
    df = pd.DataFrame({
        'x': x_all,
        'y': y_all,
        'category': cat_all
    })
    
    # Create the plot
    fig = render_datashader_plot(df, 'x', 'y', width, height, 
                               cmap={cat: color for cat, color in zip(
                                   data_dict.keys(), 
                                   ['blue', 'red', 'green', 'orange', 'purple'][:len(data_dict)]
                               )},
                               categorical=True)
    
    # Update layout
    if title:
        fig.update_layout(title=title)
    if x_label:
        fig.update_layout(xaxis_title=x_label)
    if y_label:
        fig.update_layout(yaxis_title=y_label)
        
    # Add hover functionality
    fig.update_layout(hovermode='closest')
    
    return fig
