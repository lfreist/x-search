// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./components.h"

// ----- MetaDataCreator -------------------------------------------------------
// _____________________________________________________________________________
preprocess_result MetaDataCreator::process(xs::DataChunk* data) const {
  return {std::move(data->getMetaData()), data};
}

// ----- DataWriter ------------------------------------------------------------
// _____________________________________________________________________________
DataWriter::DataWriter(const std::string& meta_file_path,
                       xs::CompressionType compressionType,
                       std::unique_ptr<std::ostream> out_stream)
    : _meta_file(meta_file_path, std::ios::out, compressionType),
      _output_stream(std::move(out_stream)) {
}

// _____________________________________________________________________________
void DataWriter::add(preprocess_result data, uint64_t id) {
  std::unique_lock lock(*this->_mutex);
  if (_current_index == id) {
    add(std::move(data));
    _current_index++;
    while (true) {
      auto search = _buffer.find(_current_index);
      if (search == _buffer.end()) {
        break;
      }
      add(std::move(search->second));
      _buffer.erase(_current_index);
      _current_index++;
    }
    // at least one partial_result was added -> notify
    this->_cv->notify_all();
  } else {
    // buffer the partial result
    _buffer.insert({id, std::move(data)});
  }
}

// _____________________________________________________________________________
void DataWriter::add(preprocess_result data) {
  if (_output_stream != nullptr) {
    // write data to output file
    _output_stream->write(data.second->data(), static_cast<int64_t>(data.second->size()));
  }
  _meta_file.writeChunkMetaData(data.first);
}