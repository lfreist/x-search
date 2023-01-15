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
size_t ExternSearcher<restype::count>::search(const std::string& file_path,
                                              const std::string& meta_file_path,
                                              const std::string& pattern,
                                              int num_threads,
                                              int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::count> result_collector{};
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
      xs::pipeline::CollectorTask<size_t>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> size_t {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
size_t ExternSearcher<restype::count_lines>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::count_lines> result_collector{};
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
      xs::pipeline::CollectorTask<size_t>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> size_t {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
std::vector<size_t> ExternSearcher<restype::byte_positions>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::byte_positions> result_collector{};
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
      [&searcher](auto&& PH1) { searcher.byte_offsets_match(PH1, false); });

  xs::pipeline::TaskManager<std::vector<size_t>> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask<std::vector<size_t>>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> std::vector<size_t> {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
std::vector<size_t> ExternSearcher<restype::line_numbers>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::line_numbers> result_collector{};
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
      [&searcher](auto&& PH1) { searcher.byte_offsets_line(PH1); });
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.line_indices(std::forward<decltype(PH1)>(PH1));
  });

  xs::pipeline::TaskManager<std::vector<size_t>> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask<std::vector<size_t>>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> std::vector<size_t> {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
std::vector<size_t> ExternSearcher<restype::line_indices>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::line_indices> result_collector{};
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
      [&searcher](auto&& PH1) { searcher.byte_offsets_line(PH1); });
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.line_indices(std::forward<decltype(PH1)>(PH1));
  });

  xs::pipeline::TaskManager<std::vector<size_t>> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask<std::vector<size_t>>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> std::vector<size_t> {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
std::vector<std::string> ExternSearcher<restype::lines>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::lines> result_collector{};
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
      [&searcher](auto&& PH1) { searcher.byte_offsets_line(PH1); });
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.line(std::forward<decltype(PH1)>(PH1));
  });

  xs::pipeline::TaskManager<std::vector<std::string>> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask<std::vector<std::string>>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> std::vector<std::string> {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

template <>
std::vector<SearchResults> ExternSearcher<restype::full>::search(
    const std::string& file_path, const std::string& meta_file_path,
    const std::string& pattern, int num_threads, int max_readers) {
  xs::reader::BlockReader reader(file_path, meta_file_path, 10);
  xs::searcher::Searcher searcher(pattern,
                                  xs::utils::use_str_as_regex(pattern));
  xs::Result<restype::full> result_collector{};
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

  // required search tasks ---------------------------------------------------
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.byte_offsets_match(std::forward<decltype(PH1)>(PH1), false);
  });
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.line_indices(std::forward<decltype(PH1)>(PH1));
  });
  processors.emplace_back([&searcher](auto&& PH1) {
    searcher.line(std::forward<decltype(PH1)>(PH1));
  });
  // -------------------------------------------------------------------------

  xs::pipeline::TaskManager<std::vector<SearchResults>> task_manager(
      xs::pipeline::ProducerTask([&reader] { return reader.read(); },
                                 max_readers),
      std::move(processors),
      xs::pipeline::CollectorTask<std::vector<SearchResults>>(
          [&result_collector](auto&& PH1) {
            result_collector.addPartialResult(std::forward<decltype(PH1)>(PH1));
          },
          [&result_collector]() -> std::vector<SearchResults> {
            return result_collector.getResult();
          }));
  task_manager.execute(num_threads);
  return task_manager.join();
}

}  // namespace xs