
# Very simple find for header only

find_path(PPL_INCLUDE_DIR ppl.h
    HINTS $ENV{PPLDIR}
    PATH_SUFFIXES include
    DOC "The directory of Parallel Patterns Library headers"
    )
mark_as_advanced(PPL_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(PPL REQUIRED_VARS PPL_INCLUDE_DIR)
