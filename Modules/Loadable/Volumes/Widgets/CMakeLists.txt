project(qSlicer${qt_module_name}ModuleWidgets)

#
# See CMake/SlicerMacroBuildModuleWidgets.cmake for details
#

set(module_widgets_name "${PROJECT_NAME}")

# The header '${module_widgets_name}Export.h' will be automatically configured.
set(module_widgets_export_directive "Q_SLICER_QTMODULES_VOLUMES_WIDGETS_EXPORT")

# Additional directories to include
set(module_widgets_include_directories
  )

# Source files
set(module_widgets_SRCS
  qSlicerDiffusionTensorVolumeDisplayWidget.cxx
  qSlicerDiffusionTensorVolumeDisplayWidget.h
  qSlicerDiffusionWeightedVolumeDisplayWidget.cxx
  qSlicerDiffusionWeightedVolumeDisplayWidget.h
  qSlicerLabelMapVolumeDisplayWidget.cxx
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.cxx
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumeDisplayWidget.cxx
  qSlicerVolumeDisplayWidget.h
  qSlicerDTISliceDisplayWidget.cxx
  qSlicerDTISliceDisplayWidget.h
  )

# Headers that should run through moc
set(module_widgets_MOC_SRCS
  qSlicerDiffusionTensorVolumeDisplayWidget.h
  qSlicerDiffusionWeightedVolumeDisplayWidget.h
  qSlicerLabelMapVolumeDisplayWidget.h
  qSlicerScalarVolumeDisplayWidget.h
  qSlicerVolumeDisplayWidget.h
  qSlicerDTISliceDisplayWidget.h
  )

# UI files
set(module_widgets_UI_SRCS
  Resources/UI/qSlicerDiffusionTensorVolumeDisplayWidget.ui
  Resources/UI/qSlicerDiffusionWeightedVolumeDisplayWidget.ui
  Resources/UI/qSlicerDTISliceDisplayWidget.ui
  Resources/UI/qSlicerLabelMapVolumeDisplayWidget.ui
  Resources/UI/qSlicerScalarVolumeDisplayWidget.ui
  )

# Resources
set(module_widgets_resources
  Resources/qSlicer${qt_module_name}WidgetsModule.qrc
  )

# Additional Target libraries
set(module_widgets_target_libraries
  )

SlicerMacroBuildModuleWidgets(
  NAME ${module_widgets_name}
  EXPORT_DIRECTIVE ${module_widgets_export_directive}
  INCLUDE_DIRECTORIES ${module_widgets_include_directories}
  SRCS ${module_widgets_SRCS}
  MOC_SRCS ${module_widgets_MOC_SRCS}
  UI_SRCS ${module_widgets_UI_SRCS}
  TARGET_LIBRARIES ${module_widgets_target_libraries}
  RESOURCES ${module_widgets_resources}
  )

# Qt Designer Plugins
#add_subdirectory(Plugins)

# Testing
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

