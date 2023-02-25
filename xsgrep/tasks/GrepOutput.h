// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <unistd.h>

#include <cstdio>
#include <memory>

#include "xsearch/xsearch.h"

// ===== Output colors =========================================================
#define COLOR_RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */
// _____________________________________________________________________________

/**
 * GrepPartialResult: A struct holding all information necessary to produce a
 *  GNU grep like output.
 *
 * @param byte_offset_match
 * @param byte_offset_line
 * @param line_number
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
 * GrepOptions: A struct holding information about what xsgrep searches and how
 * results will be printed.
 *
 * @param regex: was a regex search performed?
 * @param index: should the index be printed?
 * @param color: use colored output?
 * @param only_matching: only print matches and search for match offsets
 * @param pattern: the pattern that was searched
 */
struct GrepOptions {
  bool regex = false;
  bool line_number = false;
  bool byte_offset = false;
  bool color = isatty(STDOUT_FILENO);
  bool only_matching = false;
  bool ignore_case = true;
  bool no_ascii = false;
  std::string pattern;
};

/**
 * GrepOutput: The actual result class that inherits xs::BaseResult.
 *  It collects vectors of GrepPartialResults.
 */
class GrepOutput : public xs::BaseResult<std::vector<GrepPartialResult>> {
 public:
  explicit GrepOutput(GrepOptions options);
  GrepOutput(GrepOptions options, std::ostream& ostream);

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
   * Return the number of lines written to ostream so far.
   *
   * @return:
   */
  size_t size() const override;

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

  GrepOptions _options;
  std::ostream& _ostream;
  uint64_t _lines_written = 0;
};