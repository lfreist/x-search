// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/MetaFile.h>

namespace xs {

TEST(MetaFileTest, constructor) {
  {
    MetaFile meta_file("test/files/dummy.xs.meta", std::ios::in);

    ASSERT_EQ(meta_file._filePath, "test/files/dummy.xs.meta");
    ASSERT_EQ(meta_file._openMode, std::ios::in | std::ios::binary);
    ASSERT_TRUE(meta_file._metaFileStream.is_open());
  }
  {
    MetaFile meta_file("test/files/dummy.meta.tmp", std::ios::out);

    ASSERT_EQ(meta_file._filePath, "test/files/dummy.meta.tmp");
    ASSERT_EQ(meta_file._openMode, std::ios::out | std::ios::binary);
    ASSERT_TRUE(meta_file._metaFileStream.is_open());
  }
}

TEST(MetaFileTest, nextChunkMetaData) {
  MetaFile meta_file("test/files/dummy.xslz4.meta", std::ios::in);

  std::optional<ChunkMetaData> chunk_size = meta_file.nextChunkMetaData();
  ASSERT_TRUE(chunk_size.has_value());
  ChunkMetaData cs = chunk_size.value();
  ASSERT_EQ(cs.chunk_index, 0);
  ASSERT_EQ(cs.original_offset, 0);
  ASSERT_EQ(cs.actual_offset, 0);
  ASSERT_EQ(cs.original_size, 4497);
  ASSERT_EQ(cs.actual_size, 4050);

  chunk_size = meta_file.nextChunkMetaData();
  ASSERT_TRUE(chunk_size.has_value());
  cs = chunk_size.value();
  ASSERT_EQ(cs.chunk_index, 1);
  ASSERT_EQ(cs.original_offset, 4497);
  ASSERT_EQ(cs.actual_offset, 4050);
  ASSERT_EQ(cs.original_size, 4207);
  ASSERT_EQ(cs.actual_size, 3872);

  chunk_size = meta_file.nextChunkMetaData();
  ASSERT_TRUE(chunk_size.has_value());
  cs = chunk_size.value();
  ASSERT_EQ(cs.chunk_index, 2);
  ASSERT_EQ(cs.original_offset, 8704);
  ASSERT_EQ(cs.actual_offset, 7922);
  ASSERT_EQ(cs.original_size, 4216);
  ASSERT_EQ(cs.actual_size, 3835);

  // get last chunk
  while (true) {
    auto tmp = meta_file.nextChunkMetaData();
    if (!tmp.has_value()) {
      break;
    }
    cs = tmp.value();
  }
  ASSERT_EQ(cs.chunk_index, 42);
  ASSERT_EQ(cs.original_offset, 179001);
  ASSERT_EQ(cs.actual_offset, 162345);
  ASSERT_EQ(cs.original_size, 1407);
  ASSERT_EQ(cs.actual_size, 1354);
}

TEST(MetaFileTest, nextChunkMetaDataMultiple) {
  MetaFile meta_file("test/files/dummy.xslz4.meta", std::ios::in);

  std::vector<ChunkMetaData> chunk_sizes = meta_file.nextChunkMetaData(5);
  ASSERT_EQ(chunk_sizes.size(), 5);
  ASSERT_EQ(chunk_sizes[0].original_offset, 0);
  ASSERT_EQ(chunk_sizes[0].actual_offset, 0);
  ASSERT_EQ(chunk_sizes[0].original_size, 4497);
  ASSERT_EQ(chunk_sizes[0].actual_size, 4050);

  chunk_sizes = meta_file.nextChunkMetaData(150);
  ASSERT_EQ(chunk_sizes.size(), 38);
  ASSERT_EQ(chunk_sizes[37].original_offset, 179001);
  ASSERT_EQ(chunk_sizes[37].actual_offset, 162345);
  ASSERT_EQ(chunk_sizes[37].original_size, 1407);
  ASSERT_EQ(chunk_sizes[37].actual_size, 1354);
}

TEST(MetaFileTest, writeChunkMetaData) {
  ChunkMetaData original_cs_0{0, 0, 10, 100, 50, {{1, 2}, {2, 3}}};
  ChunkMetaData original_cs_1{0, 51, 100, 112, 42, {{4, 5}, {7, 2354}}};
  {  // write data to meta file
    MetaFile meta_file_write("test/files/tmp.meta", std::ios::out);
    meta_file_write.writeChunkMetaData(original_cs_0);
    meta_file_write.writeChunkMetaData(original_cs_1);
  }

  MetaFile meta_file_read("test/files/tmp.meta", std::ios::in);

  auto cs = meta_file_read.nextChunkMetaData().value();
  ASSERT_EQ(cs.original_offset, 0);
  ASSERT_EQ(cs.actual_offset, 10);
  ASSERT_EQ(cs.original_size, 100);
  ASSERT_EQ(cs.actual_size, 50);
  ASSERT_EQ(cs.line_mapping_data, original_cs_0.line_mapping_data);

  cs = meta_file_read.nextChunkMetaData().value();
  ASSERT_EQ(cs.original_offset, 51);
  ASSERT_EQ(cs.actual_offset, 100);
  ASSERT_EQ(cs.original_size, 112);
  ASSERT_EQ(cs.actual_size, 42);
  ASSERT_EQ(cs.line_mapping_data, original_cs_1.line_mapping_data);

  auto cs_opt = meta_file_read.nextChunkMetaData();
  ASSERT_FALSE(cs_opt.has_value());
}

}  // namespace xs