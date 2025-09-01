# import numpy as np
# import pandas as pd
# from sklearn.cluster import KMeans
# import plotly.express as px

# # Simulate data: 50 scan indices, 60 detections per scan, 2 features (range, range_rate)
# np.random.seed(0)
# num_si = 50
# num_det = 60

# # Simulated input and output data
# input_range = np.random.uniform(0, 100, size=(num_si, num_det))
# input_range_rate = np.random.uniform(-20, 20, size=(num_si, num_det))
# output_range = input_range + np.random.normal(0, 2, size=(num_si, num_det))
# output_range_rate = input_range_rate + np.random.normal(0, 1, size=(num_si, num_det))

# # Prepare DataFrame for all scan indices
# records = []
# for si in range(num_si):
#     # Stack features
#     input_features = np.stack([input_range[si], input_range_rate[si]], axis=1)
#     output_features = np.stack([output_range[si], output_range_rate[si]], axis=1)
#     # Cluster each
#     kmeans_in = KMeans(n_clusters=3, random_state=0).fit(input_features)
#     kmeans_out = KMeans(n_clusters=3, random_state=0).fit(output_features)
#     # Append input
#     for det in range(num_det):
#         records.append({
#             'range': input_range[si, det],
#             'range_rate': input_range_rate[si, det],
#             'scan_index': si,
#             'type': 'input',
#             'cluster': int(kmeans_in.labels_[det])
#         })
#         records.append({
#             'range': output_range[si, det],
#             'range_rate': output_range_rate[si, det],
#             'scan_index': si,
#             'type': 'output',
#             'cluster': int(kmeans_out.labels_[det])
#         })

# df = pd.DataFrame(records)

# # 3D scatter plot: range, range_rate, scan_index, colored by type, symbol by type
# fig = px.scatter_3d(
#     df, x='range', y='range_rate', z='scan_index',
#     color='type', symbol='type', opacity=0.7,
#     title="3D Scatter Plot of Detections (Range, Range Rate, Scan Index)",
#     labels={'range': 'Range', 'range_rate': 'Range Rate', 'scan_index': 'Scan Index'}
# )
# fig.show()


# import time

# def task1():
#     time.sleep(1)
#     return "Task 1 done"

# def task2():
#     time.sleep(1)
#     return "Task 2 done"

# def task3():
#     time.sleep(1)
#     return "Task 3 done"


# import threading
# import time

# def wrapper(func):
#     def inner():
#         func()
#     return inner

# start = time.perf_counter()

# threads = [
#     threading.Thread(target=wrapper(task1)),
#     threading.Thread(target=wrapper(task2)),
#     threading.Thread(target=wrapper(task3)),
# ]

# for t in threads:
#     t.start()
# for t in threads:
#     t.join()

# end = time.perf_counter()
# print(f"Threading execution time: {end - start:.2f} seconds")


# import time
# from multiprocessing import Process


# if __name__ == "__main__":
#     start = time.perf_counter()

#     processes = [
#         Process(target=task1),
#         Process(target=task2),
#         Process(target=task3),
#     ]

#     for p in processes:
#         p.start()
#     for p in processes:
#         p.join()

#     end = time.perf_counter()
#     print(f"Multiprocessing execution time: {end - start:.2f} seconds")

# import asyncio

# async def async_task1():
#     await asyncio.sleep(1)
#     return "Task 1 done"

# async def async_task2():
#     await asyncio.sleep(1)
#     return "Task 2 done"

# async def async_task3():
#     await asyncio.sleep(1)
#     return "Task 3 done"

# async def main():
#     start = time.perf_counter()
#     await asyncio.gather(
#         async_task1(),
#         async_task2(),
#         async_task3(),
#     )
#     end = time.perf_counter()
#     print(f"AsyncIO execution time: {end - start:.2f} seconds")

# asyncio.run(main())

