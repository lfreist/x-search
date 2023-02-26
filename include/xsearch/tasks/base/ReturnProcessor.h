// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

namespace xs::task::base {

template <typename T, typename R>
class ReturnProcessor {
 public:
  ReturnProcessor() = default;
  virtual ~ReturnProcessor() = default;

  virtual R process(T* data) const = 0;
};

}  // namespace xs::task::base