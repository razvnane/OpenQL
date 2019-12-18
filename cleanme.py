# script to clean temporary files generated by installation
import os
import shutil

dirs = ['build', 'cbuild', 'dist', 'openql.egg-info',
		'swig/qutechtools.egg-info', 'swig/openql.egg-info']
files = ['']

for dir in dirs:
    try:
        shutil.rmtree(dir)
    except OSError:
        pass

for file in files:
    try:
        os.remove(file)
    except OSError:
        pass
