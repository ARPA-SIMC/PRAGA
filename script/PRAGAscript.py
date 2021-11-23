import sys, os, subprocess, shlex
from subprocess import Popen

env = dict(os.environ)
#env['LD_LIBRARY_PATH'] = '/home/laura/CRITERIA3D/mapGraphics/release/'
args = ['/autofs/nfshomes/lcostantini/PRAGA/bin/PRAGA.AppImage /autofs/nfshomes/lcostantini/PRAGA/script/PRAGAshellCommand']
p = subprocess.Popen(args, shell=True, env=env).communicate()

directory = os.fsencode('/autofs/nfshomes/lcostantini/PRAGA/DATA/PROJECT/')
outputDir = os.fsencode('/autofs/nfshomes/lcostantini/PRAGA/DATA/OUTPUT/')

for file in os.listdir(directory):
     filename = os.fsdecode(file)
     completeFilename = os.fsdecode(directory) + os.fsdecode(file)
     if filename.endswith(".nc"): 
         #print(completeFilename)
         #print(filename)
         variable = filename.split('_')[2]
         print(variable)
         args = ['python3', '/autofs/nfshomes/lcostantini/PRAGA/script/makeGeotiff.py', variable,  completeFilename, outputDir]
         p = subprocess.Popen(args).communicate()
     else:
         continue
