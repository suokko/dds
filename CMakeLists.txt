cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Select C++ standard. Default is C++11
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif ()
# Allow user to configure standard version
set(CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CACHE STRING "C++ standard version to use (11, 14 or 17)")
mark_as_advanced(CMAKE_CXX_STANDARD)

# Require support for the standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable extensions
set(CMAKE_CXX_EXTENSIONS OFF)

# Make Release build default initial build type
set(CMAKE_BUILD_TYPE_INIT Release)

set(DESCR "Double Dummy Solver library.")
set(PURL "http://privat.bahnhof.se/wb758135/")

# Setup project
cmake_policy(SET CMP0048 NEW)
project(dds
    VERSION 2.9.0
    DESCRIPTION ${DESCR}
    HOMEPAGE_URL ${PURL}
    LANGUAGES CXX)

# Make all symbols internal by default
cmake_policy(SET CMP0063 NEW)
set(CMAKE_CXX_VISIBILITY_PRESET internal)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Include GNU standard install location configuration
# User can override installation locations using configuration variables
include(GNUInstallDirs)
# Add cmake support for export shared object symbols
include(GenerateExportHeader)
# Add cmake support for link time optimization
cmake_policy(SET CMP0069 NEW)
include(CheckIPOSupported)

check_ipo_supported(RESULT LTO_SUPPORTED LANGUAGES CXX)

if (LTO_SUPPORTED)
    message(STATUS "Link time optimization supported.")
else (LTO_SUPPORTED)
    message(WARNING "CMAKE doesn't support link time optimization for your compiler.")
endif (LTO_SUPPORTED)

# Helpers to check compiler type in generator expressions
set(gcc_like_cxx "$<OR:$<CXX_COMPILER_ID:ARMClang>,\
$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>,\
$<CXX_COMPILER_ID:GNU>>")
set(msvc_cxx "$<CXX_COMPILER_ID:MSVC>")

# Setup compiler warning flags
set(DDS_GCC_WARN
    -Wshadow
    -Wpedantic
    -Wall
    -Wextra
    -Wcast-align
    -Wcast-qual
    -Wctor-dtor-privacy
    -Wdisabled-optimization
    -Winit-self
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wcomment
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-promo
    -Wstrict-overflow=1
    -Wswitch-default
    -Wundef
    -Werror
    CACHE STRING "Warning options used for GCC like compilers"
    )
mark_as_advanced(DDS_GCC_WARN)

set(DDS_MSVC_WARN
    /Wall
    /wd4127
    /wd4514
    /wd4555
    /wd4668
    /wd4701
    /wd4711
    /wd4820
    /wd4986
    /wd4987
    /wd4996
    /WX
    CACHE STRING "Warning options used for MSVC compiler"
    )
mark_as_advanced(DDS_MSVC_WARN)

# Add warning options
add_compile_options(
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:${DDS_GCC_WARN}>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:${DDS_MSVC_WARN}>>"
    )

# User configuration to select shared or static library
option(BUILD_SHARED_LIBRARY "Build shared library" ON)

# Find out if we can support different dds threading models
find_package(Threads)

# Helper variable to select user configured library type
if (BUILD_SHARED_LIBRARY)
    set(DDS_LIBTYPE SHARED)
else ()
    set(DDS_LIBTYPE STATIC)
endif ()

# Handle all subdirectories
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(examples)
add_subdirectory(test)

include(CMakePackageConfigHelpers)
# Generate cmake find_package helpers
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DDSConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DDS"
    )
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DDSConfigVersion.cmake"
    VERSION ${DDS_VERSION}
    COMPATIBILITY SameMajorVersion
    )

# Install all files required by find_package(DDS)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/DDSConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/DDSConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DDS"
    COMPONENT Development
    )

# Add Packaging support
#Configure packaging
file(STRINGS "${CMAKE_SOURCE_DIR}/.gitignore" GITIGNORE)
list(REMOVE_ITEM GITIGNORE "SourceVersion.cmake")
#Convert glob to regular expression
string(REPLACE "." "\\\\." GITIGNORE "${GITIGNORE}")
string(REPLACE "*" ".*" GITIGNORE "${GITIGNORE}")
list(APPEND GITIGNORE "\\\\.git/")
set(CPACK_SOURCE_IGNORE_FILES "${GITIGNORE}")
set(CPACK_PACKAGE_VENDOR "DDS-bridge project")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})

set(CPACK_SOURCE_GENERATOR TGZ ZIP)
if (WIN32)
    set(CPACK_GENERATOR NSIS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR DEB RPM)
else ()
    message(WARNING "No package configuration setup for your system.")
    message(WARNING "Do you know how to setup CPack configuration?")
endif ()

#Default contact from environment variables
set(CPACK_PACKAGE_CONTACT "$ENV{NAME} <$ENV{EMAIL}>")

# DEB packaging configuration
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# RPM packaging configuration
set(CPACK_RPM_MAIN_COMPONENT Runtime)
set(CPACK_RPM_FILENAME RPM-DEFAULT)
set(CPACK_RPM_DOCS_PACKAGE_ARCHITECTURE noarch)

# Windows NSIS installer configuration
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL OFF)
set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_HOMEPAGE_URL}")

set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT Runtime_Libs)
include(InstallRequiredSystemLibraries)

include(CPack)

cpack_add_component(Runtime
    DISPLAY_NAME Library
    DESCRIPTION "The DDS library"
    )
cpack_add_component(Development
    DISPLAY_NAME Headers
    DESCRIPTION "Development files required to compile applications using the DDS library"
    DEPENDS Runtime
    )
cpack_add_component(Docs
    DISPLAY_NAME Documentation
    DESCRIPTION "Documentation and examples how to use the DDS library"
    )
