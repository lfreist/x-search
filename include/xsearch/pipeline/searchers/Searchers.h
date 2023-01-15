// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <re2/re2.h>
#include <xsearch/DataChunk.h>

namespace xs::tasks {

class Searcher {
 public:
  explicit Searcher(std::string pattern, bool regex = false);
  ~Searcher() = default;

  void count(DataChunk* data);

  void byte_offsets_match(DataChunk* data, bool skip_line = true);

  void byte_offsets_line(DataChunk* data);

  void line_indices(DataChunk* data);

  void line(DataChunk* data);

  void setPattern(const std::string& pattern, bool regex = false);

 private:
  std::string _pattern;
  std::unique_ptr<re2::RE2> _regex_pattern;
  bool _regex;
};

}  // namespace xs::tasks