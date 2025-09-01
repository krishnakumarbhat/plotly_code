import h5py
import numpy as np
from collections import deque
from b_db_layer.data_mapper import stla, stla_nav

class HdfParser:
    @staticmethod
    def bfs_hdf5(input_file):
        """Perform BFS on the HDF5 file to explore its structure."""
        
        ds = {}
        queue = deque([input_file])
        toknowlist = []
        uni_map = {}
        rev_uni_map = {}
        uni = 0
        uni_sub = 0
        parent = {}
        total = 0
        lis= []
        
        def process(name, obj, parent):
            nonlocal uni, uni_sub, total ,lis
            # #print(f"name ..... {name}, obj....{obj}")
                
            if name in stla:
                if isinstance(obj, h5py.Group):
                    print(f"Group found inside stla: {name}")
                
                elif isinstance(obj, h5py.Dataset):
                    if stla[name] == "ScanIndex":
                        lis = obj[:-1]
                        l, r = lis[0], lis[-1]
                        # print(l,r)
                        print(f"here i am ...............dsfasdf {lis}")
                        total = r - l
                        
                        for i in range(len(lis) - 1):
                            if lis[i] + 1 != lis[i + 1]:
                                print(f"Missing scan index: {lis[i] + 1}.")
                        
                        for j in lis:
                            ds[f"ScanIndex{j}"] = np.empty(shape=(0,), dtype=obj.dtype)

                        print(f"Data structure initialized: {ds}")

                    else:
                        print(f"Processing dataset under parent: {parent}, name: {name}")
                        
                        if name in parent:
                            pj = rev_uni_map[stla_nav[parent[name]]]
                            print(f"\n pj ... {pj}")
                            p, k = pj.split('_')
                            print(f"\n p ... {p,lis}")
                            # As obj is a 2D dataset so..
                            # if len(obj) == total:
                            a = 0
                            for j in lis:
                                print(j)
                                print("hi am heree............")
                                print(f"{j}=={a}")
                                ds[f"ScanIndex{j}"] = np.append(ds[f"ScanIndex{j}"], obj[a])
                                a+=1
                            uni_sub += 1
                            uni_map[f"{p}_{uni_sub}"] = stla[name]
                            rev_uni_map[f"{stla[name]}"] = f"{p}_{uni_sub}"
                        else:
                            print("Parent not found; need to add data structure for it.")


            if name in stla_nav:
                print(f"Found name in stla_nav: {name}")
                uni_map[f"{uni}_None"] = stla_nav[name]
                rev_uni_map[f"{stla_nav[name]}"] = f"{uni}_None"
                print(f"here is both the hashmap........{uni_map},.....{rev_uni_map}")
                uni += 1

        while queue:
            current = queue.popleft()

            if isinstance(current, h5py.Group):
                for name, obj in current.items():
                    if name in stla and stla[name] == "Header":
                        process(name, obj, parent)
                        queue.append(obj)
                    if name in stla and stla[name] == "ScanIndex":
                        process(name, obj, parent)
                        queue.append(obj)
                    else:
                        toknowlist.append((name, obj))
                        
                if toknowlist:
                    queue.extend(toknowlist)
                    toknowlist.clear()

                for name, obj in current.items():
                    if (name in stla or name in stla_nav) and (stla.get(name) not in ["Header", "ScanIndex"] and stla_nav.get(name) not in ["Header", "ScanIndex"]):
                        
                        if name in stla_nav:
                            for i in obj:
                                parent[i] = name
                                print(f"parent.....{parent}")

                        process(name, obj, parent)
                        queue.append(obj)

        return ds, uni_map, rev_uni_map
