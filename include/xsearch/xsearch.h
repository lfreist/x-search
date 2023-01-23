// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ExternSearcher.h>
#include <xsearch/FilePreprocessing.h>
#include <xsearch/MetaFile.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/tasks/Processor.h>
#include <xsearch/tasks/ResultCollector.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/IOColor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>
#include <xsearch/utils/utils.h>

#include <memory>

namespace xs {

typedef ExternSearcher<xs::DataChunk, CountResult, uint64_t> count;
typedef ExternSearcher<xs::DataChunk, FullResult, FullPartialResult> full;

template <class T>
std::shared_ptr<T> extern_search(const std::string& pattern,
                                 const std::string& file_path,
                                 const std::string& meta_file_path,
                                 int num_threads, int num_readers = 1);

}  // namespace xs