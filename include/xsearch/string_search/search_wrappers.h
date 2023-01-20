// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

// TODO: all functions here require an additional argument holding additional
//  information like byte offsets for line_indices or lines etc.

#pragma once

#include <re2/re2.h>
#include <xsearch/DataChunk.h>

#include <iostream>
#include <vector>

namespace xs::search {

/**
 * Search byte offsets (relative to start of data) of matches of pattern within
 * data. If a match was found, 'skip_to_nl' decides whether to continue search
 * in the next line or right behind the found pattern.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const std::string&: pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> local_byte_offsets_match(xs::DataChunk* data,
                                               const std::string& pattern,
                                               bool skip_to_nl = false);

/**
 * Search byte offsets (relative to start of data + data._offset) of matches of
 * pattern within data. If a match was found, 'skip_to_nl' decides whether to
 * continue search in the next line or right behind the found pattern.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const std::string&: pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> global_byte_offsets_match(xs::DataChunk* data,
                                                const std::string& pattern,
                                                bool skip_to_nl = false);

/**
 * Search byte offsets (relative to start of data) of lines containing a match
 * of pattern within data.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const std::string&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> local_byte_offsets_line(xs::DataChunk* data,
                                              const std::string& pattern);

/**
 * Search byte offsets (relative to start of data + data._offset) of lines
 * containing a match of pattern within data.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const std::string&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> global_byte_offsets_line(xs::DataChunk* data,
                                               const std::string& pattern);

/**
 * Search line indices of lines containing a match of pattern within data.
 *
 * !! MARK: this function requires data to hold a non empty
 * _byte_to_nl_mapping_data vector !!
 *
 * @param data SFString*: data to be searched on
 * @param pattern const std::string&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found line indices
 */
std::vector<uint64_t> line_indices(xs::DataChunk* data,
                                   const std::string& pattern);

// TODO: write docs
std::vector<std::string> lines(xs::DataChunk* data, const std::string& pattern);

/**
 * Count the numbers of lines containing a match of pattern in data.
 *
 * @param data SFString*: data to be searched in
 * @param pattern const std::string&: pattern to be searched for
 * @return number of matching lines within data
 */
uint64_t count(xs::DataChunk* data, const std::string& pattern,
               bool skip_to_nl = true);

namespace regex {

/**
 * Search byte offsets (relative to start of data) of lines containing a regex
 * match of pattern within data.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> local_byte_offsets_line(const xs::DataChunk* data,
                                              const re2::RE2& pattern);

/**
 * Search byte offsets (relative to start of data + data._offset) of lines
 * containing a regex match of pattern within data.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> global_byte_offsets_line(const xs::DataChunk* data,
                                               const re2::RE2& pattern);

/**
 * Search byte offsets (relative to start of data) of regex matches of pattern
 * within data. If a match was found, 'skip_to_nl' decides whether to continue
 * search in the next line or right behind the found pattern.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> local_byte_offsets_match(const xs::DataChunk* data,
                                               const re2::RE2& pattern,
                                               bool skip_to_nl = false);

/**
 * Search byte offsets (relative to start of data + data._offset) of regex
 * matches of pattern within data. If a match was found, 'skip_to_nl' decides
 * whether to continue search in the next line or right behind the found
 * pattern.
 *
 * @param data SFString*: data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @param skip_to_nl bool: true -> find at most one match per line, false ->
 * find all matches per line
 * @return std::vector<uint64_t>: vector of all found byte offsets
 */
std::vector<uint64_t> global_byte_offsets_match(const xs::DataChunk* data,
                                                const re2::RE2& pattern,
                                                bool skip_to_nl = false);

/**
 * Search line indices of lines containing a regex match of pattern within data.
 *
 * !! MARK: this function requires data to hold a non empty
 * _byte_to_nl_mapping_data vector !!
 *
 * @param data SFString*: data to be searched on
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return std::vector<uint64_t>: vector of all found line indices
 */
std::vector<uint64_t> line_indices(xs::DataChunk* data,
                                   const re2::RE2& pattern);

// TODO: write docs
std::vector<std::string> lines(xs::DataChunk* data, const re2::RE2& pattern);

/**
 * Count the numbers of lines containing a regex match of pattern in data.
 *
 * @param data SFString*: data to be searched in
 * @param pattern const re2::RE2&: pattern to be searched for
 * @return number of matching lines within data
 */
uint64_t count(xs::DataChunk* data, const re2::RE2& pattern,
               bool skip_to_nl = true);

}  // namespace regex

}  // namespace xs::search