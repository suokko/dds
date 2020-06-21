
# Very simple find for header only

find_path(GCD_INCLUDE_DIR dispatch/dispatch.h
    HINTS $ENV{PPLDIR}
    PATH_SUFFIXES include
    DOC "The directory of Parallel Patterns Library headers"
    )
mark_as_advanced(GCD_INCLUDE_DIR)

if (GCD_INCLUDE_DIR)
    add_library(GCD::GCD IMPORTED INTERFACE)
    target_include_directories(GCD::GCD INTERFACE ${GCD_INCLUDE_DIR})
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GCD REQUIRED_VARS GCD_INCLUDE_DIR)
