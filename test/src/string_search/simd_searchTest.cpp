// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/string_search/simd_search.h>

#include <cstring>
#include <algorithm>

using namespace xs::search;

static char dummy_text[1241] =
    "Liane reindorsing two-time zippering chromolithography rainbowweed "
    "Cacatua bunking cooptions zinckenite Polygala "
    "smooth-bellied chirognostic inkos BVM antigraphy pagne bicorne "
    "complementizer commorant ever-endingly sheikhly\n"
    "glam predamaged objectionability evil-looking quaquaversal\n"
    "composite halter-wise mosasaur Whelan coleopterist grass-grown Helladic "
    "DNB nondeliriousness arpents uncasing "
    "predepletion delator unnaive sucken solid-gold brassards tutorials "
    "refrangible terebras autobiographal mid-breast "
    "rim-fire yallow soapfish trammelled Fangio thermocauteries river-drift "
    "micromeasurement BSMetE tiddling Delaunay "
    "prebendaryship depigment Cranwell Fruma sickly-seeming Abnaki Woodlake "
    "unknocked Vanorin sudds long-shadowed\n"
    "reciters Platypoda expdt imperious ensheathes diallagic indigo-yielding "
    "enforcive Ibilao jubilus meisje sitively "
    "town-bound leukocytoblast serpuline Camb widespreadly archsnob "
    "convocational nonelliptically turfless nonagent "
    "intellectually arm-great unperceptional robotries campestral hyaenid "
    "mispublicized Othoniel ultraistic nemophily "
    "babel's overhate one-storied unkodaked Ogden Gore Gekkonidae Hymenolepis "
    "nonheritable TMR poppy-crowned inclipped "
    "well-centered by-job crop-tailed vagrantism condescensively\1";

TEST(simd_searchTest, strchr) {
  ASSERT_EQ(simd::strchr(dummy_text, 1240, 'L'), strchr(dummy_text, 'L'));
  ASSERT_EQ(simd::strchr(dummy_text, 1240, '\n'), strchr(dummy_text, '\n'));
  ASSERT_EQ(simd::strchr(dummy_text, 1240, '\1'), strchr(dummy_text, '\1'));
  ASSERT_EQ(simd::strchr(dummy_text, 1240, '\2'), strchr(dummy_text, '\2'));
}

TEST(simd_searchTest, strstr) {
  ASSERT_EQ(simd::strstr(dummy_text, 1240, "Liane", 5),
            strstr(dummy_text, "Liane"));
  ASSERT_EQ(
      simd::strstr(dummy_text, 1240,
                   "smooth-bellied chirognostic inkos BVM antigraphy pagne "
                   "bicorne complementizer commorant ever-endingly sheikhly",
                   110),
      strstr(dummy_text,
             "smooth-bellied chirognostic inkos BVM antigraphy pagne "
             "bicorne complementizer commorant ever-endingly sheikhly"));
  ASSERT_EQ(simd::strstr(dummy_text, 1240, "Helladic", 8),
            strstr(dummy_text, "Helladic"));
  ASSERT_EQ(simd::strstr(dummy_text, 1240, "ly\1", 3),
            strstr(dummy_text, "ly\1"));
  ASSERT_EQ(simd::strstr(dummy_text, 1240, "jkahgsf", 7),
            strstr(dummy_text, "jkahgsf"));
}

TEST(simd_searchTest, findNext) {
  ASSERT_EQ(simd::findNext("Liane", 5, dummy_text, 1240, 0), 0);
  ASSERT_EQ(
      simd::findNext("smooth-bellied chirognostic inkos BVM antigraphy pagne "
                     "bicorne complementizer commorant ever-endingly sheikhly",
                     110, dummy_text, 1240, 0),
      113);
  ASSERT_EQ(simd::findNext("Helladic", 8, dummy_text, 1240, 0), 346);
  ASSERT_EQ(simd::findNext("ly\1", 3, dummy_text, 1240, 0), 1237);
  ASSERT_EQ(simd::findNext("jkahgsf", 7, dummy_text, 1240, 0), -1);
  ASSERT_EQ(simd::findNext("ia", 2, dummy_text, 1240, 0), 1);
  ASSERT_EQ(simd::findNext("ia", 2, dummy_text, 1240, 3), 455);
}

TEST(simd_searchTest, findNextNewLine) {
  ASSERT_EQ(simd::findNextNewLine(dummy_text, 1240, 0), 223);
  ASSERT_EQ(simd::findNextNewLine(dummy_text, 1240, 224), 282);
  ASSERT_EQ(simd::findNextNewLine(dummy_text, 1240, 283), 728);
  ASSERT_EQ(simd::findNextNewLine(dummy_text, 1240, 729), -1);
}

TEST(simd_searchTest, findAllPerLine) {
  ASSERT_EQ(simd::findAllPerLine("is", 2, dummy_text, 1240), 2);
  ASSERT_EQ(simd::findAllPerLine("th", 2, dummy_text, 1240), 3);
  ASSERT_EQ(simd::findAllPerLine("Van", 3, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAllPerLine("y\1", 2, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAllPerLine("Liane", 5, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAllPerLine("Vansdf", 6, dummy_text, 1240), 0);
}

TEST(simd_searchTest, findAll) {
  ASSERT_EQ(simd::findAll("is", 2, dummy_text, 1240), 8);
  ASSERT_EQ(simd::findAll("th", 2, dummy_text, 1240), 5);
  ASSERT_EQ(simd::findAll("Van", 3, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAll("y\1", 2, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAll("Liane", 5, dummy_text, 1240), 1);
  ASSERT_EQ(simd::findAll("Vansdf", 6, dummy_text, 1240), 0);
}

/*
TEST(simd_searchTest, strcasestr) {
  std::string test("moin and Hello and hello");
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hello", 5),
            strcasestr(test, "hello"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "HELLO", 5),
            strcasestr(test, "HELLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hElLO", 5),
            strcasestr(test, "hElLO"));
  test.assign("moin and hELLo and HELLO");
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hello", 5),
            strcasestr(test, "hello"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "HELLO", 5),
            strcasestr(test, "HELLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hElLO", 5),
            strcasestr(test, "hElLO"));
  test.assign("moin and HELLO and hello");
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hello", 5),
            strcasestr(test, "hello"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "HELLO", 5),
            strcasestr(test, "HELLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hElLO", 5),
            strcasestr(test, "hElLO"));
  test.assign("moin and hello and Hello");
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hello", 5),
            strcasestr(test, "hello"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "HELLO", 5),
            strcasestr(test, "HELLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hElLO", 5),
            strcasestr(test, "hElLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "kerm", 5),
            strcasestr(test, "kerm"));
  test.assign("moin and hellO and Hello");
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hello", 5),
            strcasestr(test, "hello"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "HELLO", 5),
            strcasestr(test, "HELLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "hElLO", 5),
            strcasestr(test, "hElLO"));
  ASSERT_EQ(simd::strcasestr(test.data(), test.size(), "kerm", 5),
            strcasestr(test, "kerm"));
}
 */