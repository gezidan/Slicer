""" This module loads the Slicer library modules into its namespace."""

__kits_to_load = [
# slicer libs
'freesurfer', 
'mrml', 
'mrmlLogic', 
'remoteio', 
'teem', 
'PythonQt.qMRMLWidgets',
# slicer base libs
'logic',
'PythonQt.qSlicerBaseQTCore',
'PythonQt.qSlicerBaseQTGUI'
]

for kit in __kits_to_load:
   try:
     exec "from %s import *" % (kit)
   except ImportError as detail:
     print detail
   
# Removing things the user shouldn't have to see.
del __kits_to_load
