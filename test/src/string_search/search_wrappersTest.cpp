// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/string_search/search_wrappers.h>

#include <sstream>

using namespace xs;

static char dummy_text[] =
    "Liant reindorsing two-time zippering chromolithography rainbowweed\n"
    "Cacatua bunking cooptions zinckenite Polygala\n"
    "smooth-bellied chirognostic inkos BVM antigraphy pagne bicorne\n"
    "complementizer commorant ever-endingly sheikhly\n"
    "glam predamaged objectionability evil-looking quaquaversal\n"
    "composite halter-wise mosasaur Whelan coleopterist grass-grown Helladic\n"
    "DNB nondeliriousness arpents uncasing\n"
    "predepletion delator unnaive sucken solid-gold brassards tutorials\n"
    "refrangible terebras autobiographal mid-breast ant\n";

TEST(search, local_byte_offsets_match) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::local_byte_offsets_match(&data, "ant"), res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::local_byte_offsets_match(&data, "ant"), res);
  }
}

TEST(search, global_byte_offsets_match) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::global_byte_offsets_match(&data, "ant"), res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{12, 161, 207, 517};
    ASSERT_EQ(::search::global_byte_offsets_match(&data, "ant"), res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{102, 251, 297, 607};
    ASSERT_EQ(::search::global_byte_offsets_match(&data, "ant"), res);
  }
}

TEST(search, local_byte_offsets_line) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::local_byte_offsets_line(&data, "ant"), res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::local_byte_offsets_line(&data, "ant"), res);
  }
}

TEST(search, global_byte_offsets_line) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::global_byte_offsets_line(&data, "ant"), res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{10, 123, 186, 470};
    ASSERT_EQ(::search::global_byte_offsets_line(&data, "ant"), res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{100, 213, 276, 560};
    ASSERT_EQ(::search::global_byte_offsets_line(&data, "ant"), res);
  }
}

TEST(search, line_indices) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::line_indices(&data, {2, 151, 197, 507}, "ant"), res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::line_indices(&data, {12, 161, 207, 517}, "ant"), res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::line_indices(&data, {102, 251, 297, 607}, "ant"), res);
  }
}

TEST(search, count) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    ASSERT_EQ(::search::count(&data, "ant"), static_cast<uint64_t>(4));
  }
}

TEST(search_regex, local_byte_offsets_match) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(
        ::search::regex::local_byte_offsets_match(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(
        ::search::regex::local_byte_offsets_match(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
}

TEST(search_regex, global_byte_offsets_match) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::regex::global_byte_offsets_match(&data,
                                                         re2::RE2("(a[n|m]t)")),
              res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{12, 161, 207, 517};
    ASSERT_EQ(::search::regex::global_byte_offsets_match(&data,
                                                         re2::RE2("(a[n|m]t)")),
              res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{102, 251, 297, 607};
    ASSERT_EQ(::search::regex::global_byte_offsets_match(&data,
                                                         re2::RE2("(a[n|m]t)")),
              res);
  }
}

TEST(search_regex, local_byte_offsets_line) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(
        ::search::regex::local_byte_offsets_line(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(
        ::search::regex::local_byte_offsets_line(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
}

TEST(search_regex, global_byte_offsets_line) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(
        ::search::regex::global_byte_offsets_line(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{10, 123, 186, 470};
    ASSERT_EQ(
        ::search::regex::global_byte_offsets_line(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{100, 213, 276, 560};
    ASSERT_EQ(
        ::search::regex::global_byte_offsets_line(&data, re2::RE2("(a[n|m]t)")),
        res);
  }
}

TEST(search_regex, line_indices) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::regex::line_indices(&data, {2, 151, 197, 507},
                                            re2::RE2("(a[n|m]t)")),
              res);
  }
  {
    DataChunk data(0, 10, {{186, 3}, {403, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::regex::line_indices(&data, {12, 161, 207, 517},
                                            re2::RE2("(a[n|m]t)")),
              res);
  }
  {
    DataChunk data(0, 100, {{276, 3}, {493, 7}});
    data.assign(dummy_text);
    std::vector<uint64_t> res{0, 2, 3, 8};
    ASSERT_EQ(::search::regex::line_indices(&data, {102, 251, 297, 607},
                                            re2::RE2("(a[n|m]t)")),
              res);
  }
}

TEST(search_regex, count) {
  {
    DataChunk data(0, 0, {{176, 3}, {393, 7}});
    data.assign(dummy_text);
    ASSERT_EQ(::search::regex::count(&data, re2::RE2("(a[n|m]t)")),
              static_cast<uint64_t>(4));
  }
}
