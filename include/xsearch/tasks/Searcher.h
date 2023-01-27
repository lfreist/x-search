// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <re2/re2.h>
#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/utils/TSQueue.h>

namespace xs::tasks {

template <class DataT, class PartResT>
class BaseSearcher {
 public:
  BaseSearcher() = default;
  virtual ~BaseSearcher() = default;

  virtual void search(const std::string& pattern, DataT* data,
                      PartResT* result) const = 0;
  virtual void search(re2::RE2* pattern, DataT* data,
                      PartResT* result) const = 0;
};

template <class PartResT>
class MatchCounter : public BaseSearcher<DataChunk, PartResT> {
 public:
  MatchCounter() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

template <class PartResT>
class LineCounter : public BaseSearcher<DataChunk, PartResT> {
 public:
  LineCounter() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

template <class PartResT>
class MatchBytePositionSearcher : public BaseSearcher<DataChunk, PartResT> {
 public:
  MatchBytePositionSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

template <class PartResT>
class LineBytePositionSearcher : public BaseSearcher<DataChunk, PartResT> {
 public:
  LineBytePositionSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

template <class PartResT>
class LineIndexSearcher : public BaseSearcher<DataChunk, PartResT> {
 public:
  LineIndexSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

template <class PartResT>
class LineSearcher : public BaseSearcher<DataChunk, PartResT> {
 public:
  LineSearcher() = default;

  void search(const std::string& pattern, DataChunk* data,
              PartResT* result) const override;
  void search(re2::RE2* pattern, DataChunk* data,
              PartResT* result) const override;
};

}  // namespace xs::tasks