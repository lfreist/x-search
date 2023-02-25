// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/Executor.h>
#include <xsearch/MetaFile.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/string_search/offset_mappings.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/string_search/simd_search.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/tasks/InplaceProcessors.h>
#include <xsearch/tasks/ReturnProcessors.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>
#include <xsearch/utils/string_utils.h>
#include <xsearch/utils/utils.h>

#include <memory>

namespace xs {

/**
 * By defining full Executor<> types, we allow the user to get easy access
 *  to basic search functionalities including:
 *    - count: count matches
 *    - count_lines: count lines containing a match
 *    - match_byte_offsets: a vector of the byte offsets of all matches
 *    - line_byte_offsets: a vector of the byte offsets of matching lines
 *    - line_indices: a vector of the line indices of matching lines
 *    - lines: a vector of lines (as std::string) containing the match
 *    - full: a combined result of the above
 *  The user can easily call the extern_search function using one of the types
 *  defined here as its template argument.
 *
 *  E.g.: extern_search<count>(...) will start and return an ExternSearch object
 *  initialized with everything needed for counting matches.
 */
typedef Executor<xs::DataChunk, CountMatchesResult, uint64_t> count_matches;
typedef Executor<xs::DataChunk, CountLinesResult, uint64_t> count_lines;
typedef Executor<xs::DataChunk, MatchByteOffsetsResult, std::vector<uint64_t>>
    match_byte_offsets;
typedef Executor<xs::DataChunk, LineByteOffsetsResult, std::vector<uint64_t>>
    line_byte_offsets;
typedef Executor<xs::DataChunk, LineIndicesResult, std::vector<uint64_t>>
    line_indices;
typedef Executor<xs::DataChunk, LinesResult, std::vector<std::string>> lines;

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
template <typename T>
std::shared_ptr<T> extern_search(const std::string& pattern,
                                 const std::string& file_path,
                                 const std::string& meta_file_path,
                                 bool ignore_case = false, int num_threads = 1,
                                 int num_readers = 1);

template <typename T>
std::shared_ptr<T> extern_search(const std::string& pattern,
                                 const std::string& file_path,
                                 bool ignore_case = false, int num_threads = 1);

}  // namespace xs