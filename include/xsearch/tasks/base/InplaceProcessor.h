// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

namespace xs::task::base {

/**
 * BaseInplaceProcessor: The base processor class that must be inherited by all
 * classes used as processors within the xs::Executor.
 *
 * @tparam DataT
 */
template <typename T>
class InplaceProcessor {
 public:
  InplaceProcessor() = default;
  virtual ~InplaceProcessor() = default;

  virtual void process(T* data) = 0;
};

}  // namespace xs::task::base