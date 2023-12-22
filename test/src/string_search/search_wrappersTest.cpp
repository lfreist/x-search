// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/string_search/search_wrappers.h>
#include <xsearch/types.h>

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

TEST(search, byte_offsets_match) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::byte_offsets_match(data, "ant"), res);
  }
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::byte_offsets_match(data, "ant"), res);
  }
}

TEST(search, byte_offsets_line) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::byte_offsets_line(data, "ant"), res);
  }
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::byte_offsets_line(data, "ant"), res);
  }
}

TEST(search, count) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    ASSERT_EQ(::search::count(data, "ant"), static_cast<uint64_t>(4));
  }
}

TEST(search, line) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::byte_offsets_line(data, "ant"), res);
  }
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<std::string> res{
        "Liant reindorsing two-time zippering chromolithography rainbowweed\n",
        "smooth-bellied chirognostic inkos BVM antigraphy pagne bicorne\n",
        "complementizer commorant ever-endingly sheikhly\n",
        "refrangible terebras autobiographal mid-breast ant\n"
    };
    ASSERT_EQ(::search::line(data, "ant"), res);
  }
}

TEST(search_regex, byte_offsets_match) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::regex::byte_offsets_match(data, re2::RE2("(a[n|m]t)")), res);
  }
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{2, 151, 197, 507};
    ASSERT_EQ(::search::regex::byte_offsets_match(data, re2::RE2("(a[n|m]t)")), res);
  }
}

TEST(search_regex, byte_offsets_line) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::regex::byte_offsets_line(data, re2::RE2("(a[n|m]t)")), res);
  }
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    std::vector<uint64_t> res{0, 113, 176, 460};
    ASSERT_EQ(::search::regex::byte_offsets_line(data, re2::RE2("(a[n|m]t)")), res);
  }
}

TEST(search_regex, count) {
  {
    xs::strtype data(dummy_text, dummy_text + strlen(dummy_text));
    ASSERT_EQ(::search::regex::count(data, re2::RE2("(a[n|m]t)")), static_cast<uint64_t>(4));
  }
}
