
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
target_link_libraries(dtest PRIVATE test_common dds)

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

set(THREAD_MODEL
    default
    none
    stl
    stlasync
    )

if (WIN32)
    list(APPEND THREAD_MODEL winapi)
endif ()

if (OpenMP_FOUND)
    list(APPEND THREAD_MODEL openmp)
endif ()

if (APPLE)
    list(APPEND THREAD_MODEL gcd)
endif ()

if (Boost_FOUND)
    list(APPEND THREAD_MODEL boost)
endif ()

if (TBB_FOUND)
    list(APPEND THREAD_MODEL tbb)
endif ()

if (CMAKE_CXX_STANDARD GREATER 14)
    list(APPEND THREAD_MODEL stlimpl)
endif ()

if (PPL_FOUND)
    list(APPEND THREAD_MODEL pplimpl)
endif ()

# Avoid running test combinations which don't work
set(NO_RUN_solve "sol.*")
set(NO_RUN_play "sol.*")
set(NO_RUN_par "sol.*")
set(NO_RUN_dealerpar "sol.*")

foreach (TV ${TEST_VARIANTS})
    foreach (I ${TEST_DATA})
        if (NOT NO_RUN_${TV} OR NOT I MATCHES "${NO_RUN_${TV}}")
            set(ITXT "${PROJECT_SOURCE_DIR}/hands/${I}.txt")
            foreach (TM ${THREAD_MODEL})
                add_test(NAME ${TV}_${I}_${TM}
                    COMMAND ${CMAKE_COMMAND}
                    "-DDDS_DIR=$<TARGET_FILE_DIR:dds>"
                    "-DWIN32=${WIN32}"
                    "-DCMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}"
                    "-DCMD=$<TARGET_FILE:dtest>;-s;${TV};-f;${ITXT};-t;${TM}"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake
                    )
                set_tests_properties(${TV}_${I}_${TM} PROPERTIES
                    LABELS "${TV};${I};${TM}")
            endforeach ()
        endif ()
    endforeach ()
endforeach ()

set(ctest_args --output-on-failure)

# Default check target runs only reduced test set
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} -L "list100$|sol10$" ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check dtest)

add_custom_target(check_master
    COMMAND ${CMAKE_CTEST_COMMAND} -L "masterDD" ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check_master dtest)

add_custom_target(check_slow
    COMMAND ${CMAKE_CTEST_COMMAND} -L "largest|thomas." ${ctest_args}
    USES_TERMINAL VERBATIM)
add_dependencies(check_slow dtest)
