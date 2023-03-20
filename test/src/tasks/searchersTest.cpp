// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/tasks/defaults/searchers.h>

static const std::string pattern("Sherlock");
static const std::string re_pattern("She[r ]lock");

static const xs::DataChunk data(
    "this is some text in which we search for Sherlock.\nOr did she lock it? "
    "She locked it for sure.\npsst, sherlock is here... SHERLOCK?",
    130);

TEST(MatchCounter, process) {
  {
    xs::task::searcher::MatchCounter searcher(pattern, false, false);
    ASSERT_EQ(searcher.process(&data), 1);
  }
  {
    xs::task::searcher::MatchCounter searcher(pattern, false, true);
    ASSERT_EQ(searcher.process(&data), 3);
  }
  {
    xs::task::searcher::MatchCounter searcher(re_pattern, true, false);
    ASSERT_EQ(searcher.process(&data), 2);
  }
  {
    xs::task::searcher::MatchCounter searcher(re_pattern, true, true);
    ASSERT_EQ(searcher.process(&data), 5);
  }
}

TEST(LineCounter, process) {
  {
    xs::task::searcher::LineCounter searcher(pattern, false, false);
    ASSERT_EQ(searcher.process(&data), 1);
  }
  {
    xs::task::searcher::LineCounter searcher(pattern, false, true);
    ASSERT_EQ(searcher.process(&data), 2);
  }
  {
    xs::task::searcher::LineCounter searcher(re_pattern, true, false);
    ASSERT_EQ(searcher.process(&data), 2);
  }
  {
    xs::task::searcher::LineCounter searcher(re_pattern, true, true);
    ASSERT_EQ(searcher.process(&data), 3);
  }
}

TEST(MatchBytePositionSearcher, process) {
  {
    xs::task::searcher::MatchBytePositionSearcher searcher(pattern, false,
                                                           false);
    std::vector<uint64_t> res{41};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::MatchBytePositionSearcher searcher(pattern, false,
                                                           true);
    std::vector<uint64_t> res{41, 101, 121};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::MatchBytePositionSearcher searcher(re_pattern, true,
                                                           false);
    std::vector<uint64_t> res{41, 71};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::MatchBytePositionSearcher searcher(re_pattern, true,
                                                           true);
    std::vector<uint64_t> res{41, 58, 71, 101, 121};
    ASSERT_EQ(searcher.process(&data), res);
  }
}

TEST(LineBytePositionSearcher, process) {
  {
    xs::task::searcher::LineBytePositionSearcher searcher(pattern, false,
                                                          false);
    std::vector<uint64_t> res{0};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineBytePositionSearcher searcher(pattern, false, true);
    std::vector<uint64_t> res{0, 95};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineBytePositionSearcher searcher(re_pattern, true,
                                                          false);
    std::vector<uint64_t> res{0, 51};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineBytePositionSearcher searcher(re_pattern, true,
                                                          true);
    std::vector<uint64_t> res{0, 51, 95};
    ASSERT_EQ(searcher.process(&data), res);
  }
}

TEST(LineIndexSearcher, process) {
  {
    xs::task::searcher::LineIndexSearcher searcher(pattern, false, false);
    std::vector<uint64_t> res{0};
    ASSERT_THROW(searcher.process(&data), std::runtime_error);
  }
  xs::DataChunk chunk(data.data(), data.size(),
                      {0, 0, 0, data.size(), data.size(), {{0, 0}}});
  {
    xs::task::searcher::LineIndexSearcher searcher(pattern, false, false);
    std::vector<uint64_t> res{0};
    ASSERT_EQ(searcher.process(&chunk), res);
  }
  {
    xs::task::searcher::LineIndexSearcher searcher(pattern, false, true);
    std::vector<uint64_t> res{0, 2};
    ASSERT_EQ(searcher.process(&chunk), res);
  }
  {
    xs::task::searcher::LineIndexSearcher searcher(re_pattern, true, false);
    std::vector<uint64_t> res{0, 1};
    ASSERT_EQ(searcher.process(&chunk), res);
  }
  {
    xs::task::searcher::LineIndexSearcher searcher(re_pattern, true, true);
    std::vector<uint64_t> res{0, 1, 2};
    ASSERT_EQ(searcher.process(&chunk), res);
  }
}

TEST(LineSearcher, process) {
  {
    xs::task::searcher::LineSearcher searcher(pattern, false, false);
    std::vector<std::string> res{
        "this is some text in which we search for Sherlock."};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineSearcher searcher(pattern, false, true);
    std::vector<std::string> res{
        "this is some text in which we search for Sherlock.",
        "psst, sherlock is here... SHERLOCK?"};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineSearcher searcher(re_pattern, true, false);
    std::vector<std::string> res{
        "this is some text in which we search for Sherlock.",
        "Or did she lock it? She locked it for sure."};
    ASSERT_EQ(searcher.process(&data), res);
  }
  {
    xs::task::searcher::LineSearcher searcher(re_pattern, true, true);
    std::vector<std::string> res{
        "this is some text in which we search for Sherlock.",
        "Or did she lock it? She locked it for sure.",
        "psst, sherlock is here... SHERLOCK?"};
    ASSERT_EQ(searcher.process(&data), res);
  }
}