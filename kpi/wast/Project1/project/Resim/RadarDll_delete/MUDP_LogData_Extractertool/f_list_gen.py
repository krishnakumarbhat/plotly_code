'''
f_list.txt for csv conversion
'''
import os
import sys

if len(sys.argv) > 1:
    org_dir = sys.argv[1]
else:
    org_dir = input('Enter log path: ')
org = [i for i in os.listdir(org_dir) if i.endswith('.mf4')]
all_dirs = [i for i in os.listdir(org_dir) if os.path.isdir(os.path.join(org_dir,i))] # all foldersresim_dirs = [j for i in all_dirs for j in os.listdir('{}\{}'.format(org_dir,i)) if j.endswith('.mf4') ]
resim_dirs = [j for i in all_dirs for j in os.listdir(os.path.join(org_dir,i)) if j.endswith('.mf4') ] # resim folders
resim_log_dirs = [os.path.join(i.replace('.mf4',''),j) for i in resim_dirs for j  in os.listdir(os.path.join(org_dir,i.replace('.mf4',''))) if j.endswith('.mf4') ]


with open('f_list.txt','w') as f:
    for i in org+resim_log_dirs:
        f.write(os.path.join(org_dir,i))
        f.write('\n')

