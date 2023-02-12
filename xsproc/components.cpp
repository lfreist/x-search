// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include "./components.h"

preprocess_result MetaDataCreator::search(const std::string& pattern,
                                          xs::DataChunk* data) const {
  return {std::move(data->getMetaData()), data};
}