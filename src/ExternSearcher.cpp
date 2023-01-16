// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/ExternSearcher.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/pipeline/Task.h>
#include <xsearch/pipeline/TaskManager.h>
#include <xsearch/pipeline/processors/decompressors.h>
#include <xsearch/pipeline/reader/BlockReader.h>
#include <xsearch/pipeline/result_collectors/MergeSearchResults.h>
#include <xsearch/pipeline/searchers/Searchers.h>
#include <xsearch/utils/utils.h>

#include <string>

namespace xs {

CompressionType get_compression_type(MetaFile meta_file) {
  return meta_file.getCompressionType();
}

template <>
std::shared_ptr<YieldResult<restype::count>>
ExternSearcher<restype::count>::search(const std::string& file_path,
                                       const std::string& meta_file_path,
                                       const std::string& pattern,
                                       int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<xs::YieldResult<restype::count>> result_collector =
      std::make_shared<xs::YieldResult<restype::count>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.join();
  return result_collector;
}

template <>
std::shared_ptr<YieldResult<restype::count_lines>>
ExternSearcher<restype::count_lines>::search(const std::string& file_path,
                                             const std::string& meta_file_path,
                                             const std::string& pattern,
                                             int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<xs::YieldResult<restype::count_lines>> result_collector =
      std::make_shared<xs::YieldResult<restype::count_lines>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back(
      [&searcher](auto&& PH1) { searcher.count_lines(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

template <>
std::shared_ptr<YieldResult<restype::byte_positions>>
ExternSearcher<restype::byte_positions>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<YieldResult<restype::byte_positions>> result_collector =
      std::make_shared<YieldResult<restype::byte_positions>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

template <>
std::shared_ptr<xs::YieldResult<restype::line_numbers>>
ExternSearcher<restype::line_numbers>::search(const std::string& file_path,
                                              const std::string& meta_file_path,
                                              const std::string& pattern,
                                              int num_threads,
                                              int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<YieldResult<restype::line_numbers>> result_collector =
      std::make_shared<YieldResult<restype::line_numbers>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

template <>
std::shared_ptr<xs::YieldResult<restype::line_indices>>
ExternSearcher<restype::line_indices>::search(const std::string& file_path,
                                              const std::string& meta_file_path,
                                              const std::string& pattern,
                                              int num_threads,
                                              int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<YieldResult<restype::line_indices>> result_collector =
      std::make_shared<YieldResult<restype::line_indices>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

template <>
std::shared_ptr<xs::YieldResult<restype::lines>>
ExternSearcher<restype::lines>::search(const std::string& file_path,
                                       const std::string& meta_file_path,
                                       const std::string& pattern,
                                       int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<YieldResult<restype::lines>> result_collector =
      std::make_shared<YieldResult<restype::lines>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

template <>
std::shared_ptr<xs::YieldResult<restype::full>>
ExternSearcher<restype::full>::search(const std::string& file_path,
                                      const std::string& meta_file_path,
                                      const std::string& pattern,
                                      int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  // we create a shared pointer here because we want to return the result
  //  so that the partial results can be accessed via the iterator
  std::shared_ptr<YieldResult<restype::full>> result_collector =
      std::make_shared<YieldResult<restype::full>>();

  std::vector<xs::pipeline::ProcessingTask> processors;

  switch (get_compression_type(MetaFile(meta_file_path, std::ios::in))) {
    case xs::CompressionType::LZ4:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_lz4(PH1); });
      break;
    case xs::CompressionType::ZSTD:
      processors.emplace_back(
          [](auto&& PH1) { xs::processors::decompress::using_zstd(PH1); });
      break;
    default:
      break;
  }

  processors.emplace_back([&searcher](auto&& PH1) { searcher.count(PH1); });

  xs::pipeline::TaskManager<size_t> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask(
          [&result_collector](auto&& PH1) {
            result_collector->addPartialResult(
                std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() { result_collector->markAsDone(); }));
  task_manager.execute(num_threads);
  task_manager.detach();
  return result_collector;
}

}  // namespace xs