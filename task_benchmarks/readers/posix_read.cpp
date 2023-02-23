// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <unistd.h>
#include <xsearch/utils/InlineBench.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  std::string file;
  size_t chunk_size;

  po::options_description options("Options for ExternStringFinderMain");
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
  add_positional("FILE", 1);
  add("FILE", po::value<std::string>(&file)->required(), "search pattern");
  add("chunk-size,s", po::value<size_t>(&chunk_size)->default_value(16777216));

  // parse command line options ------------------------------------------------
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
    po::notify(optionsMap);
  } catch (const std::exception& e) {
    std::cerr << "Error in command line argument: " << e.what() << std::endl;
    std::cerr << options << std::endl;
    return 1;
  }

  int fd = ::open(file.c_str(), O_RDONLY);

  if (fd < 0) {
    std::cerr << "Error reading file '" << file << "'\n";
    return 2;
  }

  char* buffer = new char[chunk_size];

  while (true) {
    INLINE_BENCHMARK_WALL_START_GLOBAL("read");
    auto bytes_read = ::read(fd, buffer, chunk_size);
    INLINE_BENCHMARK_WALL_STOP("read");
    if (bytes_read <= 0) {
      break;
    }
    if (buffer[file.size()] == '@') {
      std::cout << bytes_read;
    }
  }

  ::close(fd);
  delete[] buffer;

  std::cerr << INLINE_BENCHMARK_REPORT("json") << std::endl;

  return 0;
}