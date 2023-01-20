add_library(Tasks DataProvider.cpp Processor.cpp Searcher.cpp)
target_link_libraries(Tasks PUBLIC DataChunk Result MetaFile SearchUtils)