// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/DataChunk.h>

namespace xs {

TEST(DataChunk, constructor) {
  std::string tmp("hello");
  {
    DataChunk str;

    ASSERT_EQ(str.size(), 0);
  }
  {
    DataChunk str(tmp.data(), 5);

    char content[5] = {'h', 'e', 'l', 'l', 'o'};

    ASSERT_EQ(*str.data(), *content);
    ASSERT_EQ(str.size(), 5);
  }
  {
    DataChunk str(tmp.data(), 5, {0, 7, 7, 10, 10, {{0, 1}}});

    char content[5] = {'h', 'e', 'l', 'l', 'o'};
    const ChunkMetaData cmd{0, 7, 7, 10, 10, {{0, 1}}};

    ASSERT_EQ(*str.data(), *content);
    ASSERT_EQ(str.size(), 5);
    ASSERT_EQ(str.getMetaData(), cmd);
  }
}

TEST(DataChunk, MoveConstructor) {
  {
    DataChunk fst({0, 7, 7, 10, 10, {{0, 1}}});
    fst.assign("hello");

    auto snd(std::move(fst));

    char content[5] = {'h', 'e', 'l', 'l', 'o'};
    const ChunkMetaData cmd{0, 7, 7, 10, 10, {{0, 1}}};

    ASSERT_EQ(*snd.data(), *content);
    ASSERT_EQ(snd.size(), 5);
    ASSERT_EQ(snd.getMetaData(), cmd);
  }
  {
    DataChunk fst({0, 7, 7, 10, 10, {{0, 1}}});
    fst.assign("hello");

    DataChunk snd = std::move(fst);

    char content[5] = {'h', 'e', 'l', 'l', 'o'};
    const ChunkMetaData cmd{0, 7, 7, 10, 10, {{0, 1}}};

    ASSERT_EQ(*snd.data(), *content);
    ASSERT_EQ(snd.size(), 5);
    ASSERT_EQ(snd.getMetaData(), cmd);
  }
}

TEST(DataChunk, getMetaData) {
  std::string tmp("hello");
  DataChunk str(tmp.data(), tmp.size(), {0, 7, 7, 10, 10, {{0, 1}}});

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
  std::string tmp("hello");
  DataChunk str(tmp.data(), tmp.size(), {0, 7, 7, 10, 10, {{0, 1}}});

  ASSERT_EQ(*str.data(), *tmp.data());
}

TEST(DataChunk, size) {
  std::string tmp("hello");
  DataChunk str(tmp.data(), tmp.size(), {0, 7, 7, 10, 10, {{0, 1}}});
  ASSERT_EQ(str.size(), 5);
}

TEST(DataChunk, assign) {
  DataChunk str({0, 7, 7, 10, 10, {{0, 1}}});
  str.assign("hello");
  char content[5] = {'h', 'e', 'l', 'l', 'o'};

  ASSERT_EQ(*str.data(), *content);

  str.assign("hello!");
  char content2[6] = {'h', 'e', 'l', 'l', 'o', '!'};
  ASSERT_EQ(*str.data(), *content2);
}

}  // namespace xs