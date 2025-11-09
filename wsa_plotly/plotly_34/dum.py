# import numpy as np
# import matplotlib.pyplot as plt

# # Define the range of x values
# x = np.linspace(-10, 10, 400)

# # Define the functions
# f1 = x**2 + 1
# f2 = 5*x**3 + 2
# f3 = 45*x

# # Create the plot
# plt.figure(figsize=(10, 6))

# # Plot each function
# plt.plot(x, f1, label='$f_1(x) = x^2 + 1$', color='blue')
# plt.plot(x, f2, label='$f_2(x) = 5x^3 + 2$', color='orange')
# plt.plot(x, f3, label='$f_3(x) = 45x$', color='green')

# # Add labels and title
# plt.title('Plot of Functions in Matrix M')
# plt.xlabel('x')
# plt.ylabel('f(x)')
# plt.axhline(0, color='black',linewidth=0.5, ls='--')
# plt.axvline(0, color='black',linewidth=0.5, ls='--')
# plt.grid(color = 'gray', linestyle = '--', linewidth = 0.5)
# plt.legend()
# plt.ylim(-1000, 1000)  # Adjust y-limits for better visibility

# # Show the plot
# plt.show()

# matrix grah 
# is this 3d

# M= 
# ​
  
# x 
# 2
#  +1
# 5x 
# 3
#  +2
# 45x
# ​
  
# ​



# import numpy as np
# import matplotlib.pyplot as plt

# # Define the range of x values
# x = np.linspace(-10, 10, 400)

# # Define the functions (example functions)
# f1 = x**2 + 1
# f2 = 5*x**3 + 2
# f3 = 45*x

# # Create the plot
# plt.figure(figsize=(10, 6))

# # Plot each function
# plt.plot(x, f1, label='$f_1(x) = x^2 + 1$', color='blue')
# plt.plot(x, f2, label='$f_2(x) = 5x^3 + 2$', color='orange')
# plt.plot(x, f3, label='$f_3(x) = 45x$', color='green')

# # Add labels and title
# plt.title('Plot of Functions Represented in Matrix M')
# plt.xlabel('x')
# plt.ylabel('f(x)')
# plt.axhline(0, color='black', linewidth=0.5, ls='--')
# plt.axvline(0, color='black', linewidth=0.5, ls='--')
# plt.grid(color='gray', linestyle='--', linewidth=0.5)
# plt.legend()
# plt.show()



# import plotly.graph_objects as go
# import numpy as np

# # Define the matrix
# M = np.array([[1, 2],
#               [3, 4],
#               [5, 6]])

# # Extract x, y, z coordinates from the matrix
# x = M[:, 0]  # First column
# y = M[:, 1]  # Second column
# z = [0, 0, 0]  # All points at z=0 for a flat representation

# # Create a scatter plot in 3D
# fig = go.Figure(data=[go.Scatter3d(
#     x=x,
#     y=y,
#     z=z,
#     mode='markers+text',
#     text=['(1,2)', '(3,4)', '(5,6)'], # Labels for each point
#     textposition="top center",
#     marker=dict(size=10)
# )])

# # Set layout options
# fig.update_layout(
#     title='3D Representation of Matrix M',
#     scene=dict(
#         xaxis_title='Column 1',
#         yaxis_title='Column 2',
#         zaxis_title='Z-axis'
#     )
# )

# # Show the plot
# fig.show()


import numpy as np
import plotly.graph_objects as go

# Define the range of x values
x = np.linspace(-10, 10, 100)

# Define the functions
f1 = x**2 + 1
f2 = 5*x**3 + 2
f3 = 45*x
f4 = 5*x

# Create a meshgrid for x and y axes (for surface plotting)
X, Y = np.meshgrid(x, x)

# Calculate Z values for each function based on X and Y
Z1 = X**2 + 1
Z2 = 5*Y**3 + 2
Z3 = 45*Y
Z4 = 5*Y

# Create a figure for the plot
fig = go.Figure()

# Add surfaces for each function
fig.add_trace(go.Surface(z=Z1, x=X, y=Y, colorscale='Blues', opacity=0.5, name='f1: $x^2 + 1$'))
fig.add_trace(go.Surface(z=Z2, x=X, y=Y, colorscale='Oranges', opacity=0.5, name='f2: $5x^3 + 2$'))
fig.add_trace(go.Surface(z=Z3, x=X, y=Y, colorscale='Greens', opacity=0.5, name='f3: $45x$'))
fig.add_trace(go.Surface(z=Z4, x=X, y=Y, colorscale='Reds', opacity=0.5, name='f4: $5x$'))

# Update layout options
fig.update_layout(
    title='3D Representation of Functions',
    scene=dict(
        xaxis_title='X-axis',
        yaxis_title='Y-axis',
        zaxis_title='Z-axis'
    ),
    height=800,
    width=800,
)

# Show the plot
fig.show()

# Calculate gradients for each function at specific points (optional)
gradient_f1 = np.gradient(Z1)
gradient_f2 = np.gradient(Z2)
gradient_f3 = np.gradient(Z3)
gradient_f4 = np.gradient(Z4)

# You can add gradient plots similarly if needed.
