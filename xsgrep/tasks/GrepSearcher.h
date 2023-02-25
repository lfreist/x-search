// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include "./GrepOutput.h"

/**
 * GrepSearcher: The searcher used by the xs::Executor for searching results.
 */
class GrepSearcher
    : public xs::tasks::BaseReturnProcessor<xs::DataChunk,
                                            std::vector<GrepPartialResult>> {
 public:
  /**
   * @param options: search/output options for grep like results
   */
  explicit GrepSearcher(GrepOptions options);

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
  std::vector<GrepPartialResult> process_utf8(xs::DataChunk* data) const;

  /// search for line numbers
  GrepOptions _options;
  std::unique_ptr<re2::RE2> _re_pattern;
};
