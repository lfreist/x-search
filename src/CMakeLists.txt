add_subdirectory(utils)
add_subdirectory(string_search)
add_subdirectory(tasks)

add_library(Result ResultTypes.cpp)

add_library(DataChunk DataChunk.cpp)
add_library(MetaFile MetaFile.cpp)

add_library(Searcher Executor.cpp)
target_link_libraries(Searcher PUBLIC DataChunk MetaFile Tasks)

add_library(xsearch xsearch.cpp)
target_link_libraries(xsearch PUBLIC DataChunk MetaFile Searcher SearchUtils lz4 zstd)