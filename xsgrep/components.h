// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <iostream>
#include <string>
#include <vector>

/**
 * GrepPartialResult: A struct holding all information necessary to produce a
 *  GNU grep like output.
 *
 * @param index: prefix of the outputted line: line index, byte offset, ...
 * @param str: the actual output: either line or pattern
 *
 * GNU grep output looks like: "[index]:str"
 */
struct GrepPartialResult {
  uint64_t byte_offset_match = 0;
  uint64_t byte_offset_line = 0;
  uint64_t line_number = 0;
  std::string str;

  /**
   * GrepPartialResult can be sorted using this method
   * @param other
   * @return
   */
  bool operator<(const GrepPartialResult& other) const;
};

/**
 * GrepResultSettings: A struct storing information about the assigned result
 *  output style and content.
 *
 * @param regex: was a regex search performed?
 * @param index: should the index be printed?
 * @param color: use colored output?
 * @param only_matching: only print matches and search for match offsets
 * @param pattern: the pattern that was searched
 */
struct GrepResultSettings {
  bool regex = false;
  bool index = false;
  bool color = true;
  bool only_matching = false;
  std::string pattern;
};

/**
 * GrepResult: The actual result class that inherits xs::BaseResult.
 *  It collects a pair of a vector of GrepPartialResults and corresponding
 *  GrepResultSettings.
 *  Both are provided from the searcher used.
 */
class GrepResult : public xs::BaseResult<std::vector<GrepPartialResult>> {
 public:
  explicit GrepResult(std::string pattern);
  GrepResult(std::string pattern, bool regex, bool byte_offset,
             bool line_number, bool only_matching, bool color);

  /**
   * Collect results and pass them ordered to the private add method.
   *  Results that are received before they are in turn are buffered in a
   *  std::map until its their turn.
   *
   * @param partial_result:
   * @param id: used for ordered output. Must be a closed sequence {0..X} of int
   */
  void add(std::vector<GrepPartialResult> partial_result, uint64_t id) override;

  /**
   * Dummy implementation. Always returns 0. Must be implemented because of pure
   *  virtual method inherited by xs::BaseResult.
   *
   * @return: always 0 (zero).
   */
  constexpr size_t size() const override;

 private:
  /**
   * The actual output of the collected results. Called from public add method.
   *  Always writes to standard out.
   * @param partial_result
   */
  void add(std::vector<GrepPartialResult> partial_result) override;

  /// Buffer for results that are received not in order
  std::unordered_map<uint64_t, std::vector<GrepPartialResult>> _buffer;
  /// Indicates the index of the result that is written next
  uint64_t _current_index = 0;

  /// grep output settings
  std::string _pattern;
  bool _regex = false;
  bool _byte_offset = false;
  bool _line_number = false;
  bool _only_matching = false;
  bool _color = true;
};

/**
 * GrepSearcher: The searcher used by the xs::Executor for searching results.
 */
class GrepSearcher
    : public xs::tasks::BaseSearcher<xs::DataChunk,
                                     std::vector<GrepPartialResult>> {
 public:
  /**
   * @param byte_offset: search byte offsets
   * @param line_number: search line numbers
   * @param only_matching: search matches only (default is searchin matching
   *  lines)
   * @param color: colored output
   */
  GrepSearcher(std::string pattern, bool regex, bool case_insensitive,
               bool line_number, bool only_matching);

  /**
   * Search provided data according to the specified search criteria using a
   *  plain text pattern
   * @param pattern: plain text pattern
   * @param data: data that are searched
   * @return
   */
  std::vector<GrepPartialResult> process(xs::DataChunk* data) const override;

 private:
  std::vector<GrepPartialResult> process_regex(xs::DataChunk* data) const;
  std::vector<GrepPartialResult> process_plain(xs::DataChunk* data) const;

  /// search for line numbers
  bool _line_number;
  /// search matches only (default is searching matching lines)
  bool _only_matching;
};