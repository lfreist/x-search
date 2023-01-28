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

/**
 * By defining full ExternSearcher<> types, we allow the user to get easy access
 *  to basic search functionalities including:
 *    - count: count matches
 *    - count_lines: count lines containing a match
 *    - match_byte_offsets: a vector of the byte offsets of all matches
 *    - line_byte_offsets: a vector of the byte offsets of matching lines
 *    - lines: a vector of lines (as std::string) containing the match
 *    - full: a combined result of the above
 *  The user can easily call the extern_search function using one of the types
 *  defined here as its template argument.
 *
 *  E.g.: extern_search<count>(...) will start and return an ExternSearch object
 *  initialized with everything needed for counting matches.
 */
typedef ExternSearcher<xs::DataChunk, CountResult, uint64_t> count;
typedef ExternSearcher<xs::DataChunk, CountLinesResult, uint64_t> count_lines;
typedef ExternSearcher<xs::DataChunk, MatchByteOffsetsResult,
                       IndexPartialResult>
    match_byte_offsets;
typedef ExternSearcher<xs::DataChunk, LineByteOffsetsResult, IndexPartialResult>
    line_byte_offsets;
typedef ExternSearcher<xs::DataChunk, LineIndexResult, IndexPartialResult>
    line_indices;
typedef ExternSearcher<xs::DataChunk, LinesResult, LinesPartialResult> lines;
typedef ExternSearcher<xs::DataChunk, FullResult, FullPartialResult> full;

/**
 * A simple one-function API call to ExternSearcher.
 *
 * @tparam T: defines the type of searcher and result used. Options are:
 *   - count: count matches
 *   - count_lines: count lines containing a match
 *   - match_byte_offsets: a vector of the byte offsets of all matches
 *   - line_byte_offsets: a vector of the byte offsets of matching lines
 *   - lines: a vector of lines (as std::string) containing the match
 *   - full: a combined result of the above
 * @param pattern: pattern to be searched for
 * @param file_path: path to file
 * @param meta_file_path: path to corresponding metafile
 * @param num_threads: max. number of used threads
 * @param num_readers: max. number of threads that are allowed reading at the
 *  same time
 * @return: a shared_ptr of an initialized and started ExternSearcher
 */
template <class T>
std::shared_ptr<T> extern_search(const std::string& pattern,
                                 const std::string& file_path,
                                 const std::string& meta_file_path,
                                 int num_threads, int num_readers = 1);

}  // namespace xs