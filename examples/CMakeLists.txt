# Example usage in windows:
# mkdir build
# cmake -DCMAKE_PREFIX_PATH=../../../../ ..
# make
# ./SolveBoard.exe
#
# Unix installation to paths like /usr or /usr/local can skip setting CMAKE_PREFIX_PATH

cmake_minimum_required(VERSION 3.12)

project(dds_examples LANGUAGES CXX)

# Exclude examples from ALL target when building in source tree
set(EFA EXCLUDE_FROM_ALL)
# Allow build dds_examples standard alone
if (NOT TARGET dds)
    find_package(DDS REQUIRED)
    # Add all examples to ALL target
    set(EFA)
endif ()

add_library(hands EXCLUDE_FROM_ALL ${DDS_LIBTYPE}
    hands.cpp
    hands.h
    )
include(GenerateExportHeader)
generate_export_header(hands)
target_link_libraries(hands PUBLIC dds)
target_include_directories(hands PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

set(EXAMPLE_SRC
    AnalyseAllPlaysBin.cpp
    AnalyseAllPlaysPBN.cpp
    AnalysePlayBin.cpp
    AnalysePlayPBN.cpp
    CalcAllTables.cpp
    CalcAllTablesPBN.cpp
    CalcDDtable.cpp
    CalcDDtablePBN.cpp
    DealerPar.cpp
    Par.cpp
    SolveAllBoards.cpp
    SolveBoard.cpp
    SolveBoardPBN.cpp
    )

add_custom_target(examples
    COMMENT "Build all examples")

foreach(SRC ${EXAMPLE_SRC})
    string(REPLACE ".cpp" "" BIN ${SRC})
    add_executable(${BIN} ${EFA} ${SRC})
    target_link_libraries(${BIN} PRIVATE hands)
    add_dependencies(examples ${BIN})
endforeach()

install(FILES ${EXAMPLE_SRC} hands.cpp hands.h CMakeLists.txt
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
    COMPONENT Docs)

