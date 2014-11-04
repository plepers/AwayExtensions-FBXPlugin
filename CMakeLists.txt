cmake_minimum_required (VERSION 2.6)



project (FbxAwdExporter)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")



# The version number.
set (FbxAwdExporter_VERSION_MAJOR 1)
set (FbxAwdExporter_VERSION_MINOR 0)

# find FBX_SDK
FIND_PACKAGE(FBX REQUIRED)
ADD_DEFINITIONS(-DFBX_VERSION_MAJOR=${FBX_VERSION_MAJOR})

# find AWD cpp library
find_path( AWD_INCLUDE_DIRS awd )
find_library( AWD_LIBRARIES libawd.a awd )



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

set( LINK_LIBRARIES ${FBX_LIBRARIES} ${AWD_LIBRARIES} )

add_subdirectory (AwdWriterReader)

add_executable(fbx2awd ${FBX2AWD_SOURCES})

TARGET_LINK_LIBRARIES ( fbx2awd AwdWriterReader_static )
TARGET_LINK_LIBRARIES ( fbx2awd ${LINK_LIBRARIES} )

#
# Tests
#