# # import numpy as np

# # # Starting with a 2D NumPy array with a couple of rows
# # data_container = np.array([[10, 20, 30],
# #                            [40, 50, 60]])
# # print("Original NumPy array:")
# # print(data_container)

# # # Suppose row is a new row to add:
# # row = [70, 80, 90]
# # # Make sure row is converted to an array with the same shape (i.e. 1 x 3)
# # row_array = np.array([row])  

# # # Use np.append along axis=0 to add the new row:
# # data_container = np.append(data_container, row_array, axis=0)

# # print("\nNumPy array after using np.append:")
# # print(data_container)

# # import numpy as np

# # # Starting with a 2D array
# # data_container = np.array([[10, 20, 30],
# #                            [40, 50, 60]])
# # print("Original NumPy array:")
# # print(data_container)

# # row = [70, 80, 90]
# # row_array = np.array([row])  # Make sure that row is a 2D array (1 x 3)
# #     i want row like this 
# # ""[[10 20 30]
# #  [40 50 60]
# # ],[[70 80 90]]""


# import numpy as np
# import itertools

# # Create some example numpy arrays
# array1 = np.array([[1, 2, 3], [4, 5, 6]])
# array2 = np.array([[7, 8, 9], [10, 11, 12]])
# array3 = np.array([[13, 14, 15], [16, 17, 18]])

# # Store these arrays in a list, for example
# arrays = [array1, array2, array3]

# # Let’s assume we want to work with a data container that's structured similarly – here a dictionary.
# # For demonstration, we create a dictionary with keys 0, 1, and 2, initializing them with empty arrays
# data_container = {j: np.array([]) for j in range(len(arrays))}

# # Loop through each array and perform some action.
# # For example, suppose we want to iterate through the arrays and store each processed result in the container.

# # Here, I'm using zip_longest to iterate over both arrays and data_container keys
# for idx, arr in itertools.zip_longest(range(len(arrays)), arrays, fillvalue=None):
#     if arr is not None:
#         # Replace the following with your actual operations.
#         # For instance, let's compute the sum of each row as an example operation:
#         processed = np.sum(arr, axis=1)
#         data_container[idx] = processed
#     else:
#         # If there's no corresponding array, we just continue or perform any fallback.
#         continue

# # # If needed, you can also assign directly using the dictionary:
# # data_container[0] = np.array([[1, 2, 3], [3, 4, 5], [2, 3, 4]])
# # data_container[1] = np.array([[3, 4, 5], [2, 3, 4], [4, 5, 6]])
# # data_container[2] = np.array([[2, 3, 4], [5, 6, 7], [8, 9, 10]])

# # Print the final result for demonstration
# for key, value in data_container.items():
#     print(f"data_container[{key}] =\n{value}\n")


for i in range(115):
    if i % 5 == 0:
        print("multi at 5")  # Print for multiples of 5
        continue  # Skip further checks for this iteration
    elif i % 2 == 0:
        print("even")  # Print "even" for other even numbers
    else:
        print(i)  # Print the number if it's odd and not a multiple of 5
