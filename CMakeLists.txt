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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif ()

IF (${MAIN_PROJECT})

    add_executable(metafile_cat metafile_cat.cpp)
    target_link_libraries(metafile_cat PUBLIC xsearch)
    # --- BOOST ------------------------------------------------------------------------------------------------------------
    find_package(Boost 1.74 REQUIRED COMPONENTS program_options)
    include_directories(${Boost_INCLUDE_DIR})
    # ----------------------------------------------------------------------------------------------------------------------
    # --- GTEST --------------------------------------------------------------------------------------------------------
    add_subdirectory(third_party/googletest/)
    include_directories(third_party/googletest/googletest/include)
    # ------------------------------------------------------------------------------------------------------------------
endif ()

# --- RE2 --------------------------------------------------------------------------------------------------------------
add_subdirectory(third_party/re2/)
include_directories(third_party/re2/)
# ----------------------------------------------------------------------------------------------------------------------

# --- X-SEARCHER -------------------------------------------------------------------------------------------------------
include_directories(${PROJECT_SOURCE_DIR}/include/)
add_subdirectory(src)
# ----------------------------------------------------------------------------------------------------------------------

IF (${MAIN_PROJECT})
    # ___ InlineBench Library ______________________________________________________________________________________________________
    add_library(InlineBench INTERFACE include/xsearch/utils/InlineBench.h)
endif ()

option(EXE_TESTS "run exhaustive executable tests" OFF)

IF (${MAIN_PROJECT})
    # ___ Executables __________________________________________________________________________________________________
    add_subdirectory(xsgrep)
    add_subdirectory(xsproc)

    # ___ Tests ________________________________________________________________________________________________________
    include(CTest)
    add_subdirectory(test)

    add_test(MetaFileTest test/src/MetaFileTestMain)
    add_test(DataChunkTest test/src/DataChunkTestMain)
    add_test(ExternSearcherTest test/src/ExternSearcherTestMain)
    add_test(TSQueueTest test/src/utils/TSQueueTestMain)
    add_test(OffsetMappingsTest test/src/string_search/OffsetMappingsTestMain)
    add_test(SearchWrappersTest test/src/string_search/SearchWrappersTestMain)
    add_test(SimdSearchTest test/src/string_search/SimdSearchTestMain)
    add_test(xsearchTest test/src/xsearchTestMain)

    # executable exhaustive tests --------------------------------------------------------------------------------------
    IF (XS_EXE_TESTS)
        configure_file(test/test_grep.sh test/test_grep.sh COPYONLY)
        configure_file(test/test_xspreprocessor.sh test/test_xspreprocessor.sh COPYONLY)

        configure_file(files/words.txt files/words.txt COPYONLY)
        configure_file(scripts/createTestFile.py scripts/createTestFile.py COPYONLY)
        add_test(grepTest bash test/test_grep.sh ${CMAKE_BINARY_DIR})
        add_test(preprocessorTest bash test/test_grep.sh ${CMAKE_BINARY_DIR})

        set_tests_properties(grepTest PROPERTIES TIMEOUT 10000)
        set_tests_properties(grepTest PROPERTIES LABELS "grep_exe_test")
        set_tests_properties(preprocessorTest PROPERTIES TIMEOUT 10000)
        set_tests_properties(preprocessorTest PROPERTIES LABELS "preprocessor_exe_test")
    endif ()

    # benchmarks -------------------------------------------------------------------------------------------------------
    IF (XS_BENCHMARKS)
        configure_file(scripts/benchmark_chunk_size.sh scripts/benchmark_chunk_size.sh COPYONLY)
        configure_file(scripts/benchmark_compression.sh scripts/benchmark_compression.sh COPYONLY)
        configure_file(scripts/benchmark_grep.sh scripts/benchmark_grep.sh COPYONLY)
        configure_file(scripts/benchmark_nl_mapping.sh scripts/benchmark_nl_mapping.sh COPYONLY)
        configure_file(scripts/benchmark_pattern_density.sh scripts/benchmark_pattern_density.sh COPYONLY)

        configure_file(files/words.txt files/words.txt COPYONLY)
        configure_file(scripts/createTestFile.py scripts/createTestFile.py COPYONLY)

        add_test(benchmark_chunk_size bash scripts/benchmark_chunk_size.sh ${CMAKE_BINARY_DIR})
        add_test(benchmark_compression bash scripts/benchmark_compression.sh ${CMAKE_BINARY_DIR})
        add_test(benchmark_grep bash scripts/benchmark_grep.sh ${CMAKE_BINARY_DIR})
        add_test(benchmark_nl_mapping bash scripts/benchmark_nl_mapping.sh ${CMAKE_BINARY_DIR})
        add_test(benchmark_pattern_density bash scripts/benchmark_pattern_density.sh ${CMAKE_BINARY_DIR})

        set_tests_properties(benchmark_chunk_size PROPERTIES TIMEOUT 10000)
        set_tests_properties(benchmark_chunk_size PROPERTIES LABELS "benchmark_chunk_size")
        set_tests_properties(benchmark_compression PROPERTIES TIMEOUT 10000)
        set_tests_properties(benchmark_compression PROPERTIES LABELS "benchmark_compression")
        set_tests_properties(benchmark_grep PROPERTIES TIMEOUT 10000)
        set_tests_properties(benchmark_grep PROPERTIES LABELS "benchmark_grep")
        set_tests_properties(benchmark_nl_mapping PROPERTIES TIMEOUT 10000)
        set_tests_properties(benchmark_nl_mapping PROPERTIES LABELS "benchmark_nl_mapping")
        set_tests_properties(benchmark_pattern_density PROPERTIES TIMEOUT 10000)
        set_tests_properties(benchmark_pattern_density PROPERTIES LABELS "benchmark_pattern_density")
    endif()
endif ()
