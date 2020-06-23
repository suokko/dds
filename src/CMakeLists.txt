

set(DDS_IDIR ../include)

# All sources including headers. Headers are listed to allow IDEs list them
# along source files.
set(DDS_SRC
    ABsearch.cpp
    ABsearch.h
    ABstats.cpp
    ABstats.h
    CalcTables.cpp
    CalcTables.h
    dds.cpp
    dds.h
    DealerPar.cpp
    debug.h
    dump.cpp
    dump.h
    File.cpp
    File.h
    Init.cpp
    Init.h
    LaterTricks.cpp
    LaterTricks.h
    Memory.cpp
    Memory.h
    Moves.cpp
    Moves.h
    parallel.h
    Par.cpp
    PBN.cpp
    PBN.h
    PlayAnalyser.cpp
    PlayAnalyser.h
    QuickTricks.cpp
    QuickTricks.h
    Scheduler.cpp
    Scheduler.h
    SolveBoard.cpp
    SolveBoard.h
    SolverIF.cpp
    SolverIF.h
    System.cpp
    System.h
    ThreadMgr.cpp
    ThreadMgr.h
    Timer.cpp
    TimerGroup.cpp
    TimerGroup.h
    Timer.h
    TimerList.cpp
    TimerList.h
    TimeStat.cpp
    TimeStat.h
    TimeStatList.cpp
    TimeStatList.h
    TransTable.h
    TransTableL.cpp
    TransTableL.h
    TransTableS.cpp
    TransTableS.h
    ${DDS_IDIR}/portab.h
    )

# List headers which are part of public interface. They are installed to <prefix>/include/dds
set(DDS_PUBLIC_HEADERS
    ${DDS_IDIR}/dll.h
    ${CMAKE_CURRENT_BINARY_DIR}/dds_export.h
    )

if (WIN32)
    enable_language(RC)
    # Add windows only export locations and dll information resource
    list(APPEND DDS_SRC
        dds.rc
        Exports.def
        )
endif ()

# Add dds target with all sources
add_library(dds ${DDS_LIBTYPE} ${DDS_SRC} ${DDS_PUBLIC_HEADERS})

# Rename Mac library to so like it used to be with Makefiles
if (APPLE AND NOT DDS_LIBRARY_SUFFIX)
    set(DDS_LIBRARY_SUFFIX ".so")
    message(STATUS "Using library suffix '.so' for Mac. \
You can change the suffix to '.dylib' with DDS_LIBRARY_SUFFIX configuration \
variable.")
endif ()

set(DDS_LIBRARY_SUFFIX ${DDS_LIBRARY_SUFFIX} CACHE STRING
    "Configuration option to override system default library suffix.")

if (DDS_LIBRARY_SUFFIX)
    set_target_properties(dds PROPERTIES SUFFIX ${DDS_LIBRARY_SUFFIX})
endif ()

# Helper function to setup user debug configuration objects
function(user_config_define NAME DOCSTR)
    set(PREFIX)
    list(LENGTH ARGN LEN)
    if (LEN GREATER 0)
        list(GET ARGN 0 PREFIX)
    endif ()
    option(${NAME} ${DOCSTR} OFF)
    mark_as_advanced(${NAME})
    if (PREFIX)
        set(${NAME}_PREFIX "${PREFIX}" CACHE STRING
            "Prefix for ${NAME} debug output")
        mark_as_advanced(${NAME}_PREFIX)
    endif ()
endfunction(user_config_define)

# List all configuration objects which are configured into srd/debug.h.in
set(DDS_DEBUG_SUFFIX ".txt" CACHE STRING "Suffix for all debug output files")
mark_as_advanced(DDS_DEBUG_SUFFIX)
user_config_define(DDS_DEBUG_ALL "Enable all debug output")
user_config_define(DDS_AB_DETAILS "Gives more detail, in combination with DDS_AB_STATS")
user_config_define(DDS_AB_HITS "Gives information on nodes stored and retrieved from TT memory.")
set(DDS_AB_HITS_RETRIEVED_PREFIX "retrieved" CACHE STRING "Prefix for DDS_AB_HITS_RETRIEVED debug output.")
mark_as_advanced(DDS_AB_HITS_RETRIEVED_PREFIX)
set(DDS_AB_HITS_STORED_PREFIX "stored" CACHE STRING "Prefix for DDS_AB_HITS_STORED debug output.")
mark_as_advanced(DDS_AB_HITS_STORED_PREFIX)
user_config_define(DDS_AB_STATS
    "Enables AB statistics, node counts etc."
    "ABstats")
