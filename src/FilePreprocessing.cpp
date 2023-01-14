// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/FilePreprocessing.h>
#include <xsearch/MetaFile.h>
#include <xsearch/utils/compression/Lz4Wrapper.h>
#include <xsearch/utils/compression/ZstdWrapper.h>

#include <cmath>
#include <filesystem>
#include <iostream>

namespace xs {

// _____________________________________________________________________________
void FilePreprocessor::preprocess(
    const std::string& srcFile, const std::string& outFile,
    const std::string& metaFile, CompressionType compressionAlg,
    int compressionLevel, uint64_t byteToNewLineMappingDistance,
    size_t minChunkSize, size_t maxOverflowSize, bool progress) {
  if (compressionAlg == UNKNOWN) {
    compressionAlg = NONE;
  }
  // LZ4 compression requires the size of the data that are compressed to be
  //  stored as an integer. therefor, we might need to adjust the maximal size
  //  of the read buffer:
  if (compressionAlg == LZ4) {
    if (maxOverflowSize >= std::numeric_limits<int>::max()) {
      maxOverflowSize = std::numeric_limits<int>::max() - (1024 * 1024);
    }
    if (minChunkSize + maxOverflowSize > std::numeric_limits<int>::max()) {
      std::cout << "Warning: decreasing minChunkSize because LZ4 only handles "
                   "chunks with integer sizes..."
                << std::endl;
      minChunkSize = std::numeric_limits<int>::max() - maxOverflowSize;
    }
  }
  std::ifstream in_stream(srcFile);
  if (!in_stream.is_open()) {
    throw std::runtime_error("Could not read source file '" + srcFile + "'...");
  }
  MetaFile meta_file(metaFile, std::ios::out, compressionAlg);

  auto fileSize = std::filesystem::file_size(std::filesystem::path(srcFile));

  size_t write_offset = 0;  // actual offset
  size_t read_offset = 0;   // original offset

  size_t original_offset = 0;
  size_t actual_offset = 0;

  if (compressionAlg == NONE) {
    // read file line by line and safe new line index all x bytes in metafile
    uint64_t line_index = 0;
    std::string line;
    size_t buffer_length = 0;
    size_t byte_count = 0;
    std::vector<ByteToNewLineMappingInfo> byteNLMappingVector;
    // We read the file line by line. This has the effect that for each line
    // at most one new line mapping data is collected. Thus, the
    // byteToNewLineMappingDistance does not indicate an exact byte offset of
    // mapping data collected but a minimum distance.
    while (getline(in_stream, line)) {
      if (progress) {  // print progress bar
        double p =
            static_cast<double>(read_offset) / static_cast<double>(fileSize);
        if (p > 1) {
          p = 1;
        }
        std::cout << "\r" << std::round(p * 100) << "%" << std::flush;
      }
      line.push_back('\n');  // nl char is not read by getline()
      buffer_length += line.size();
      byte_count += line.size();
      read_offset += line.size();
      write_offset += line.size();
      if (buffer_length >= minChunkSize) {
        if (buffer_length > minChunkSize + maxOverflowSize) {
          throw std::runtime_error(
              "Could not find new line. Increase maxOverflowSize value.");
        }
        // we save the first byte of a line together with its line index
        // rather than the last byte of the previous line. However, this has
        // no technical reasons and is just a design decision because it is
        // more intuitive...
        byteNLMappingVector.push_back({read_offset + 1, line_index + 1});
        ChunkMetaData cmd{original_offset, actual_offset, buffer_length,
                          buffer_length, byteNLMappingVector};
        meta_file.writeChunkMetaData(cmd);
        original_offset = read_offset;
        actual_offset = write_offset;
        buffer_length = 0;
        byte_count = 0;
        byteNLMappingVector.clear();
      } else if (byte_count >= byteToNewLineMappingDistance) {
        // we save the first byte of a line together with its line index
        // rather than the last byte of the previous line. However, this has
        // no technical reasons and is just a design decision because it is
        // more intuitive...
        byteNLMappingVector.push_back({read_offset + 1, line_index + 1});
        byte_count = 0;
      }
      line_index++;
    }
    byteNLMappingVector.push_back({read_offset, line_index});
    ChunkMetaData cmd{original_offset, actual_offset, buffer_length,
                      buffer_length, byteNLMappingVector};
    meta_file.writeChunkMetaData(cmd);
  } else {
    std::ofstream out_stream(outFile.empty()
                                 ? srcFile + ".sf" + to_string(compressionAlg)
                                 : outFile);
    if (!out_stream.is_open()) {
      throw std::runtime_error("Could not write out file '" + outFile + "'...");
    }
    // read file line by line, compress in blocks and store new line indices
    uint64_t line_index = 0;
    std::string line;
    size_t byte_count = 0;
    std::vector<ByteToNewLineMappingInfo> byteNLMappingVector;
    // this is the max number of values of byte-NL-mappings
    byteNLMappingVector.reserve((maxOverflowSize + minChunkSize) /
                                byteToNewLineMappingDistance);
    std::string buffer;
    std::vector<char> compressed;
    while (getline(in_stream, line)) {
      if (progress) {
        double p =
            static_cast<double>(read_offset) / static_cast<double>(fileSize);
        if (p > 1) {
          p = 1;
        }
        std::cout << "\r" << std::round(p * 100) << "%" << std::flush;
      }
      line.push_back('\n');
      buffer.append(line);
      byte_count += line.size();
      read_offset += line.size();
      if (buffer.size() >= minChunkSize) {
        if (buffer.size() > minChunkSize + maxOverflowSize) {
          throw std::runtime_error(
              "Could not find new line. Increase maxOverflowSize value.");
        }
        if (compressionAlg == ZSTD) {
          compressed = xs::utils::compression::ZSTD::compress(
              buffer.data(), buffer.size(), compressionLevel);
        } else if (compressionAlg == LZ4) {
          compressed = xs::utils::compression::LZ4::compress(
              buffer.data(), static_cast<int>(buffer.size()), compressionLevel);
        }
        write_offset += compressed.size();
        byteNLMappingVector.push_back({read_offset + 1, line_index + 1});
        ChunkMetaData cmd{original_offset, actual_offset, buffer.size(),
                          compressed.size(), byteNLMappingVector};
        out_stream.write(compressed.data(),
                         static_cast<std::streamsize>(compressed.size()));
        meta_file.writeChunkMetaData(cmd);
        buffer.clear();
        compressed.clear();
        byte_count = 0;
        byteNLMappingVector.clear();
        original_offset = read_offset;
        actual_offset = write_offset;
      } else if (byte_count >= byteToNewLineMappingDistance) {
        byteNLMappingVector.push_back({read_offset + 1, line_index + 1});
        byte_count = 0;
      }
      line_index++;
    }
    if (!buffer.empty()) {
      if (compressionAlg == ZSTD) {
        compressed = xs::utils::compression::ZSTD::compress(
            buffer.data(), buffer.size(), compressionLevel);
      } else if (compressionAlg == LZ4) {
        compressed = xs::utils::compression::LZ4::compress(
            buffer.data(), static_cast<int>(buffer.size()), compressionLevel);
      }
      ChunkMetaData cmd{original_offset, actual_offset, buffer.size(),
                        compressed.size(), byteNLMappingVector};
      out_stream.write(compressed.data(),
                       static_cast<std::streamsize>(compressed.size()));
      meta_file.writeChunkMetaData(cmd);
    }
    out_stream.close();
  }
  in_stream.close();
  std::cout << std::endl;
}

}  // namespace xs
