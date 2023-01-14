// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/MetaFile.h>

#include <string>

namespace xs {

/**
 * @brief Compress a given file using Zstandard or LZ4 and create a
 * corresponding meta file readable for SFMetaFile.
 */
class FilePreprocessor {
 public:
  /**
   * @brief Compress a given file using Zstandard or LZ4 and create a
   * corresponding meta file readable for SFMetaFile.
   *
   * @param srcFile file system path to file that will be compressed (file stays
   * unchanged)
   * @param outFile file system path to output of compressed file
   * @param metaFile file system path to output of corresponding meta file
   * @param compressionAlg compression algorithm (either zstd or lz4 or empty
   * for no compression)
   * @param compressionLevel compression level
   * @param byteToNewLineMappingDistance byte distance if new line - byte
   * mappings stored in meta file, 0 -> no mappings stored
   * @param minChunkSize size of one chunk of the file that is compressed
   * @param maxOverflowSize maximum number of additional bytes when looking for
   * next new line char
   * @param progress whether to display progress or don't
   */
  static void preprocess(const std::string& srcFile, const std::string& outFile,
                         const std::string& metaFile,
                         CompressionType compressionAlg = NONE,
                         int compressionLevel = 1,
                         uint64_t byteToNewLineMappingDistance = 500,
                         size_t minChunkSize = (1 << 24),
                         size_t maxOverflowSize = 1024 * 1024,
                         bool progress = false);
};

}  // namespace xs
