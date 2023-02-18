// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/DataChunk.h>
#include <xsearch/string_search/offset_mappings.h>

using namespace xs;

static char dummy_text[] =
    "Liane reindorsing two-time zippering chromolithography rainbowweed\n"
    "Cacatua bunking cooptions zinckenite Polygala\n"
    "smooth-bellied chirognostic inkos BVM antigraphy pagne bicorne\n"
    "complementizer commorant ever-endingly sheikhly\n"
    "glam predamaged objectionability evil-looking quaquaversal\n"
    "composite halter-wise mosasaur Whelan coleopterist grass-grown Helladic\n"
    "DNB nondeliriousness arpents uncasing\n"
    "predepletion delator unnaive sucken solid-gold brassards tutorials\n"
    "refrangible terebras autobiographal mid-breast\n"
    "rim-fire yallow soapfish trammelled Fangio thermocauteries river-drift\n"
    "micromeasurement BSMetE tiddling Delaunay\n"
    "prebendaryship depigment Cranwell Fruma sickly-seeming Abnaki Woodlake\n"
    "unknocked Vanorin sudds long-shadowed\n"
    "reciters Platypoda expdt imperious ensheathes diallagic indigo-yielding\n"
    "enforcive Ibilao jubilus meisje sitively\n"
    "town-bound leukocytoblast serpuline Camb widespreadly archsnob\n"
    "convocational nonelliptically turfless nonagent\n"
    "intellectually arm-great unperceptional robotries campestral hyaenid\n"
    "mispublicized Othoniel ultraistic nemophily\n"
    "babel's overhate one-storied unkodaked Ogden Gore Gekkonidae\n"
    "Hymenolepis\n"
    "nonheritable TMR poppy-crowned inclipped\n"
    "well-centered by-job crop-tailed vagrantism condescensivelyx";

TEST(offset_mapping, to_line_indices) {
  DataChunk str(
      {0, 0, 0, 0, 0, {{177, 3}, {394, 7}, {802, 14}, {1067, 19}, {1239, 22}}});
  str.assign(dummy_text);
  std::vector<uint64_t> byte_offsets(1240);
  // vector of all byte offsets
  for (size_t i = 0; i < byte_offsets.size(); ++i) {
    byte_offsets.at(i) = i;
  }

  std::vector<uint64_t> line_indices;
  line_indices.reserve(1240);
  uint64_t line_index = 0;
  for (auto& c : dummy_text) {
    // necessary because we do not want to count \0 byte as part of the data
    if (c == '\0') {
      break;
    }
    line_indices.push_back(line_index);
    if (c == '\n') {
      line_index++;
    }
  }

  auto tmp = map::bytes::to_line_indices(&str, byte_offsets);
  for (size_t ind = 0; ind < line_indices.size(); ++ind) {
    ASSERT_EQ(tmp[ind], line_indices[ind]);
  }

  // ASSERT_EQ(map::bytes::to_line_indices(&str, byte_offsets), line_indices);
}

TEST(offset_mapping, to_line) {
  DataChunk str(
      {0, 0, 0, 0, 0, {{177, 3}, {394, 7}, {802, 14}, {1067, 19}, {1239, 22}}});
  str.assign(dummy_text);
  ASSERT_EQ(
      xs::map::byte::to_line(&str, 0),
      "Liane reindorsing two-time zippering chromolithography rainbowweed");
  ASSERT_EQ(
      xs::map::byte::to_line(&str, 12),
      "Liane reindorsing two-time zippering chromolithography rainbowweed");
  ASSERT_EQ(xs::map::byte::to_line(&str, 180),
            "complementizer commorant ever-endingly sheikhly");
  ASSERT_EQ(xs::map::byte::to_line(&str, 250),
            "glam predamaged objectionability evil-looking quaquaversal");
  ASSERT_EQ(xs::map::byte::to_line(&str, 392),
            "DNB nondeliriousness arpents uncasing");
  ASSERT_EQ(xs::map::byte::to_line(&str, 801),
            "enforcive Ibilao jubilus meisje sitively");
  ASSERT_EQ(xs::map::byte::to_line(&str, 1239),
            "well-centered by-job crop-tailed vagrantism condescensivelyx");
  ASSERT_THROW(xs::map::byte::to_line(&str, 1240), std::runtime_error);
}