// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xsearch/utils/InlineBench.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
  std::string file;
  int64_t chunk_size;

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
  add("chunk-size,s", po::value<int64_t>(&chunk_size)->default_value(16777216));

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

  if (chunk_size % sysconf(_SC_PAGE_SIZE) != 0) {
    std::cerr << "chunk-size must be a multiple of '" +
                     std::to_string(sysconf(_SC_PAGE_SIZE)) + "'.\n";
    return 3;
  }

  int fd = ::open(file.c_str(), O_RDONLY);
  if (fd < 0) {
    std::cerr << "Error reading file '" << file << "'\n";
    return 2;
  }

  int64_t offset = 0;

  int64_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  if (chunk_size > file_size) {
    chunk_size = file_size;
  }

  while (true) {
    INLINE_BENCHMARK_WALL_START_GLOBAL("read");
    void* buffer = ::mmap(nullptr, chunk_size, PROT_READ, MAP_PRIVATE, fd, 0);
    INLINE_BENCHMARK_WALL_STOP("read");
    if (buffer == MAP_FAILED) {
      std::cerr << "Mapping failed.\n";
      ::close(fd);
      return 4;
    }
    char* str = static_cast<char*>(buffer);
    offset += chunk_size;
    if (offset >= file_size) {
      munmap(buffer, chunk_size);
      break;
    }
    if (str[file.size()] == '@') {
      std::cout << file_size;
    }
    munmap(buffer, chunk_size);
  }

  ::close(fd);

  std::cerr << INLINE_BENCHMARK_REPORT("json") << std::endl;

  return 0;
}