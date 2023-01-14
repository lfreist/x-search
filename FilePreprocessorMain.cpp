// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/xsearch.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  std::string sourceFile;
  std::string outputFile;
  std::string metaFile;
  std::string compressionAlg;
  int compressionLevel;
  size_t minBlockSize;
  uint64_t bytesNewLineMappingDistance;

  po::options_description options("Options for ESFCompressorMain");
  po::positional_options_description positional_options;

  auto add_positional =
      [&positional_options]<typename... Args>(Args&&... args) {
        positional_options.add(std::forward<Args>(args)...);
      };
  auto add = [&options]<typename... Args>(Args&&... args) {
    options.add_options()(std::forward<Args>(args)...);
  };
  add_positional("input-file", 1);
  add_positional("output-file", 1);
  add("help,h", "Produces this help message.");
  add("input-file", po::value<std::string>(&sourceFile), "input-file.");
  add("output-file,o", po::value<std::string>(&outputFile)->default_value(""),
      "output-file.");
  add("meta-file,m", po::value<std::string>(&metaFile)->default_value(""),
      "meta-file.");
  add("compression-alg,a",
      po::value<std::string>(&compressionAlg)->default_value(""),
      "Compression alg (zstd, lz4, none).");
  add("compression-level,l",
      po::value<int>(&compressionLevel)->default_value(0),
      "compression level (c.f. with chosen compression algorithm).");
  add("block-size", po::value<size_t>(&minBlockSize)->default_value(1 << 24),
      "size of one block.");
  add("bytes-nl-distance,d",
      po::value<uint64_t>(&bytesNewLineMappingDistance)->default_value(500),
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
      if (compressionLevel == 0) {
        compressionLevel = compressionAlg == "lz4" ? 1 : 3;
      }
    }
    po::notify(optionsMap);
  } catch (const std::exception& e) {
    std::cerr << "Error in command line argument: " << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  if (outputFile.empty()) {
  }

  xs::FilePreprocessor::preprocess(
      sourceFile, outputFile, metaFile, xs::from_string(compressionAlg),
      compressionLevel, bytesNewLineMappingDistance, minBlockSize, 1024 * 1024,
      true);

  return 0;
}
