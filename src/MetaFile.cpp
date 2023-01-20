// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/MetaFile.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace xs {

// _____________________________________________________________________________
CompressionType from_string(const std::string& ct) {
  std::string lower_case;
  lower_case.resize(ct.size());
  std::transform(ct.begin(), ct.end(), lower_case.begin(), ::tolower);
  if (lower_case == "zstd" || lower_case == "zst") {
    return ZSTD;
  } else if (lower_case == "lz4") {
    return LZ4;
  } else if (lower_case == "none") {
    return NONE;
  }
  return UNKNOWN;
}

// _____________________________________________________________________________
std::string to_string(const CompressionType& ct) {
  switch (ct) {
    case UNKNOWN:
      return "undef";
    case NONE:
      return "";
    case ZSTD:
      return "zst";
    case LZ4:
      return "lz4";
    default:
      return "undef";
  }
}

// _____________________________________________________________________________
void ChunkMetaData::serialize(std::fstream* stream) const {
  stream->write(reinterpret_cast<const char*>(&original_offset),
                sizeof(original_offset));
  stream->write(reinterpret_cast<const char*>(&actual_offset),
                sizeof(actual_offset));
  stream->write(reinterpret_cast<const char*>(&original_size),
                sizeof(original_size));
  stream->write(reinterpret_cast<const char*>(&actual_size),
                sizeof(actual_size));
  size_t size = line_mapping_data.size();
  stream->write(reinterpret_cast<const char*>(&size), sizeof(size));
  if (size == 0) {
    return;
  }
  stream->write(reinterpret_cast<const char*>(line_mapping_data.data()),
                line_mapping_data.size() * sizeof(ByteToNewLineMappingInfo));
}

// _____________________________________________________________________________
ChunkMetaData readChunkMetaData(std::fstream* stream) {
  ChunkMetaData cmd{0, 0, 0, 0, {}};
  stream->read(reinterpret_cast<char*>(&cmd.original_offset),
               sizeof(cmd.original_offset));
  if (stream->eof()) {
    return cmd;
  }
  stream->read(reinterpret_cast<char*>(&cmd.actual_offset),
               sizeof(cmd.actual_offset));
  stream->read(reinterpret_cast<char*>(&cmd.original_size),
               sizeof(cmd.original_size));
  stream->read(reinterpret_cast<char*>(&cmd.actual_size),
               sizeof(cmd.actual_size));
  size_t sizeNewLineVector;
  stream->read(reinterpret_cast<char*>(&sizeNewLineVector),
               sizeof(sizeNewLineVector));
  if (sizeNewLineVector == 0) {
    return cmd;
  }
  cmd.line_mapping_data.resize(sizeNewLineVector);
  stream->read(reinterpret_cast<char*>(cmd.line_mapping_data.data()),
               sizeNewLineVector * sizeof(ByteToNewLineMappingInfo));
  return cmd;
}

// _____________________________________________________________________________
MetaFile::MetaFile(const std::string& filePath, std::ios::openmode mode,
                   CompressionType compression_type) {
  _filePath = filePath;
  _openMode = mode | std::ios::binary;
  _metaFileStream.open(_filePath, _openMode);
  if (!_metaFileStream.is_open()) {
    throw std::runtime_error("Cannot open file '" + _filePath + "'");
  }
  _compressionType = compression_type;
  if (mode == std::ios::out) {
    _compressionType = _compressionType == UNKNOWN ? NONE : _compressionType;
    _metaFileStream.write(reinterpret_cast<char*>(&_compressionType),
                          sizeof(_compressionType));
  }
  if (mode == std::ios::in && _compressionType == UNKNOWN) {
    _metaFileStream.read(reinterpret_cast<char*>(&_compressionType),
                         sizeof(_compressionType));
  }
}

// _____________________________________________________________________________
MetaFile::MetaFile(MetaFile&& XSMetaFile) noexcept
    : _metaFileStream(std::move(XSMetaFile._metaFileStream)) {
  XSMetaFile._getChunkMutex.lock();
  _filePath = std::move(XSMetaFile._filePath);
  _openMode = XSMetaFile._openMode;
  _compressionType = XSMetaFile._compressionType;
  XSMetaFile._getChunkMutex.unlock();
}

// _____________________________________________________________________________
MetaFile::~MetaFile() { _metaFileStream.close(); }

// _____________________________________________________________________________
std::optional<ChunkMetaData> MetaFile::nextChunkMetaData() {
  std::unique_lock _getChunkLock(_getChunkMutex);
  ChunkMetaData cs = readChunkMetaData(&_metaFileStream);
  if (_metaFileStream.eof()) {
    return {};
  }
  return cs;
}

// _____________________________________________________________________________
std::vector<ChunkMetaData> MetaFile::nextChunkMetaData(uint32_t num) {
  std::unique_lock _getChunkLock(_getChunkMutex);
  std::vector<ChunkMetaData> cs;
  cs.reserve(num);
  for (uint32_t i = 0; i < num; ++i) {
    auto cmd = readChunkMetaData(&_metaFileStream);
    if (_metaFileStream.eof()) {
      break;
    }
    cs.emplace_back(cmd);
  }
  return cs;
}

// _____________________________________________________________________________
void MetaFile::writeChunkMetaData(const ChunkMetaData& chunk) {
  assert(_openMode == (std::ios::out | std::ios::binary));
  assert(_compressionType != UNKNOWN);
  chunk.serialize(&_metaFileStream);
}

// _____________________________________________________________________________
CompressionType MetaFile::getCompressionType() const {
  return _compressionType;
}

// ----- static ----------------------------------------------------------------
// _____________________________________________________________________________
CompressionType MetaFile::getCompressionType(const std::string& metaFilePath) {
  std::ifstream metaFileStream(metaFilePath);
  if (!metaFileStream.is_open()) {
    throw std::runtime_error("Cannot open file '" + metaFilePath + "'.");
  }
  CompressionType compressionType(UNKNOWN);
  metaFileStream.read(reinterpret_cast<char*>(&compressionType),
                      sizeof(_compressionType));
  metaFileStream.close();
  return compressionType;
}

}  // namespace xs
