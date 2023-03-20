// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/tasks/defaults/readers.h>

static const std::string file("test/files/sample.txt");

TEST(FileBlockReader, getNextData) {
  {
    xs::task::reader::FileBlockReader reader(file, 50, 0);
    ASSERT_THROW(reader.getNextData(), std::runtime_error);
  }
  {
    xs::task::reader::FileBlockReader reader(file, 50, 5);
    ASSERT_THROW(reader.getNextData(), std::runtime_error);
  }
  {
    xs::task::reader::FileBlockReader reader(file, 50, 100);
    auto data = reader.getNextData();

    ASSERT_TRUE(data.has_value());

    auto& val = data.value();

    ASSERT_EQ(val.second, 0);
    ASSERT_EQ(val.first.size(), 56);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 0);
    ASSERT_EQ(val.first.getMetaData().original_offset, 0);
    ASSERT_EQ(val.first.getMetaData().actual_size, 56);
    ASSERT_EQ(val.first.getMetaData().original_size, 56);
    ASSERT_EQ(std::string(val.first.data(), val.first.size()),
              "What did you do?\nWhat did you...?\n- Nothing!\n- Nothing?\n");

    data = reader.getNextData();
    ASSERT_TRUE(data.has_value());

    val = data.value();

    ASSERT_EQ(val.second, 1);
    ASSERT_EQ(val.first.size(), 59);
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 56);
    ASSERT_EQ(val.first.getMetaData().original_offset, 56);
    ASSERT_EQ(val.first.getMetaData().actual_size, 59);
    ASSERT_EQ(val.first.getMetaData().original_size, 59);
    ASSERT_EQ(std::string(val.first.data(), val.first.size()),
              "Bullshit!\nFuck, he's bleeding to death.\nThey still coming?\n");
  }
}