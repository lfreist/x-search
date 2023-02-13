// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/MetaFile.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc != 2 && argc != 3) {
    std::cout << "Prints content of *.sf.meta files human readable";
    std::cout << "Usage:\n";
    std::cout << " ./sf_metafile_cat <path/to/meta/file> [--mapping-data]"
              << std::endl;
    return 1;
  }

  std::string path(argv[1]);

  bool print_mapping_data =
      argc == 3 && std::string(argv[2]) == "--mapping-data";

  xs::MetaFile meta_file(path, std::ios::in);

  auto compression_type = meta_file.getCompressionType();
  std::cout << "Compression Type: "
            << xs::to_string(compression_type) << " (" << sizeof(compression_type) << ")" << std::endl;

  uint64_t counter = 0;
  while (true) {
    auto md = meta_file.nextChunkMetaData();
    if (!md.has_value()) {
      break;
    }
    auto& md_val = md.value();
    std::cout << "---\n";
    std::cout << "Chunk " << counter << ":\n";
    std::cout << " Byte offset:\n";
    std::cout << "  original: " << md_val.original_offset << '\n';
    std::cout << "  actual  : " << md_val.actual_offset << '\n';
    std::cout << " Size (bytes):\n";
    std::cout << "  original: " << md_val.original_size << '\n';
    std::cout << "  actual  : " << md_val.actual_size << '\n';
    std::cout << " 'Byte offset -> line index' mapping data (total: "
              << md_val.line_mapping_data.size() << "):\n";
    if (print_mapping_data) {
      for (const auto& v : md_val.line_mapping_data) {
        std::cout << "  global byte offset: " << v.globalByteOffset << '\n';
        std::cout << "  global line index : " << v.globalLineIndex << '\n';
        std::cout << "  ---\n";
      }
    }
    counter++;
  }
  return 0;
}