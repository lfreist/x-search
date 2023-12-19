add_subdirectory(utils)
add_subdirectory(string_search)
add_library(MetaFile MetaFile.cpp)

add_library(Searcher Searcher.cpp)
target_link_libraries(Searcher PUBLIC MetaFile)

add_library(xsearch xsearch.cpp)
target_link_libraries(xsearch PUBLIC StringUtils Executor SearchUtils lz4 zstd)
target_compile_options(xsearch PUBLIC
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall>
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4>)
