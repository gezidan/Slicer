
#-----------------------------------------------------------------------------
# Extract dashboard option passed from command line
#-----------------------------------------------------------------------------
# Note: The syntax to pass option from the command line while invoking ctest is
#       the following: ctest -S /path/to/script.cmake,OPTNAME1##OPTVALUE1^^OPTNAME2##OPTVALUE2
#
# Example:
#       ctest -S /path/to/script.cmake,SCRIPT_MODE##continuous^^GIT_TAG##next
#
if(NOT CTEST_SCRIPT_ARG STREQUAL "")
  string(REPLACE "^^" "\\;" CTEST_SCRIPT_ARG_AS_LIST "${CTEST_SCRIPT_ARG}")
  set(CTEST_SCRIPT_ARG_AS_LIST ${CTEST_SCRIPT_ARG_AS_LIST})
  foreach(argn_argv ${CTEST_SCRIPT_ARG_AS_LIST})
    string(REPLACE "##" "\\;" argn_argv_list ${argn_argv})
    set(argn_argv_list ${argn_argv_list})
    list(GET argn_argv_list 0 argn)
    list(GET argn_argv_list 1 argv)
    set(${argn} ${argv})
  endforeach()
endif()

#-----------------------------------------------------------------------------
# Macro allowing to set a variable to its default value only if not already defined
macro(setIfNotDefined var defaultvalue)
  if(NOT DEFINED ${var})
    set(${var} "${defaultvalue}")
  endif()
endmacro()

#-----------------------------------------------------------------------------
# Set build configuration
if(NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_CFG_INTDIR})
else()
  set(CTEST_BUILD_CONFIGURATION ${CMAKE_BUILD_TYPE})
endif()

#-----------------------------------------------------------------------------
# Sanity checks
set(expected_defined_vars EXTENSION_NAME EXTENSION_SOURCE_DIR EXTENSION_BINARY_DIR CTEST_BUILD_CONFIGURATION CTEST_CMAKE_GENERATOR Slicer_CMAKE_DIR Slicer_DIR Slicer_WC_REVISION  EXTENSION_BUILD_OPTIONS_STRING RUN_CTEST_SUBMIT RUN_CTEST_UPLOAD)
foreach(var ${expected_defined_vars})
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is not defined !")
  endif()
endforeach()

include(${Slicer_CMAKE_DIR}/SlicerFunctionCTestPackage.cmake)

#-----------------------------------------------------------------------------
# Set site name
site_name(CTEST_SITE)
# Force to lower case
string(TOLOWER "${CTEST_SITE}" CTEST_SITE)

# Set build name
set(CTEST_BUILD_NAME "${Slicer_WC_REVISION}-${EXTENSION_NAME}-${EXTENSION_COMPILER}-${EXTENSION_BUILD_OPTIONS_STRING}-${CTEST_BUILD_CONFIGURATION}")

# The following variable can be used while testing the script
set(run_ctest_with_configure TRUE)
set(run_ctest_with_build TRUE)
set(run_ctest_with_test TRUE)
set(run_ctest_with_packages TRUE)
# See also RUN_CTEST_SUBMIT in SlicerBlockUploadExtension.cmake

setIfNotDefined(CTEST_PARALLEL_LEVEL 8)
setIfNotDefined(CTEST_MODEL "Experimental")

set(label ${EXTENSION_NAME})
set_property(GLOBAL PROPERTY SubProject ${label})
set_property(GLOBAL PROPERTY Label ${label})

set(track "Extensions-${CTEST_MODEL}")
ctest_start(${CTEST_MODEL} TRACK ${track} ${EXTENSION_SOURCE_DIR} ${EXTENSION_BINARY_DIR})
ctest_read_custom_files("${EXTENSION_BINARY_DIR}")

set(cmakecache_content
"#Generated by SlicerBlockBuildPackageAndUploadExtension.cmake
CMAKE_BUILD_TYPE:STRING=${CTEST_BUILD_CONFIGURATION}
Slicer_DIR:PATH=${Slicer_DIR}
")

#-----------------------------------------------------------------------------
# Write CMakeCache.txt only if required
set(cmakecache_current "")
if(EXISTS ${EXTENSION_BINARY_DIR}/CMakeCache.txt)
  file(READ ${EXTENSION_BINARY_DIR}/CMakeCache.txt cmakecache_current)
endif()
if(NOT ${cmakecache_content} STREQUAL "${cmakecache_current}")
  file(WRITE ${EXTENSION_BINARY_DIR}/CMakeCache.txt ${cmakecache_content})
endif()

#-----------------------------------------------------------------------------
# Configure extension
if(run_ctest_with_configure)
  #message("----------- [ Configuring extension ${EXTENSION_NAME} ] -----------")
  ctest_configure(
    BUILD ${EXTENSION_BINARY_DIR}
    SOURCE ${EXTENSION_SOURCE_DIR}
    )
  if(RUN_CTEST_SUBMIT)
    ctest_submit(PARTS Configure)
  endif()
endif()

#-----------------------------------------------------------------------------
# Build extension
set(build_errors)
if(run_ctest_with_build)
  #message("----------- [ Building extension ${EXTENSION_NAME} ] -----------")
  ctest_build(BUILD ${EXTENSION_BINARY_DIR} NUMBER_ERRORS build_errors APPEND)
  if(RUN_CTEST_SUBMIT)
    ctest_submit(PARTS Build)
  endif()
endif()

#-----------------------------------------------------------------------------
# Test extension
if(run_ctest_with_test)
  #message("----------- [ Testing extension ${EXTENSION_NAME} ] -----------")
  # Check if there are tests to run
  execute_process(COMMAND ${CMAKE_CTEST_COMMAND} -N
    WORKING_DIRECTORY ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  string(REGEX REPLACE ".*Total Tests: ([0-9]+)" "\\1" test_count "${output}")
  if("${test_count}" GREATER 0)
    ctest_test(
        BUILD ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}
        PARALLEL_LEVEL ${CTEST_PARALLEL_LEVEL})
    if(RUN_CTEST_SUBMIT)
      ctest_submit(PARTS Test)
    endif()
  endif()
endif()

#-----------------------------------------------------------------------------
# Package extension
if(run_ctest_with_packages)
  if(build_errors GREATER "0")
    message(WARNING "Skip extension packaging: ${build_errors} build error(s) occured !")
  else()
    #message("----------- [ Packaging extension ${EXTENSION_NAME} ] -----------")
    message("Packaging extension ${EXTENSION_NAME} ...")
    set(extension_packages)
    SlicerFunctionCTestPackage(
      BINARY_DIR ${EXTENSION_BINARY_DIR}
      CONFIG ${CTEST_BUILD_CONFIGURATION}
      RETURN_VAR extension_packages)
    if(RUN_CTEST_UPLOAD AND COMMAND ctest_upload)
      message("Uploading extension ${EXTENSION_NAME} ...")
      foreach(p ${extension_packages})
        ctest_upload(FILES ${p})
        if(RUN_CTEST_SUBMIT)
          ctest_submit(PARTS Upload)
        endif()
      endforeach()
    endif()
  endif()
endif()

