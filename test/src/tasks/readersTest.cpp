// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/tasks/defaults/readers.h>

static const std::string file("test/files/sample.txt");
static const std::string meta_file("test/files/sample.meta");

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
  {
    xs::task::reader::FileBlockReader reader(file, 50, 10000);
    auto data = reader.getNextData();
    while (true) {
      auto tmp = reader.getNextData();
      if (!tmp.has_value()) {
        break;
      }
      data = std::move(tmp);
    }
    ASSERT_TRUE(data.has_value());
    auto& val = data.value();
    ASSERT_EQ(val.second, 1396209);
    ASSERT_EQ(val.first.size(), 46);
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 99999954);
    ASSERT_EQ(val.first.getMetaData().original_offset, 99999954);
    ASSERT_EQ(val.first.getMetaData().actual_size, 46);
    ASSERT_EQ(val.first.getMetaData().original_size, 46);
    ASSERT_EQ(std::string(val.first.data(), val.first.size()),
              "Package for you, sir.\nI want to come back.\n- T");
  }
}

TEST(FileBlockReaderMMAP, getNextData) {
  {
    xs::task::reader::FileBlockReaderMMAP reader(file, 4, 0);
    ASSERT_THROW(reader.getNextData(), std::runtime_error);
  }
  {
    xs::task::reader::FileBlockReaderMMAP reader(file, 4, 2);
    ASSERT_THROW(reader.getNextData(), std::runtime_error);
  }
  {
    xs::task::reader::FileBlockReaderMMAP reader(file);
    auto data = reader.getNextData();

    ASSERT_TRUE(data.has_value());

    auto& val = data.value();

    ASSERT_EQ(val.second, 0);
    ASSERT_EQ(val.first.size(), 16777222);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 0);
    ASSERT_EQ(val.first.getMetaData().original_offset, 0);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777222);

    data = reader.getNextData();
    ASSERT_TRUE(data.has_value());

    val = data.value();

    ASSERT_EQ(val.second, 1);
    ASSERT_EQ(val.first.size(), 16777261);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777261);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777261);
  }
  {
    xs::task::reader::FileBlockReaderMMAP reader(file);
    auto data = reader.getNextData();
    while (true) {
      auto tmp = reader.getNextData();
      if (!tmp.has_value()) {
        break;
      }
      data = std::move(tmp);
    }
    ASSERT_TRUE(data.has_value());
    auto& val = data.value();
    ASSERT_EQ(val.second, 5);
    ASSERT_EQ(val.first.size(), 16113720);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_TRUE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 83886280);
    ASSERT_EQ(val.first.getMetaData().original_offset, 83886280);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16113720);
    ASSERT_EQ(val.first.getMetaData().original_size, 16113720);
  }
}

TEST(FileBlockMetaReaderMMAP, getNextData) {
  {
    xs::task::reader::FileBlockMetaReaderMMAP reader(file, meta_file, 1);
    auto data = reader.getNextData();

    ASSERT_TRUE(data.has_value());

    auto& val = data.value();

    ASSERT_EQ(val.second, 0);
    ASSERT_EQ(val.first.size(), 16777222);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 0);
    ASSERT_EQ(val.first.getMetaData().original_offset, 0);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777222);

    data = reader.getNextData();
    ASSERT_TRUE(data.has_value());

    val = data.value();

    ASSERT_EQ(val.second, 1);
    ASSERT_EQ(val.first.size(), 16777261);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777261);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777261);
  }
  {
    xs::task::reader::FileBlockMetaReaderMMAP reader(file, meta_file, 1);
    auto data = reader.getNextData();
    while (true) {
      auto tmp = reader.getNextData();
      if (!tmp.has_value()) {
        break;
      }
      data = std::move(tmp);
    }
    ASSERT_TRUE(data.has_value());
    auto& val = data.value();
    ASSERT_EQ(val.second, 5);
    ASSERT_EQ(val.first.size(), 16113790);
    ASSERT_TRUE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().original_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16113790);
    ASSERT_EQ(val.first.getMetaData().original_size, 16113790);
  }
}

TEST(FileBlockMetaReader, getNextData) {
  {
    xs::task::reader::FileBlockMetaReader reader(file, meta_file, 1);
    auto data = reader.getNextData();

    ASSERT_TRUE(data.has_value());

    auto& val = data.value();

    ASSERT_EQ(val.second, 0);
    ASSERT_EQ(val.first.size(), 16777222);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 0);
    ASSERT_EQ(val.first.getMetaData().original_offset, 0);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777222);

    data = reader.getNextData();
    ASSERT_TRUE(data.has_value());

    val = data.value();

    ASSERT_EQ(val.second, 1);
    ASSERT_EQ(val.first.size(), 16777261);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777261);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777261);
  }
  {
    xs::task::reader::FileBlockMetaReader reader(file, meta_file, 1);
    auto data = reader.getNextData();
    while (true) {
      auto tmp = reader.getNextData();
      if (!tmp.has_value()) {
        break;
      }
      data = std::move(tmp);
    }
    ASSERT_TRUE(data.has_value());
    auto& val = data.value();
    ASSERT_EQ(val.second, 5);
    ASSERT_EQ(val.first.size(), 16113790);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().original_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16113790);
    ASSERT_EQ(val.first.getMetaData().original_size, 16113790);
  }
}

TEST(FileBlockMetaReaderSingle, getNextData) {
  {
    xs::task::reader::FileBlockMetaReaderSingle reader(file, meta_file);
    auto data = reader.getNextData();

    ASSERT_TRUE(data.has_value());

    auto& val = data.value();

    ASSERT_EQ(val.second, 0);
    ASSERT_EQ(val.first.size(), 16777222);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 0);
    ASSERT_EQ(val.first.getMetaData().original_offset, 0);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777222);

    data = reader.getNextData();
    ASSERT_TRUE(data.has_value());

    val = data.value();

    ASSERT_EQ(val.second, 1);
    ASSERT_EQ(val.first.size(), 16777261);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().original_offset, 16777222);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16777261);
    ASSERT_EQ(val.first.getMetaData().original_size, 16777261);
  }
  {
    xs::task::reader::FileBlockMetaReaderSingle reader(file, meta_file);
    auto data = reader.getNextData();
    while (true) {
      auto tmp = reader.getNextData();
      if (!tmp.has_value()) {
        break;
      }
      data = std::move(tmp);
    }
    ASSERT_TRUE(data.has_value());
    auto& val = data.value();
    ASSERT_EQ(val.second, 5);
    ASSERT_EQ(val.first.size(), 16113790);
    ASSERT_FALSE(val.first.is_mmap());
    ASSERT_FALSE(val.first.getMetaData().line_mapping_data.empty());
    ASSERT_EQ(val.first.getMetaData().actual_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().original_offset, 83886210);
    ASSERT_EQ(val.first.getMetaData().actual_size, 16113790);
    ASSERT_EQ(val.first.getMetaData().original_size, 16113790);
  }
}