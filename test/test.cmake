# Fake RPATH for windows and wine
if (WIN32)
    if (CMAKE_CROSSCOMPILING)
        set(ENV{WINEPATH} "$ENV{WINEPATH};${DDS_DIR}")
    else ()
        set(ENV{PATH} "$ENV{PATH};${DDS_DIR}")
    endif ()
endif ()

execute_process(COMMAND ${CMD}
    RESULT_VARIABLE RES)

if (NOT RES EQUAL 0)
    list(JOIN CMD " " COMM)
    message(FATAL_ERROR "Command line: ${COMM}")
endif ()
