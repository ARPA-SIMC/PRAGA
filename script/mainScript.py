import sys, os, subprocess, shlex
from subprocess import Popen

env = dict(os.environ)
env['LD_LIBRARY_PATH'] = '/home/laura/CRITERIA3D/mapGraphics/release/'
#args = ['/home/laura/CRITERIA3D/bin/PRAGA/PRAGA /home/laura/CRITERIA3D/script/command']
args = ['/home/laura/CRITERIA3D/bin/PRAGA/PRAGA.AppImage /home/laura/CRITERIA3D/script/command_radicchio']
p = subprocess.Popen(args, shell=True, env=env).communicate()

directory = os.fsencode('/home/laura/CRITERIA3D/DATA/PROJECT/')

for file in os.listdir(directory):
     filename = os.fsdecode(file)
     completeFilename = os.fsdecode(directory) + os.fsdecode(file)
     if filename.endswith(".nc"): 
         #print(completeFilename)
         #print(filename)
         variable = filename.split('_')[2]
         print(variable)
         args = ['python3', '/home/laura/CRITERIA3D/script/prova.py', variable,  completeFilename]
         p = subprocess.Popen(args).communicate()
     else:
         continue
