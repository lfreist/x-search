// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/DataChunk.h>

namespace xs {

TEST(DataChunk, constructor) {
  {
    DataChunk str;

    ASSERT_TRUE(str._content.empty());
    ASSERT_EQ(str._offset, 0);
    ASSERT_EQ(str._originalSize, 0);
    ASSERT_EQ(str._index, 0);
    ASSERT_TRUE(str._byte_to_nl_mapping_data.empty());
  }
  {
    DataChunk str({'h', 'e', 'l', 'l', 'o'}, 5, {{0, 1}}, 1);

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};

    ASSERT_EQ(str._content, content);
    ASSERT_EQ(str._offset, 5);
    ASSERT_EQ(str._originalSize, 0);
    ASSERT_EQ(str._index, 1);
    ASSERT_EQ(str._byte_to_nl_mapping_data, mapping_data);
  }
  {
    DataChunk str({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};

    ASSERT_EQ(str._content, content);
    ASSERT_EQ(str._offset, 5);
    ASSERT_EQ(str._originalSize, 10);
    ASSERT_EQ(str._index, 1);
    ASSERT_EQ(str._byte_to_nl_mapping_data, mapping_data);
  }
  {
    DataChunk str(10);

    ASSERT_EQ(str._content.size(), 10);
    ASSERT_EQ(str._offset, 0);
    ASSERT_EQ(str._originalSize, 0);
    ASSERT_EQ(str._index, 0);
    ASSERT_TRUE(str._byte_to_nl_mapping_data.empty());
  }
  {
    DataChunk str(10, 1, {{0, 1}, {1, 2}}, 3);

    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}, {1, 2}};

    ASSERT_EQ(str._content.size(), 10);
    ASSERT_EQ(str._offset, 1);
    ASSERT_EQ(str._originalSize, 0);
    ASSERT_EQ(str._index, 3);
    ASSERT_EQ(str._byte_to_nl_mapping_data, mapping_data);
  }
  {
    DataChunk str(10, 15, 1, {{0, 1}, {1, 2}}, 3);

    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}, {1, 2}};

    ASSERT_EQ(str._content.size(), 10);
    ASSERT_EQ(str._offset, 1);
    ASSERT_EQ(str._originalSize, 15);
    ASSERT_EQ(str._index, 3);
    ASSERT_EQ(str._byte_to_nl_mapping_data, mapping_data);
  }
}

TEST(DataChunk, MoveConstructor) {
  {
    DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

    auto snd(std::move(fst));

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};

    ASSERT_EQ(snd._content, content);
    ASSERT_EQ(snd._offset, 5);
    ASSERT_EQ(snd._originalSize, 10);
    ASSERT_EQ(snd._index, 1);
    ASSERT_EQ(snd._byte_to_nl_mapping_data, mapping_data);
  }
  {
    DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

    auto snd = std::move(fst);

    const strtype content{'h', 'e', 'l', 'l', 'o'};
    const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};

    ASSERT_EQ(snd._content, content);
    ASSERT_EQ(snd._offset, 5);
    ASSERT_EQ(snd._originalSize, 10);
    ASSERT_EQ(snd._index, 1);
    ASSERT_EQ(snd._byte_to_nl_mapping_data, mapping_data);
  }
}

TEST(DataChunk, str) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

  const strtype content{'h', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(fst.str(), content);
}

TEST(DataChunk, getOffset) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.getOffset(), 5);
}

TEST(DataChunk, getOriginalSize) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.getOriginalSize(), 10);
}

TEST(DataChunk, setOffset) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.getOffset(), 5);

  fst.setOffset(15);
  ASSERT_EQ(fst.getOffset(), 15);
}

TEST(DataChunk, setOriginalSize) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.getOriginalSize(), 10);

  fst.setOriginalSize(15);
  ASSERT_EQ(fst.getOriginalSize(), 15);
}

TEST(DataChunk, data) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

  ASSERT_EQ(fst.data(), fst._content.data());
}

TEST(DataChunk, size) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.size(), 5);
  ASSERT_EQ(fst._content.size(), 5);
}

TEST(DataChunk, resize) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.size(), 5);
  ASSERT_EQ(fst._content.size(), 5);

  fst.resize(10);
  ASSERT_EQ(fst.size(), 10);
  ASSERT_EQ(fst._content.size(), 10);
}

TEST(DataChunk, getMappingData) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);

  const std::vector<ByteToNewLineMappingInfo> mapping_data{{0, 1}};
  ASSERT_EQ(fst.getMappingData(), mapping_data);
}

TEST(DataChunk, push_back) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  const strtype content{'h', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(fst._content, content);

  fst.push_back('!');
  const strtype content2{'h', 'e', 'l', 'l', 'o', '!'};
  ASSERT_EQ(fst._content, content2);
}

TEST(DataChunk, assign) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  const strtype content{'h', 'e', 'l', 'l', 'o'};
  ASSERT_EQ(fst._content, content);

  fst.assign("hello!");
  const strtype content2{'h', 'e', 'l', 'l', 'o', '!'};
  ASSERT_EQ(fst._content, content2);
}

TEST(DataChunk, getIndex) {
  DataChunk fst({'h', 'e', 'l', 'l', 'o'}, 10, 5, {{0, 1}}, 1);
  ASSERT_EQ(fst.getIndex(), 1);
}

}  // namespace xs