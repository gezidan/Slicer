################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by
#   Dave Partyka and Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#-----------------------------------------------------------------------------
# Git protocole option
#-----------------------------------------------------------------------------

option(Slicer_USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)

set(git_protocol "git")
if(NOT Slicer_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Qt - Let's check if a valid version of Qt is available
#-----------------------------------------------------------------------------
INCLUDE(SlicerBlockFindQtAndCheckVersion)

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------

INCLUDE(ExternalProject)

set(ep_base        "${CMAKE_BINARY_DIR}")
#set(ep_install_dir "${ep_base}/Install")

SET(ep_common_args
  #-DCMAKE_INSTALL_PREFIX:PATH=${ep_install_dir}
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DBUILD_TESTING:BOOL=OFF
  )

SET(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
SET(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")
  
# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#------------------------------------------------------------------------------
# Establish Target Dependencies based on Selected Options
#------------------------------------------------------------------------------

set(tcl_DEPENDENCIES)
set(tk_DEPENDENCIES tcl)
set(incrTcl_DEPENDENCIES tcl tk)
set(iwidgets_DEPENDENCIES tcl itcl)
set(python_DEPENDENCIES)
if(Slicer_USE_PYTHONQT_WITH_TCL)
  if(WIN32)
    set(python_DEPENDENCIES tcl)
  else()
    set(python_DEPENDENCIES tk)
  endif()
endif()
set(CLAPACK_DEPENDENCIES)
set(NUMPY_DEPENDENCIES CLAPACK python)
set(scipy_DEPENDENCIES NUMPY)
set(weave_DEPENDENCIES python NUMPY)

set(VTK_DEPENDENCIES)
set(CTKAPPLAUNCHER_DEPENDENCIES)
set(CTK_DEPENDENCIES VTK Insight)
if(Slicer_USE_PYTHONQT)
  list(APPEND CTK_DEPENDENCIES python)
endif()
set(Insight_DEPENDENCIES)

if(Slicer_USE_BatchMake)
  set(BatchMake_DEPENDENCIES Insight)
  list(APPEND slicer_DEPENDENCIES BatchMake)
endif()

set(OpenIGTLink_DEPENDENCIES)
set(teem_DEPENDENCIES VTK)
set(cmcurl_DEPENDENCIES)
set(libarchive_DEPENDENCIES)
set(slicer_DEPENDENCIES VTK Insight OpenIGTLink teem cmcurl libarchive)

#------------------------------------------------------------------------------
# Conditionnaly include ExternalProject Target
#------------------------------------------------------------------------------

if(Slicer_USE_PYTHONQT_WITH_TCL)
  include(SuperBuild/External_Tcl.cmake)
  include(SuperBuild/External_Tk.cmake)
  include(SuperBuild/External_incrTcl.cmake)
endif()

if(Slicer_USE_PYTHONQT)
  include(SuperBuild/External_Python26.cmake)
  if(Slicer_USE_NUMPY)
    include(SuperBuild/External_CLAPACK.cmake)
    include(SuperBuild/External_NUMPY.cmake)
    if(Slicer_USE_WEAVE)
      include(SuperBuild/External_weave.cmake)
    endif()
    if(Slicer_USE_SCIPY)
      include(SuperBuild/External_SciPy.cmake)
    endif()
  endif()
endif()

include(SuperBuild/External_VTK.cmake)
include(SuperBuild/External_Insight.cmake)
include(SuperBuild/External_CTK.cmake)
if (Slicer_USE_CTKAPPLAUNCHER)
  include(SuperBuild/External_CTKAPPLAUNCHER.cmake)
endif()
include(SuperBuild/External_teem.cmake)
include(SuperBuild/External_OpenIGTLink.cmake)
if(Slicer_USE_BatchMake)
  include(SuperBuild/External_BatchMake.cmake)
endif()
include(SuperBuild/External_cmcurl.cmake)
include(SuperBuild/External_libarchive.cmake)

#-----------------------------------------------------------------------------
# Update external project dependencies
#------------------------------------------------------------------------------

if (Slicer_USE_PYTHONQT_WITH_TCL)
  if(UNIX)
    list(APPEND slicer_DEPENDENCIES itcl)
  endif()
endif()

list(APPEND slicer_DEPENDENCIES CTK)
if (Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND slicer_DEPENDENCIES CTKAPPLAUNCHER)
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND slicer_DEPENDENCIES python)
  if(Slicer_USE_NUMPY)
    list(APPEND slicer_DEPENDENCIES NUMPY)
    if(Slicer_USE_SCIPY)
      list(APPEND slicer_DEPENDENCIES scipy)
    endif()
  endif()
endif()

#------------------------------------------------------------------------------
# List of external projects
#------------------------------------------------------------------------------  

#set(external_project_list tk tcl incrTcl iwidgets python weave CLAPACK NUMPY scipy VTK CTK  Insight BatchMake OpenIGTLink teem cmcurl slicer)

#-----------------------------------------------------------------------------
# Dump external project dependencies
#------------------------------------------------------------------------------

#set(ep_dependency_graph "# External project dependencies")
#foreach(ep ${external_project_list})
#  set(ep_dependency_graph "${ep_dependency_graph}\n${ep}:${${ep}_DEPENDENCIES}")
#endforeach()
#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjectDependencies.txt "${ep_dependency_graph}\n")

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#

SET(slicer_cmake_boolean_args
  DOCUMENTATION_TARGET_IN_ALL
  BUILD_TESTING
  BUILD_SHARED_LIBS
  WITH_COVERAGE
  WITH_MEMCHECK
  Slicer_BUILD_CLI
  Slicer_BUILD_QTLOADABLEMODULES
  Slicer_BUILD_QTSCRIPTEDMODULES
  Slicer_USE_PYTHONQT
  Slicer_USE_PYTHONQT_WITH_TCL
  Slicer_USE_CTKAPPLAUNCHER
  Slicer_USE_BatchMake
  Slicer_USE_MIDAS
  Slicer_USE_OPENIGTLINK
  Slicer_USE_OPENCV
  Slicer_WITH_LIBRARY_VERSION
  Slicer_USE_NUMPY
  #Slicer_USE_WEAVE
  )
  
SET(slicer_superbuild_boolean_args)
FOREACH(slicer_cmake_arg ${slicer_cmake_boolean_args})
  LIST(APPEND slicer_superbuild_boolean_args -D${slicer_cmake_arg}:BOOL=${${slicer_cmake_arg}})
ENDFOREACH()

# MESSAGE("CMake args:")
# FOREACH(arg ${slicer_superbuild_boolean_args})
#   MESSAGE("  ${arg}")
# ENDFOREACH()
  
#-----------------------------------------------------------------------------
# Configure and build Slicer
#------------------------------------------------------------------------------

set(proj Slicer)
ExternalProject_Add(${proj}
  DEPENDS ${slicer_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR Slicer-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${slicer_superbuild_boolean_args}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -DSlicer_REQUIRED_C_FLAGS:STRING=${Slicer_REQUIRED_C_FLAGS}
    -DSlicer_REQUIRED_CXX_FLAGS:STRING=${Slicer_REQUIRED_CXX_FLAGS}
    -DWITH_COVERAGE:BOOL=${WITH_COVERAGE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DSlicer_SUPERBUILD:BOOL=OFF
    -DCTEST_CONFIGURATION_TYPE:STRING=${CTEST_CONFIGURATION_TYPE}
    -DDOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
    -DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}
    # ITK
    -DITK_DIR:PATH=${ITK_DIR}
    # Teem
    -DTeem_DIR:PATH=${Teem_DIR}
    # OpenIGTLink
    -DOpenIGTLink_DIR:PATH=${OpenIGTLink_DIR}
    # BatchMake
    -DBatchMake_DIR:PATH=${BatchMake_DIR}
    # VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer_USE_VTK_DEBUG_LEAKS}
    # TCL/Tk
    -DINCR_TCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DINCR_TK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DSlicer_TCL_DIR:PATH=${tcl_build}
    # cmcurl
    -DSLICERLIBCURL_DIR:PATH=${SLICERLIBCURL_DIR}
    # libarchive
    -DLibArchive_DIR:PATH=${LIBARCHIVE_DIR}
    -DLibArchive_INCLUDE_DIR:PATH=${LIBARCHIVE_INCLUDE_DIR}
    -DLibArchive_LIBRARY:PATH=${LIBARCHIVE_LIBRARY}
    # Python
    -DSlicer_USE_SYSTEM_PYTHON:BOOL=OFF
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    # Qt
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    # CTK
    -DCTK_DIR:PATH=${CTK_DIR}
    # CTKAppLauncher
    -DCTKAPPLAUNCHER_DIR:PATH=${CTKAPPLAUNCHER_DIR}
  INSTALL_COMMAND ""
  )
  
#-----------------------------------------------------------------------------
# Slicer extensions
#-----------------------------------------------------------------------------
IF(Slicer_BUILD_EXTENSIONS)
  ADD_SUBDIRECTORY(Extensions)
ENDIF()
