
enable_testing()

set(TEST_SRC
    args.cpp
    args.h
    compare.cpp
    compare.h
    cst.h
    loop.cpp
    loop.h
    parse.cpp
    parse.h
    print.cpp
    print.h
    testcommon.cpp
    testcommon.h
    TestTimer.cpp
    TestTimer.h
    )

add_library(test_common EXCLUDE_FROM_ALL OBJECT ${TEST_SRC})
target_link_libraries(test_common PRIVATE dds)

add_executable(dtest EXCLUDE_FROM_ALL dtest.cpp)
target_link_libraries(dtest PRIVATE test_common dds Threads::Threads)

set(TEST_VARIANTS
    solve
    calc
    play
    par
    dealerpar
    )

set(TEST_DATA
    largest
    list10000
    list1000
    list100
    list10
    list1
    list2
    list300
    masterDD
    sol100000
    sol10
    thomas1
    thomas2
    )

# Avoid running test combinations which don't work
set(NO_RUN_solve "sol.*")
set(NO_RUN_play "sol.*")
set(NO_RUN_par "sol.*")
set(NO_RUN_dealerpar "sol.*")

macro(dds_add_test _NAME _ARGS _LABELS)
    add_test(NAME ${_NAME}
        COMMAND ${CMAKE_COMMAND}
        "-DDDS_DIR=$<TARGET_FILE_DIR:dds>"
        "-DWIN32=${WIN32}"
        "-DCMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}"
        "-DCMD=$<TARGET_FILE:dtest>;${_ARGS}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake
        )
    set_tests_properties(${_NAME} PROPERTIES
        LABELS "${_LABELS}")
endmacro(dds_add_test)

foreach (TV ${TEST_VARIANTS})
    foreach (I ${TEST_DATA})
        if (NOT NO_RUN_${TV} OR NOT I MATCHES "${NO_RUN_${TV}}")
            set(ITXT "${PROJECT_SOURCE_DIR}/hands/${I}.txt")
            dds_add_test(${TV}_${I}
                "-s;${TV};-f;${ITXT}"
                "${TV};${I}")
        endif ()
    endforeach ()
endforeach ()

list(JOIN TEST_VARIANTS ";-s;" ARG_VARIANTS)
set(ARG_DATA -f "${PROJECT_SOURCE_DIR}/hands/list100.txt"
    -f "${PROJECT_SOURCE_DIR}/hands/list10.txt"
    -f "${PROJECT_SOURCE_DIR}/hands/list1.txt"
    )

dds_add_test(parallel_small
    "-s;${ARG_VARIANTS};${ARG_DATA}"
    "parallel_small")

list(APPEND ARG_DATA -f "${PROJECT_SOURCE_DIR}/hands/largest.txt"
    -f "${PROJECT_SOURCE_DIR}/hands/list1000.txt")

dds_add_test(parallel_large
    "-s;${ARG_VARIANTS};${ARG_DATA}"
    "parallel_large"
    )

set(ctest_args --output-on-failure)

# Default check target runs only reduced test set
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} -L "list100$|sol10$|parallel_small" ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check dtest)

add_custom_target(check_master
    COMMAND ${CMAKE_CTEST_COMMAND} -L "masterDD" ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check_master dtest)

add_custom_target(check_slow
    COMMAND ${CMAKE_CTEST_COMMAND} -L "largest|thomas.|parallel_large" ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check_slow dtest)
