// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/tasks/defaults/processors.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>

static const std::string data(
    "This are the compressed data.\nsome data are repeated for compression.\n"
    "we will also consider some new lines\nsince we want to use these data "
    "for the NewLineSearcher too.\nlets go!");

TEST(LZ4Decompressor, process) {
  {
    auto compressed =
        xs::utils::compression::LZ4::compress(data.data(), data.size());
    xs::DataChunk chunk(compressed.data(), compressed.size());
    chunk.getMetaData().original_size = data.size();
    chunk.getMetaData().actual_size = compressed.size();

    ASSERT_NE(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));

    xs::task::processor::LZ4Decompressor decompressor;
    decompressor.process(&chunk);

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));
  }
  {
    auto compressed =
        xs::utils::compression::LZ4::compress(data.data(), data.size(), true);
    xs::DataChunk chunk(compressed.data(), compressed.size());
    chunk.getMetaData().original_size = data.size();
    chunk.getMetaData().actual_size = compressed.size();

    ASSERT_NE(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));

    xs::task::processor::LZ4Decompressor decompressor;
    decompressor.process(&chunk);

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));
  }
}

TEST(ZSTDDecompressor, process) {
  auto compressed =
      xs::utils::compression::ZSTD::compress((void*)data.data(), data.size());
  xs::DataChunk chunk(compressed.data(), compressed.size());
  chunk.getMetaData().original_size = data.size();
  chunk.getMetaData().actual_size = compressed.size();

  ASSERT_NE(std::string(chunk.data(), chunk.size()),
            std::string(data.data(), data.size()));

  xs::task::processor::ZSTDDecompressor decompressor;
  decompressor.process(&chunk);

  ASSERT_EQ(std::string(chunk.data(), chunk.size()),
            std::string(data.data(), data.size()));
}

TEST(LZ4Compressor, process) {
  {
    auto compressed =
        xs::utils::compression::LZ4::compress(data.data(), data.size());
    xs::DataChunk chunk(data.data(), data.size());
    chunk.getMetaData().original_size = data.size();
    chunk.getMetaData().actual_size = data.size();

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));

    xs::task::processor::LZ4Compressor compressor;
    compressor.process(&chunk);

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(compressed.data(), compressed.size()));
  }
  {
    auto compressed =
        xs::utils::compression::LZ4::compress(data.data(), data.size(), true);
    xs::DataChunk chunk(data.data(), data.size());
    chunk.getMetaData().original_size = data.size();
    chunk.getMetaData().actual_size = data.size();

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(data.data(), data.size()));

    xs::task::processor::LZ4Compressor compressor(true);
    compressor.process(&chunk);

    ASSERT_EQ(std::string(chunk.data(), chunk.size()),
              std::string(compressed.data(), compressed.size()));
  }
}

TEST(ZSTDCompressor, process) {
  auto compressed =
      xs::utils::compression::ZSTD::compress((void*)data.data(), data.size());
  xs::DataChunk chunk(data.data(), data.size());
  chunk.getMetaData().original_size = data.size();
  chunk.getMetaData().actual_size = data.size();

  ASSERT_EQ(std::string(chunk.data(), chunk.size()),
            std::string(data.data(), data.size()));

  xs::task::processor::ZSTDCompressor compressor;
  compressor.process(&chunk);

  ASSERT_EQ(std::string(chunk.data(), chunk.size()),
            std::string(compressed.data(), compressed.size()));
}

TEST(NewLineSearcher, process) {
  {
    xs::DataChunk chunk(data.data(), data.size());
    chunk.getMetaData().actual_size = data.size();
    chunk.getMetaData().original_size = data.size();
    xs::task::processor::NewLineSearcher nls(5);

    nls.process(&chunk);

    std::vector<xs::ByteToNewLineMappingInfo> md{
        {0, 0}, {30, 1}, {70, 2}, {107, 3}, {168, 4}};

    ASSERT_EQ(chunk.getMetaData().line_mapping_data, md);
  }
  {
    xs::DataChunk chunk(data.data(), data.size());
    chunk.getMetaData().actual_size = data.size();
    chunk.getMetaData().original_size = data.size();
    xs::task::processor::NewLineSearcher nls(50);

    nls.process(&chunk);

    std::vector<xs::ByteToNewLineMappingInfo> md{{0, 0}, {70, 2}, {168, 4}};

    ASSERT_EQ(chunk.getMetaData().line_mapping_data, md);
  }
  {
    xs::DataChunk chunk(data.data(), data.size());
    chunk.getMetaData().actual_size = data.size();
    chunk.getMetaData().original_size = data.size();
    xs::task::processor::NewLineSearcher nls(500);

    nls.process(&chunk);

    std::vector<xs::ByteToNewLineMappingInfo> md{{0, 0}};

    ASSERT_EQ(chunk.getMetaData().line_mapping_data, md);
  }
}