// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>

#include "./components.h"

namespace po = boost::program_options;

struct ProcArgs {
  int num_threads = 1;
  std::string source_file;
  std::string output_file;
  std::string meta_file;
  std::string compression_alg;
  int compression_level = 0;
  bool hc = false;
  size_t min_chunk_size = 16777216;
  uint64_t mapping_data_distance = 500;
};

int main(int argc, char** argv) {
  ProcArgs args;

  po::options_description options("Options for xsproc");
  po::positional_options_description positional_options;

  // wrappers for adding command line arguments --------------------------------
  auto add_positional =
      [&positional_options]<typename... Args>(Args&&... args) {
        positional_options.add(std::forward<Args>(args)...);
      };
  auto add = [&options]<typename... Args>(Args&&... args) {
    options.add_options()(std::forward<Args>(args)...);
  };

  // defining possible command line arguments ----------------------------------
  add_positional("input-file", 1);
  add_positional("output-file", 1);
  add("help,h", "Produces this help message.");
  add("input-file", po::value<std::string>(&args.source_file),
      "path to input-file");
  add("output-file,o",
      po::value<std::string>(&args.output_file)->default_value(""),
      "path to output-file (gets overwritten)");
  add("meta-file,m", po::value<std::string>(&args.meta_file)->default_value(""),
      "path to meta-file (gets overwritten)");
  add("compression-alg,a",
      po::value<std::string>(&args.compression_alg)->default_value("none"),
      "Compression alg (zstd, lz4, none (default))");
  add("compression-level,l",
      po::value<int>(&args.compression_level)->default_value(0),
      "compression level (default: lz4 (only available if used with --hc): 1, "
      "zstd: 3)");
  add("hc", po::bool_switch(&args.hc),
      "use high compression algorithm. Only available for lz4.");
  add("chunk-size,s",
      po::value<uint64_t>(&args.min_chunk_size)->default_value(16777216),
      "number of threads");
  add("threads,j", po::value<int>(&args.num_threads)->default_value(1),
      "size of one chunk that is read");
  add("bytes-nl-distance,d",
      po::value<uint64_t>(&args.mapping_data_distance)->default_value(500),
      "number of bytes between new lines that are stored in meta file");

  po::variables_map optionsMap;

  try {
    po::store(po::command_line_parser(argc, argv)
                  .options(options)
                  .positional(positional_options)
                  .run(),
              optionsMap);
    if (optionsMap.count("help")) {
      std::cout << options << std::endl;
      return 0;
    }
    if (!optionsMap.count("input-file")) {
      std::cerr << "Error: You must provide an input-file." << std::endl;
      return 1;
    }
    if (optionsMap.count("compression-alg")) {
      if (args.compression_level == 0) {
        args.compression_level = args.compression_alg == "lz4" ? 1 : 3;
      }
    }
    po::notify(optionsMap);
  } catch (const std::exception& e) {
    std::cerr << "Error in command line argument: " << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  // fix number of threads -----------------------------------------------------
  //  a) 0 -> 1
  //  b) < 0 -> number of threads available
  //  c) > number of threads available -> number of threads available
  int max_threads = static_cast<int>(std::thread::hardware_concurrency());
  args.num_threads = args.num_threads < 0 ? max_threads : args.num_threads;
  args.num_threads =
      args.num_threads > max_threads ? max_threads : args.num_threads;
  args.num_threads = args.num_threads == 0 ? 1 : args.num_threads;

  // ===== Setup xs::Executor for preprocessing ================================

  // set reader ----------------------------------------------------------------
  auto reader = std::make_unique<xs::tasks::ExternBlockReader>(
      args.source_file, args.min_chunk_size);

  // set inplace processors ----------------------------------------------------
  std::vector<std::unique_ptr<xs::tasks::BaseInplaceProcessor<xs::DataChunk>>>
      inplace_processors;

  inplace_processors.push_back(
      std::make_unique<xs::tasks::NewLineSearcher>(args.mapping_data_distance));

  xs::CompressionType compression_type = xs::from_string(args.compression_alg);
  switch (compression_type) {
    case xs::CompressionType::LZ4:
      inplace_processors.push_back(
          std::make_unique<xs::tasks::LZ4Compressor>(args.hc, args.compression_level));
      break;
    case xs::CompressionType::ZSTD:
      inplace_processors.emplace_back(
          std::make_unique<xs::tasks::ZSTDCompressor>(args.compression_level));
      break;
    default:
      break;
  }

  // set return processor ------------------------------------------------------
  auto output_creator = std::make_unique<MetaDataCreator>();
  auto out_stream = args.output_file.empty()
                        ? nullptr
                        : std::make_unique<std::ofstream>(args.output_file);

  auto processor =
      xs::Executor<xs::DataChunk, DataWriter, preprocess_result, std::string,
                   xs::CompressionType, std::unique_ptr<std::ostream>>(
          args.num_threads, 1, std::move(reader), std::move(inplace_processors),
          std::move(output_creator), std::string(args.meta_file),
          xs::CompressionType(compression_type), std::move(out_stream));
  processor.join();

  return 0;
}