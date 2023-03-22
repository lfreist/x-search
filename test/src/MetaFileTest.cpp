// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/MetaFile.h>

#include <cstdio>

namespace xs {

TEST(MetaFileTest, constructor) {
  {
    MetaFile meta_file("test/files/sample.meta", std::ios::in);

    ASSERT_EQ(meta_file.get_file_path(), "test/files/sample.meta");
    ASSERT_FALSE(meta_file.is_writable());
  }
  {
    MetaFile meta_file("test/files/sample.meta.tmp", std::ios::out);

    ASSERT_EQ(meta_file.get_file_path(), "test/files/sample.meta.tmp");
    ASSERT_TRUE(meta_file.is_writable());

    std::remove("test/files/sample.meta.tmp");
  }
}

TEST(MetaFileTest, next_chunk_meta_data) {
  MetaFile meta_file("test/files/sample.xslz4.meta", std::ios::in);

  std::optional<ChunkMetaData> chunk_size = meta_file.next_chunk_meta_data();
  ASSERT_TRUE(chunk_size.has_value());
  ChunkMetaData cs = chunk_size.value();
  ASSERT_EQ(cs.chunk_index, 0);
  ASSERT_EQ(cs.original_offset, 0);
  ASSERT_EQ(cs.actual_offset, 0);
  ASSERT_EQ(cs.original_size, 16777222);
  ASSERT_EQ(cs.actual_size, 6661835);

  chunk_size = meta_file.next_chunk_meta_data();
  ASSERT_TRUE(chunk_size.has_value());
  cs = chunk_size.value();
  ASSERT_EQ(cs.chunk_index, 1);
  ASSERT_EQ(cs.original_offset, 16777222);
  ASSERT_EQ(cs.actual_offset, 6661835);
  ASSERT_EQ(cs.original_size, 16777261);
  ASSERT_EQ(cs.actual_size, 7090180);

  // get last chunk
  while (true) {
    auto tmp = meta_file.next_chunk_meta_data();
    if (!tmp.has_value()) {
      break;
    }
    cs = tmp.value();
  }
  ASSERT_EQ(cs.chunk_index, 5);
  ASSERT_EQ(cs.original_offset, 83886210);
  ASSERT_EQ(cs.actual_offset, 34833468);
  ASSERT_EQ(cs.original_size, 16113790);
  ASSERT_EQ(cs.actual_size, 7035946);
}

TEST(MetaFileTest, write_chunk_meta_data) {
  ChunkMetaData original_cs_0{0, 0, 10, 100, 50, {{1, 2}, {2, 3}}};
  ChunkMetaData original_cs_1{0, 51, 100, 112, 42, {{4, 5}, {7, 2354}}};
  {  // write data to meta file
    MetaFile meta_file_write("test/files/tmp.meta", std::ios::out);
    meta_file_write.write_chunk_meta_data(original_cs_0);
    meta_file_write.write_chunk_meta_data(original_cs_1);
  }

  MetaFile meta_file_read("test/files/tmp.meta", std::ios::in);

  auto cs = meta_file_read.next_chunk_meta_data().value();
  ASSERT_EQ(cs.original_offset, 0);
  ASSERT_EQ(cs.actual_offset, 10);
  ASSERT_EQ(cs.original_size, 100);
  ASSERT_EQ(cs.actual_size, 50);
  ASSERT_EQ(cs.line_mapping_data, original_cs_0.line_mapping_data);

  cs = meta_file_read.next_chunk_meta_data().value();
  ASSERT_EQ(cs.original_offset, 51);
  ASSERT_EQ(cs.actual_offset, 100);
  ASSERT_EQ(cs.original_size, 112);
  ASSERT_EQ(cs.actual_size, 42);
  ASSERT_EQ(cs.line_mapping_data, original_cs_1.line_mapping_data);

  auto cs_opt = meta_file_read.next_chunk_meta_data();
  ASSERT_FALSE(cs_opt.has_value());

  std::remove("test/files/tmp.meta");
}

TEST(MetaFile, get_compression_type) {
  {
    MetaFile meta_file("test/files/sample.meta", std::ios::in);
    ASSERT_EQ(meta_file.get_compression_type(), CompressionType::NONE);
  }
  {
    MetaFile meta_file("test/files/sample.xslz4.meta", std::ios::in);
    ASSERT_EQ(meta_file.get_compression_type(), CompressionType::LZ4);
  }
  {
    MetaFile meta_file("test/files/sample.xslz4hc.meta", std::ios::in);
    ASSERT_EQ(meta_file.get_compression_type(), CompressionType::LZ4);
  }
  {
    MetaFile meta_file("test/files/sample.xszst.meta", std::ios::in);
    ASSERT_EQ(meta_file.get_compression_type(), CompressionType::ZSTD);
  }
}

TEST(MetaFile, get_compression_type_static) {
  ASSERT_EQ(MetaFile::getCompressionType("test/files/sample.meta"),
            CompressionType::NONE);
  ASSERT_EQ(MetaFile::getCompressionType("test/files/sample.xslz4.meta"),
            CompressionType::LZ4);
  ASSERT_EQ(MetaFile::getCompressionType("test/files/sample.xslz4hc.meta"),
            CompressionType::LZ4);
  ASSERT_EQ(MetaFile::getCompressionType("test/files/sample.xszst.meta"),
            CompressionType::ZSTD);
}

}  // namespace xs