# Locate the FBX Extensions SDK (version 2015.1 only atm)
#
# Defines the following variables:
#
#    FBX_EXT_FOUND - Found the FBX SDK
#    FBX_EXT_VERSION - Version number
#    FBX_EXT_INCLUDE_DIRS - Include directories
#
# Accepts the following variables as input:
#
#    FBX_EXT_VERSION - as a CMake variable, e.g. 2014.1
#    FBX_EXT_ROOT - (as a CMake or environment variable)
#               The root directory of the FBX Extensions SDK install

if(NOT FBX_EXT_VERSION)
    set(FBX_EXT_VERSION 2015.1)
endif()
string(REGEX REPLACE "^([0-9]+).*$" "\\1" FBX_EXT_VERSION_MAJOR "${FBX_EXT_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*$" "\\1" FBX_EXT_VERSION_MINOR  "${FBX_EXT_VERSION}")
string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" FBX_EXT_VERSION_PATCH "${FBX_EXT_VERSION}")

set(FBX_EXT_MAC_LOCATIONS
    "/Applications/Autodesk/FBX\ Extensions\ SDK/${FBX_VERSION}"
)

if(WIN32)
    string(REGEX REPLACE "\\\\" "/" WIN_PROGRAM_FILES_X64_DIRECTORY $ENV{ProgramW6432})
endif()

set(FBX_EXT_WIN_LOCATIONS
    "${WIN_PROGRAM_FILES_X64_DIRECTORY}/Autodesk/FBX/FBX Extensions SDK/${FBX_EXT_VERSION}"
)

set(FBX_EXT_SEARCH_LOCATIONS
    $ENV{FBX_EXT_ROOT} ${FBX_EXT_ROOT} ${FBX_EXT_MAC_LOCATIONS} ${FBX_EXT_WIN_LOCATIONS}
)

function(_fbx_find_library _name _lib)
    find_library(${_name}
        NAMES ${_lib}
        HINTS ${FBX_EXT_SEARCH_LOCATIONS}
        PATH_SUFFIXES lib/vs2013 lib/vs2012 lib/vs2010 lib/vs2008
    )
    mark_as_advanced(${_name})
endfunction()


find_path(FBX_EXT_INCLUDE_DIR fbxsdk.h
    PATHS ${FBX_EXT_SEARCH_LOCATIONS}
    PATH_SUFFIXES include
)
mark_as_advanced(FBX_EXT_INCLUDE_DIR)

if(WIN32)
    _fbx_find_library(FBX_EXT_LIBRARY            fbxmax_amd64 )
elseif(APPLE)
    
endif()


include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(FBX_EXT DEFAULT_MSG FBX_EXT_LIBRARY FBX_EXT_INCLUDE_DIR)

if(FBX_EXT_FOUND)
    set(FBX_EXT_INCLUDE_DIRS ${FBX_EXT_INCLUDE_DIR})
    add_definitions (-DHAS_FBX_EXT)
    if(WIN32)
        set(FBX_EXT_LIBRARIES ${FBX_EXT_LIBRARY} Wininet.lib)
    elseif(APPLE)
        set(FBX_EXT_LIBRARIES ${FBX_EXT_LIBRARY} ${CARBON} ${SYSTEM_CONFIGURATION})
    endif()
endif()

