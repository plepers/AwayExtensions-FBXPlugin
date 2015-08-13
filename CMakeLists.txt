cmake_minimum_required (VERSION 2.6)



project (FbxAwdExporter)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")



# FBX plugin constants.
set (FbxAwdExporter_VERSION_MAJOR 0)
set (FbxAwdExporter_VERSION_MINOR 1)
set (FbxAwdExporter_PLUGIN_EXTENSION "awd")
set (FbxAwdExporter_PLUGIN_DESCRIPTION "Away3D AWD (*.awd)")
set (FbxAwdExporter_PLUGIN_NAME "Awd_Writer_Reader")



# find FBX_SDK
FIND_PACKAGE(FBX REQUIRED)
ADD_DEFINITIONS(-DFBX_VERSION_MAJOR=${FBX_VERSION_MAJOR})

# find FBX_Extensions_SDK
# FIND_PACKAGE(FBXExt)

# find AWD cpp library
find_path( AWD_INCLUDE_DIRS awd )
find_library( AWD_LIBRARIES awd_static )



message(STATUS "AWD_INC = ${AWD_INCLUDE_DIRS}")
message(STATUS "AWD_LIB = ${AWD_LIBRARIES}")



# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/include/ExporterConfig.h.in"
  "${PROJECT_BINARY_DIR}/include/ExporterConfig.h"
)

set( FBX2AWD_SOURCES
  src/main.cxx
  src/Common.cxx
  include/Common.h
  include/Thumbnail.h
)


# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories( SYSTEM ${FBX_INCLUDE_DIRS} )
include_directories( SYSTEM ${AWD_INCLUDE_DIRS} )
include_directories("${PROJECT_SOURCE_DIR}/AwdWriterReader")
include_directories("${PROJECT_SOURCE_DIR}/submodules/tclap/include")

set( LINK_LIBRARIES ${FBX_LIBRARIES} ${AWD_LIBRARIES} )

add_subdirectory (AwdWriterReader)

add_executable(fbx2awd ${FBX2AWD_SOURCES})

TARGET_LINK_LIBRARIES ( fbx2awd AwdWriterReader_static )
TARGET_LINK_LIBRARIES ( fbx2awd ${LINK_LIBRARIES} )

#
# Tests
#

enable_testing()
add_test( test fbx2awd /Applications/Autodesk/FBX\ SDK/2015.1/samples/UVSample/Sadface.FBX )