#-----------------------------------------------------------------------------
# Set the default CMAKE_INSTALL_PREFIX variable
# 
macro(slicer3_set_default_install_prefix_for_external_projects)
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(Slicer_INSTALL_PREFIX)
      set(__install_prefix "${Slicer_INSTALL_PREFIX}")
    else(Slicer_INSTALL_PREFIX)
      set(__install_prefix "${Slicer_HOME}")
    endif(Slicer_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${__install_prefix}"
      CACHE PATH "Install path prefix." FORCE)
  endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endmacro(slicer3_set_default_install_prefix_for_external_projects)