user_config_define(DDS_MEMORY_LEAKS_WIN32
    "Debug memory leaks on MSVC.")
user_config_define(DDS_MOVES
    "Enables statistics on move generation quality."
    "movestats")
user_config_define(DDS_MOVES_DETAILS
    "Detailed statistics for DDS_MOVES.")
user_config_define(DDS_SCHEDULER
    "Enables timing in the scheduler."
    "sched")
user_config_define(DDS_TIMING
    "Enables timing of the AB search and related functions."
    "timer")
user_config_define(DDS_TIMING_DETAILS
    "Detailed statistics for DDS_TIMING.")
user_config_define(DDS_TOP_LEVEL
    "Enables data about each call to the top-level AB routine."
    "toplevel")
user_config_define(DDS_TT_STATS
    "Gives statistics on the usage of TT memory."
    "TTstats")

# Generate debug.h from debug.h.in and cmake variables
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/debug.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/debug.h
    @ONLY)

# Generate export dds_export.h
generate_export_header(dds
    EXPORT_MACRO_NAME DLLEXPORT
    NO_EXPORT_MACRO_NAME DLLNOEXPORT
    DEPRECATED_MACRO_NAME DLLDEPRECATED
    )

# Make sure install knows about headers required to use libdds
set_target_properties(dds PROPERTIES PUBLIC_HEADER "${DDS_PUBLIC_HEADERS}")
# Make sure user can find header locations after find_package
target_include_directories(dds PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include;${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/dds>"
    )

# Let cmake handle shared object versioning
set_target_properties(dds PROPERTIES
    SOVERSION ${dds_VERSION_MAJOR}
    VERSION ${dds_VERSION}
    )

# Make sure we use link time optimization if it is supported
if (LTO_SUPPORTED)
    set_target_properties(dds PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# Add WINAPI threading support on windows
if (WIN32)
    target_compile_definitions(dds PRIVATE DDS_THREADS_WINAPI)
endif ()
# Add GCD threading support on Mac OS X
if (GCD_FOUND)
    # If it is standard library then there is no need to detect presence
    target_compile_definitions(dds PRIVATE DDS_THREADS_GCD)
    target_link_libraries(dds PRIVATE GCD::GCD)
endif ()

# Add TBB threading if configure found the library
if (TBB_FOUND)
    target_compile_definitions(dds PRIVATE DDS_THREADS_TBB)
    target_link_libraries(dds PRIVATE TBB::tbb)
endif ()

# Add PPL threading if configure found the header
if (PPL_FOUND)
    target_compile_definitions(dds PRIVATE DDS_THREADS_PPLIMPL)
    target_link_libraries(dds PRIVATE PPL::PPL)
endif ()

# Always enable stl threads as c++11 is required
target_link_libraries(dds PRIVATE Threads::Threads)
target_compile_definitions(dds PRIVATE DDS_THREADS_STL)
target_compile_definitions(dds PRIVATE DDS_THREADS_STLASYNC)

# If C++ standard is 17 or higher then STLIMPL is available
if (CMAKE_CXX_STANDARD GREATER 14)
    target_compile_definitions(dds PRIVATE DDS_THREADS_STLIMPL)
endif ()

if (Boost_FOUND)
    # Add boost threading if libraries were found
    target_link_libraries(dds PRIVATE Boost::thread)
    target_compile_definitions(dds PRIVATE DDS_THREADS_BOOST)
endif ()

if (OpenMP_FOUND)
    # Add OpenMP if compilers supports it
    target_compile_definitions(dds PRIVATE DDS_THREADS_OPENMP)
    target_link_libraries(dds PRIVATE OpenMP::OpenMP_CXX)
endif ()

# Install the library and related development files
# It also generates DDSTargets.cmake helper for find_package(DDS)
install(TARGETS dds
    EXPORT DDSTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        COMPONENT Runtime
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT Runtime
        NAMELINK_COMPONENT Development
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        COMPONENT Development
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/dds"
        COMPONENT Development
    )

# Install cmake helpers for find_package
install(EXPORT DDSTargets
    FILE DDSTargets.cmake
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/DDS"
    COMPONENT Development)
