#!/usr/bin/env python
# coding: utf-8

# In[19]:


import random


# In[20]:


number_of_file = 10
file_to_be_used = "sample_file_for_test"
division = {'small':0.60, 'med':0.20, 'large':0.20}
size_def = {'zero':0, 'small':4096, 'med':8192, 'large':16384}


# In[21]:


def make_files(prev_key, key, _data):
    for i in range(int(number_of_file*(division[key]))):
        max_size = size_def[key]
        min_size = size_def[prev_key]
        r = random.randint(min_size,max_size)
        _data_to_write = _data[0:r]
        with open('files/{}_{}'.format(key, i), 'w') as wfd:
            wfd.write(_data_to_write)


# In[24]:


# generation of files
with open(file_to_be_used, 'r') as fd:
    _data = fd.read()
    make_files('zero', 'small', _data)
    make_files('small', 'med', _data)
    make_files('med', 'large', _data)


# In[28]:


import glob
files = glob.glob("files/*")


# In[32]:


#generate commands
commands = []

# init commands
commands.append("format\n")
commands.append("mount\n")

# create inodes
for i, _file in enumerate(files):
    commands.append("create\n")
    commands.append("copyin {} {}\n".format(_file, i))

# cat data from files
for i in range(50):
    r = random.randint(0,9)
    commands.append("cat {}\n".format(r))

# exchange data
for i in range(30):
    r1 = random.randint(0,9)
    r2 = r1
    while(r2 == r1):
        r2 = random.randint(0,9)
    commands.append("copyout {} {}\n".format(r1, "temp1"))
    commands.append("copyout {} {}\n".format(r2, "temp2"))
    commands.append("copyin {} {}\n".format("temp2", r1))
    commands.append("copyin {} {}\n".format("temp1", r2))

# unmount
commands.append("quit\n")

with open("commands", 'w') as wfd:
    wfd.writelines(commands)


# In[ ]:




