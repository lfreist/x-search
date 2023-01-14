// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/DataChunk.h>

namespace xs {

TEST(DataChunk, constructor) {
  {
    DataChunk str;

    ASSERT_TRUE(str._content.empty());
    ASSERT_EQ(str._offset, static_cast<size_t>(0));
    ASSERT_EQ(str._originalSize, static_cast<size_t>(0));
  }
}

// TODO: TEST

}  // namespace xs