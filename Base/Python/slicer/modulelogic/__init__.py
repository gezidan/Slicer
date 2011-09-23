""" This module loads the Slicer Module Logic vtk classes into its namespace."""

# Import the CLI logic
from qSlicerBaseQTCLIPython import vtkSlicerCLIModuleLogic

from __main__ import _qSlicerCoreApplicationInstance as app
from slicer.util import importVTKClassesFromDirectory

# HACK Ideally constant from vtkSlicerConfigure and vtkSlicerVersionConfigure should
#      be wrapped.
slicer_qt_loadable_modules_lib_subdir =  "/lib/Slicer-%d.%d/qt-loadable-modules" % (app.majorVersion, app.minorVersion)
directory = app.slicerHome + slicer_qt_loadable_modules_lib_subdir + "/Python"
importVTKClassesFromDirectory(directory, __name__, filematch = "vtkSlicer*ModuleLogic.py")

# Removing things the user shouldn't have to see.
del app, importVTKClassesFromDirectory, directory, slicer_qt_loadable_modules_lib_subdir
