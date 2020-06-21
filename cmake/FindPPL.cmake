
# Very simple find for header only

find_path(PPL_INCLUDE_DIR ppl.h
    HINTS $ENV{PPLDIR}
    PATH_SUFFIXES include
    DOC "The directory of Parallel Patterns Library headers"
    )
mark_as_advanced(PPL_INCLUDE_DIR)

if (PPL_INCLUDE_DIR)
    add_library(PPL::PPL IMPORTED INTERFACE)
    target_include_directories(PPL::PPL INTERFACE ${PPL_INCLUDE_DIR})
endif ()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PPL REQUIRED_VARS PPL_INCLUDE_DIR)
