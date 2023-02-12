// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

struct ProcArgs {
  std::string source_file;
  std::string output_file;
  std::string meta_file;
  std::string compression_alg;
  int compression_level = 0;
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
      "compression level (default: lz4: 1, zstd: 3)");
  add("chunk-size,s",
      po::value<size_t>(&args.min_chunk_size)->default_value(16777216),
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

  // ===== Setup xs::Executor for preprocessing ================================

  std::vector<std::unique_ptr<xs::tasks::BaseProcessor<xs::DataChunk>>>
      processors;

  // check for compression and add decompression task --------------------------
  processors.push_back(
      std::make_unique<xs::tasks::NewLineSearcher>(args.mapping_data_distance));
  if (args.compression_alg != "none") {
    // TODO: implement compressors
    xs::CompressionType compression_type =
        xs::from_string(args.compression_alg);
    switch (compression_type) {
      case xs::CompressionType::LZ4:
        // processors.push_back(std::make_unique<xs::tasks::LZ4Compressor>());
        break;
      case xs::CompressionType::ZSTD:
        // processors.emplace_back(
        //     std::make_unique<xs::tasks::ZSTDCompressor>());
        break;
      default:
        break;
    }
  }
  // set reader ----------------------------------------------------------------
  auto reader = std::make_unique<xs::tasks::ExternBlockReader>(
      args.source_file, args.min_chunk_size);
}