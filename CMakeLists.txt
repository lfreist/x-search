cmake_minimum_required(VERSION 3.9)
project(x-search C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_BENCHMARK "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DBENCHMARK")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

# --- BOOST ------------------------------------------------------------------------------------------------------------
find_package(Boost 1.74 REQUIRED COMPONENTS program_options chrono)
include_directories(${Boost_INCLUDE_DIR})
# ----------------------------------------------------------------------------------------------------------------------

IF(${MAIN_PROJECT})
    # --- GTEST --------------------------------------------------------------------------------------------------------
    add_subdirectory(third_party/googletest/)
    include_directories(third_party/googletest/googletest/include)
    # ------------------------------------------------------------------------------------------------------------------
endif()

# --- RE2 --------------------------------------------------------------------------------------------------------------
add_subdirectory(third_party/googlere2/)
include_directories(third_party/googlere2/)
# ----------------------------------------------------------------------------------------------------------------------

# --- X-SEARCHER -------------------------------------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include/)
add_subdirectory(src)
# ----------------------------------------------------------------------------------------------------------------------

IF(${MAIN_PROJECT})
    # ___ Library ______________________________________________________________________________________________________
    add_library(InlineBench INTERFACE include/xsearch/utils/InlineBench.h)

    # ___ Executables __________________________________________________________________________________________________
    add_executable(sfgrep sfgrep.cpp)
    target_link_libraries(sfgrep PUBLIC xsearch boost_program_options)

    add_executable(FilePreprocessor FilePreprocessorMain.cpp)
    target_link_libraries(FilePreprocessor PUBLIC xsearch boost_program_options)
endif()

IF(${MAIN_PROJECT})
    include(CTest)
    # ___ Tests ________________________________________________________________________________________________________
    add_subdirectory(test)

    add_test(MetaFileTest test/src/MetaFileTestMain)
    add_test(DataChunkTest test/src/DataChunkTestMain)
    add_test(TSQueueTest test/src/utils/TSQueueTestMain)
    add_test(OffsetMappingsTest test/src/string_search/OffsetMappingsTestMain)
    add_test(SearchWrappersTest test/src/string_search/SearchWrappersTestMain)
    add_test(SimdSearchTest test/src/string_search/SimdSearchTestMain)
endif()
