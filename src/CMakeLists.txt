add_subdirectory(pipeline)
add_subdirectory(utils)
add_subdirectory(string_search)

add_library(DataChunk DataChunk.cpp)
add_library(MetaFile MetaFile.cpp)

add_library(ExternSearcher ExternSearcher.cpp)
target_link_libraries(ExternSearcher PUBLIC DataChunk MetaFile TaskManager DefaultTasks)

add_library(FilePreprocessing FilePreprocessing.cpp)
target_link_libraries(FilePreprocessing PUBLIC DataChunk MetaFile lz4 zstd)

add_library(xsearch xsearch.cpp)
target_link_libraries(xsearch PUBLIC DataChunk MetaFile ExternSearcher TaskManager DefaultTasks SearchUtils FilePreprocessing)