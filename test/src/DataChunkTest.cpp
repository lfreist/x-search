// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/DataChunk.h>

namespace xs {

TEST(DataChunk, constructor) {
  {
    DataChunk str;

    ASSERT_TRUE(str._data.empty());
    ASSERT_EQ(str.size(), 0);
    ASSERT_EQ(str._meta_data.chunk_index, 0);
    ASSERT_EQ(str._meta_data.original_offset, 0);
    ASSERT_EQ(str._meta_data.actual_offset, 0);
    ASSERT_EQ(str._meta_data.original_size, 0);
    ASSERT_EQ(str._meta_data.actual_size, 0);
    ASSERT_TRUE(str._meta_data.line_mapping_data.empty());
  }
  {
    DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const ChunkMetaData cmd{0, 7, 7, 10, 10, {{0, 1}}};

    ASSERT_EQ(str._data, content);
    ASSERT_EQ(str.size(), 5);
    ASSERT_EQ(str._meta_data.chunk_index, 0);
    ASSERT_EQ(str._meta_data.original_offset, 7);
    ASSERT_EQ(str._meta_data.actual_offset, 7);
    ASSERT_EQ(str._meta_data.original_size, 10);
    ASSERT_EQ(str._meta_data.actual_size, 10);
    ASSERT_EQ(str._meta_data.line_mapping_data.size(), 1);
    ASSERT_EQ(str._meta_data.line_mapping_data[0].globalByteOffset, 0);
    ASSERT_EQ(str._meta_data.line_mapping_data[0].globalLineIndex, 1);
  }
  {
    DataChunk str(strtype{'h', 'e', 'l', 'l', 'o'});

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};

    ASSERT_EQ(str._data, content);
    ASSERT_EQ(str.size(), 5);
    ASSERT_EQ(str._meta_data.chunk_index, 0);
    ASSERT_EQ(str._meta_data.original_offset, 0);
    ASSERT_EQ(str._meta_data.actual_offset, 0);
    ASSERT_EQ(str._meta_data.original_size, 0);
    ASSERT_EQ(str._meta_data.actual_size, 0);
    ASSERT_TRUE(str._meta_data.line_mapping_data.empty());
  }
}

TEST(DataChunk, MoveConstructor) {
  {
    DataChunk fst({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});

    auto snd(std::move(fst));

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const ChunkMetaData cmd{0, 7, 7, 10, 10, {{0, 1}}};

    ASSERT_EQ(snd._data, content);
    ASSERT_EQ(snd.size(), 5);
    ASSERT_EQ(snd._meta_data.chunk_index, 0);
    ASSERT_EQ(snd._meta_data.original_offset, 7);
    ASSERT_EQ(snd._meta_data.actual_offset, 7);
    ASSERT_EQ(snd._meta_data.original_size, 10);
    ASSERT_EQ(snd._meta_data.actual_size, 10);
    ASSERT_EQ(snd._meta_data.line_mapping_data.size(), 1);
    ASSERT_EQ(snd._meta_data.line_mapping_data[0].globalByteOffset, 0);
    ASSERT_EQ(snd._meta_data.line_mapping_data[0].globalLineIndex, 1);
  }
}

TEST(DataChunk, getData) {
  DataChunk str(strtype{'h', 'e', 'l', 'l', 'o'});

  const strtype content{'h', 'e', 'l', 'l', 'o'};

  ASSERT_EQ(str.getData(), content);
}

TEST(DataChunk, getMetaData) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});

  ASSERT_EQ(str.getMetaData().chunk_index, 0);
  ASSERT_EQ(str.getMetaData().original_offset, 7);
  ASSERT_EQ(str.getMetaData().actual_offset, 7);
  ASSERT_EQ(str.getMetaData().original_size, 10);
  ASSERT_EQ(str.getMetaData().actual_size, 10);
  ASSERT_EQ(str.getMetaData().line_mapping_data.size(), 1);
  ASSERT_EQ(str.getMetaData().line_mapping_data[0].globalByteOffset, 0);
  ASSERT_EQ(str.getMetaData().line_mapping_data[0].globalLineIndex, 1);
}

TEST(DataChunk, data) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});

  ASSERT_EQ(str.data(), str._data.data());
}

TEST(DataChunk, size) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});
  ASSERT_EQ(str.size(), 5);
  ASSERT_EQ(str._data.size(), 5);
}

TEST(DataChunk, resize) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});
  ASSERT_EQ(str.size(), 5);
  ASSERT_EQ(str._data.size(), 5);

  str.resize(10);
  ASSERT_EQ(str.size(), 10);
  ASSERT_EQ(str._data.size(), 10);
}

TEST(DataChunk, push_back) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});
  const strtype content{'h', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(str._data, content);

  str.push_back('!');
  const strtype content2{'h', 'e', 'l', 'l', 'o', '!'};
  ASSERT_EQ(str._data, content2);
}

TEST(DataChunk, assign) {
  DataChunk str({'h', 'e', 'l', 'l', 'o'}, {0, 7, 7, 10, 10, {{0, 1}}});
  const strtype content{'h', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(str._data, content);

  str.assign("hello!");
  const strtype content2{'h', 'e', 'l', 'l', 'o', '!'};
  ASSERT_EQ(str._data, content2);
}

}  // namespace xs