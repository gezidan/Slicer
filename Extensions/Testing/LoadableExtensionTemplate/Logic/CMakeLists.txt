project(vtkSlicer${EXTENSION_NAME}ModuleLogic)

#-----------------------------------------------------------------------------
set(module_logic_name "${PROJECT_NAME}")

set(module_logic_export_directive "VTK_SLICER_LOADABLEEXTENSIONTEMPLATE_MODULE_LOGIC_EXPORT")

set(module_logic_include_directories
  )

set(module_logic_SRCS
  vtkSlicerLoadableExtensionTemplateLogic.cxx
  vtkSlicerLoadableExtensionTemplateLogic.h
  )

set(module_logic_target_libraries
  ${ITK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
