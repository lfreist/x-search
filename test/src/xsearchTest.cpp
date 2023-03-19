// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/xsearch.h>

static const std::string pattern("Sherlock");
static const std::string re_pattern("She[r ]lock");
static const std::string file_path("test/files/sample.txt");
static const std::string meta_file_path("test/files/sample.meta");
static const std::string lz4_path("test/files/sample.xslz4");
static const std::string lz4_meta("test/files/sample.xslz4.meta");
static const std::string zst_path("test/files/sample.xszst");
static const std::string zst_meta("test/files/sample.xszst.meta");

static const uint64_t literal_case_count = 46;
static const uint64_t literal_icase_count = 48;
static const uint64_t regex_case_count = 53;
static const uint64_t regex_icase_count = 59;
static const uint64_t literal_case_count_match = 46;
static const uint64_t literal_icase_count_match = 48;
static const uint64_t regex_case_count_match = 53;
static const uint64_t regex_icase_count_match = 59;
static const std::vector<uint64_t> literal_case_line_byte_offsets{
    3069978,  16699621, 16710519, 16719916, 16731645, 16741644, 16753058,
    29284087, 29306116, 29366861, 40366176, 40370472, 56795637, 56846283,
    56898623, 56950633, 57000246, 57055263, 57105384, 57155473, 57203089,
    57254051, 57302528, 57353529, 57407561, 57455096, 57502512, 57553326,
    57603496, 57655820, 57707597, 57759041, 57809187, 57859958, 73940808,
    73976836, 74008903, 74037906, 74074010, 74106199, 74142366, 74174416,
    74203571, 74232206, 86549400, 86549455};
static const std::vector<uint64_t> regex_case_line_byte_offsets{
    3069978,  15811186, 15850270, 15889344, 16699621, 16710519, 16719916,
    16731645, 16741644, 16753058, 26238516, 29284087, 29306116, 29366861,
    40366176, 40370472, 56795637, 56846283, 56898623, 56950633, 57000246,
    57055263, 57105384, 57155473, 57203089, 57254051, 57302528, 57353529,
    57407561, 57455096, 57502512, 57553326, 57603496, 57655820, 57707597,
    57759041, 57809187, 57859958, 73940808, 73976836, 74008903, 74037906,
    74074010, 74106199, 74142366, 74174416, 74203571, 74232206, 86549400,
    86549455, 96221854, 96889681, 98104205};
static const std::vector<uint64_t> literal_icase_line_byte_offsets{
    3069978,  16699621, 16710519, 16719916, 16731645, 16741644, 16753058,
    29284087, 29306116, 29366861, 40333565, 40338107, 40366176, 40370472,
    56795637, 56846283, 56898623, 56950633, 57000246, 57055263, 57105384,
    57155473, 57203089, 57254051, 57302528, 57353529, 57407561, 57455096,
    57502512, 57553326, 57603496, 57655820, 57707597, 57759041, 57809187,
    57859958, 73940808, 73976836, 74008903, 74037906, 74074010, 74106199,
    74142366, 74174416, 74203571, 74232206, 86549400, 86549455};
static const std::vector<uint64_t> regex_icase_line_byte_offsets{
    3069978,  15811186, 15850270, 15889344, 16699621, 16710519, 16719916,
    16731645, 16741644, 16753058, 26238516, 29284087, 29306116, 29366861,
    40333565, 40338107, 40366176, 40370472, 56795637, 56846283, 56898623,
    56950633, 57000246, 57055263, 57105384, 57155473, 57203089, 57254051,
    57302528, 57353529, 57407561, 57455096, 57502512, 57553326, 57603496,
    57655820, 57707597, 57759041, 57809187, 57859958, 73940808, 73976836,
    74008903, 74037906, 74074010, 74106199, 74142366, 74174416, 74203571,
    74232206, 83143686, 83179720, 86549400, 86549455, 96221854, 96222331,
    96889681, 96890158, 98104205};
static const std::vector<uint64_t> literal_case_match_byte_offsets{
    3069997,  16699644, 16710519, 16719939, 16731662, 16741667, 16753075,
    29284101, 29306130, 29366870, 40366191, 40370478, 56795653, 56846299,
    56898639, 56950647, 57000259, 57055279, 57105400, 57155488, 57203105,
    57254067, 57302543, 57353542, 57407576, 57455110, 57502527, 57553342,
    57603512, 57655833, 57707613, 57759057, 57809203, 57859974, 73940817,
    73976845, 74008912, 74037915, 74074019, 74106208, 74142375, 74174425,
    74203580, 74232215, 86549405, 86549455};
static const std::vector<uint64_t> regex_case_match_byte_offsets{
    3069997,  15811186, 15850270, 15889344, 16699644, 16710519, 16719939,
    16731662, 16741667, 16753075, 26238516, 29284101, 29306130, 29366870,
    40366191, 40370478, 56795653, 56846299, 56898639, 56950647, 57000259,
    57055279, 57105400, 57155488, 57203105, 57254067, 57302543, 57353542,
    57407576, 57455110, 57502527, 57553342, 57603512, 57655833, 57707613,
    57759057, 57809203, 57859974, 73940817, 73976845, 74008912, 74037915,
    74074019, 74106208, 74142375, 74174425, 74203580, 74232215, 86549405,
    86549455, 96221854, 96889681, 98104205};
static const std::vector<uint64_t> literal_icase_match_byte_offsets{
    3069997,  16699644, 16710519, 16719939, 16731662, 16741667, 16753075,
    29284101, 29306130, 29366870, 40333580, 40338113, 40366191, 40370478,
    56795653, 56846299, 56898639, 56950647, 57000259, 57055279, 57105400,
    57155488, 57203105, 57254067, 57302543, 57353542, 57407576, 57455110,
    57502527, 57553342, 57603512, 57655833, 57707613, 57759057, 57809203,
    57859974, 73940817, 73976845, 74008912, 74037915, 74074019, 74106208,
    74142375, 74174425, 74203580, 74232215, 86549405, 86549455};
static const std::vector<uint64_t> regex_icase_match_byte_offsets{
    3069997,  15811186, 15850270, 15889344, 16699644, 16710519, 16719939,
    16731662, 16741667, 16753075, 26238516, 29284101, 29306130, 29366870,
    40333580, 40338113, 40366191, 40370478, 56795653, 56846299, 56898639,
    56950647, 57000259, 57055279, 57105400, 57155488, 57203105, 57254067,
    57302543, 57353542, 57407576, 57455110, 57502527, 57553342, 57603512,
    57655833, 57707613, 57759057, 57809203, 57859974, 73940817, 73976845,
    74008912, 74037915, 74074019, 74106208, 74142375, 74174425, 74203580,
    74232215, 83143693, 83179727, 86549405, 86549455, 96221854, 96222360,
    96889681, 96890187, 98104205};
static const std::vector<uint64_t> literal_case_line_indices{
    113525,  577444,  577785,  578046,  578425,  578714,  579084,  1025581,
    1026348, 1028314, 1407712, 1407826, 1983678, 1985388, 1987113, 1988884,
    1990607, 1992500, 1994181, 1995861, 1997493, 1999204, 2000839, 2002600,
    2004453, 2006083, 2007712, 2009457, 2011139, 2012932, 2014694, 2016412,
    2018093, 2019813, 2563484, 2564900, 2566318, 2567731, 2569154, 2570573,
    2571992, 2573410, 2574820, 2576164, 2984075, 2984077};
static const std::vector<uint64_t> regex_case_line_indices{
    113525,  547405,  548914,  550269,  577444,  577785,  578046,  578425,
    578714,  579084,  918771,  1025581, 1026348, 1028314, 1407712, 1407826,
    1983678, 1985388, 1987113, 1988884, 1990607, 1992500, 1994181, 1995861,
    1997493, 1999204, 2000839, 2002600, 2004453, 2006083, 2007712, 2009457,
    2011139, 2012932, 2014694, 2016412, 2018093, 2019813, 2563484, 2564900,
    2566318, 2567731, 2569154, 2570573, 2571992, 2573410, 2574820, 2576164,
    2984075, 2984077, 3318488, 3345733, 3384278};
static const std::vector<uint64_t> literal_icase_line_indices{
    113525,  577444,  577785,  578046,  578425,  578714,  579084,  1025581,
    1026348, 1028314, 1406449, 1406631, 1407712, 1407826, 1983678, 1985388,
    1987113, 1988884, 1990607, 1992500, 1994181, 1995861, 1997493, 1999204,
    2000839, 2002600, 2004453, 2006083, 2007712, 2009457, 2011139, 2012932,
    2014694, 2016412, 2018093, 2019813, 2563484, 2564900, 2566318, 2567731,
    2569154, 2570573, 2571992, 2573410, 2574820, 2576164, 2984075, 2984077};
static const std::vector<uint64_t> regex_icase_line_indices{
    113525,  547405,  548914,  550269,  577444,  577785,  578046,  578425,
    578714,  579084,  918771,  1025581, 1026348, 1028314, 1406449, 1406631,
    1407712, 1407826, 1983678, 1985388, 1987113, 1988884, 1990607, 1992500,
    1994181, 1995861, 1997493, 1999204, 2000839, 2002600, 2004453, 2006083,
    2007712, 2009457, 2011139, 2012932, 2014694, 2016412, 2018093, 2019813,
    2563484, 2564900, 2566318, 2567731, 2569154, 2570573, 2571992, 2573410,
    2574820, 2576164, 2867410, 2868630, 2984075, 2984077, 3318488, 3318508,
    3345733, 3345753, 3384278};
static const std::vector<std::string> literal_case_lines{
    "-l'll get a check, Sherlock.",
    "Playing Watson to your Sherlock.",
    "Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Take it easy, Sherlock.",
    "Take it easy, Sherlock.",
    "No shit, Sherlock.",
    "Just a minute, Sherlock.",
    "Look, Sherlock.",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "So where to, Sherlock?",
    "-So, where to, Sherlock?",
    "So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "Meet Sherlock Bones.",
    "Sherlock, okay, Harvey was last seen--"};
static const std::vector<std::string> regex_case_lines{
    "-l'll get a check, Sherlock.",
    "She locked herself inside",
    "She locked herself inside",
    "She locked herself inside",
    "Playing Watson to your Sherlock.",
    "Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "She locked the door.",
    "Take it easy, Sherlock.",
    "Take it easy, Sherlock.",
    "No shit, Sherlock.",
    "Just a minute, Sherlock.",
    "Look, Sherlock.",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "So where to, Sherlock?",
    "-So, where to, Sherlock?",
    "So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "Meet Sherlock Bones.",
    "Sherlock, okay, Harvey was last seen--",
    "She locks the door, staggers back, collapses...",
    "She locks the door, staggers back, collapses...",
    "She locked me in at home and ran to spend the evening with them."};
static const std::vector<std::string> literal_icase_lines{
    "-l'll get a check, Sherlock.",
    "Playing Watson to your Sherlock.",
    "Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Take it easy, Sherlock.",
    "Take it easy, Sherlock.",
    "No shit, Sherlock.",
    "JUST A MINUTE, SHERLOCK.",
    "LOOK, SHERLOCK,",
    "Just a minute, Sherlock.",
    "Look, Sherlock.",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "So where to, Sherlock?",
    "-So, where to, Sherlock?",
    "So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "Meet Sherlock Bones.",
    "Sherlock, okay, Harvey was last seen--"};
static const std::vector<std::string> regex_icase_lines{
    "-l'll get a check, Sherlock.",
    "She locked herself inside",
    "She locked herself inside",
    "She locked herself inside",
    "Playing Watson to your Sherlock.",
    "Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "Playing Watson to your Sherlock.",
    "- No... [Static] Sherlock.",
    "She locked the door.",
    "Take it easy, Sherlock.",
    "Take it easy, Sherlock.",
    "No shit, Sherlock.",
    "JUST A MINUTE, SHERLOCK.",
    "LOOK, SHERLOCK,",
    "Just a minute, Sherlock.",
    "Look, Sherlock.",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "-So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "So where to, Sherlock?",
    "-So, where to, Sherlock?",
    "So, where to, Sherlock?",
    "- So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "So where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "- So, where to, Sherlock?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "You read Sherlock Holmes to deduce that?",
    "Why is she locked up?",
    "Why is she locked up?",
    "Meet Sherlock Bones.",
    "Sherlock, okay, Harvey was last seen--",
    "She locks the door, staggers back, collapses...",
    "The question now is, who was she locking out?",
    "She locks the door, staggers back, collapses...",
    "The question now is, who was she locking out?",
    "She locked me in at home and ran to spend the evening with them."};

// _____ Reading plain text file without metadata ______________________________

// ===== join search and copy results ==========================================

TEST(ExternSearcherTest, count_matches_literal_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, false, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, true, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, false, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, true, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_join) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, false, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, true, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, false, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, true, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, false, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, true, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, false, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, true, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, false, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, true, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, false, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, true, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_join) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_join) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_indices>(pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(pattern, file_path, false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path, true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_join) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, false, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, true, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_join) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_icase_lines);
  }

  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, false, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, true, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_join) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, false, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, true, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_icase_lines);
  }

  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, false, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, true, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// ===== live results accessed via iterators ===================================

TEST(ExternSearcherTest, count_matches_literal_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, false, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, true, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, false, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(pattern, file_path, true, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_live) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, false, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, true, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, false, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path, true, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, false, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, true, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, false, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_matches>(re_pattern, file_path, true, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, false, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, true, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, false, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, file_path, true, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::line_byte_offsets>(pattern, file_path, true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_live) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::match_byte_offsets>(pattern, file_path, true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_live) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_indices>(pattern, file_path, false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path, true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(pattern, file_path, false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path, true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_live) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, false, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, true, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, false, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::line_indices>(re_pattern, file_path, true, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_live) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, false, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, true, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, false, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, true, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_live) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, false, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, true, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, false, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path, true, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// _____ Reading file with metadata ____________________________________________

// ===== join search and copy results ==========================================

TEST(ExternSearcherTest, count_matches_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(
        pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(
        pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_join_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// ===== live results accessed via iterators ===================================

TEST(ExternSearcherTest, count_matches_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(
        pattern, file_path, meta_file_path, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(
        pattern, file_path, meta_file_path, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        pattern, file_path, meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        pattern, file_path, meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, file_path, meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(pattern, file_path, meta_file_path,
                                            true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_live_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// _____ Reading lz4 compressed file with metadata _____________________________

// ===== join search and copy results ==========================================

TEST(ExternSearcherTest, count_matches_literal_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_join_lz4_meta) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_join_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// ===== live results accessed via iterators ===================================

TEST(ExternSearcherTest, count_matches_literal_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, lz4_path, lz4_meta,
                                                    true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, lz4_path, lz4_meta,
                                                  true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, lz4_path,
                                                    lz4_meta, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, lz4_path,
                                                  lz4_meta, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, lz4_path,
                                                        lz4_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, lz4_path,
                                                        lz4_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, lz4_path,
                                                         lz4_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, lz4_path,
                                                         lz4_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, lz4_path, lz4_meta,
                                                   true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, lz4_path,
                                                   lz4_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_live_lz4_meta) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, lz4_path, lz4_meta, true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_live_lz4_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, lz4_path, lz4_meta,
                                            true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// _____ Reading zst compressed file with metadata _____________________________

// ===== join search and copy results ==========================================

TEST(ExternSearcherTest, count_matches_literal_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, false, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, true, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, false, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, true, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_join_zst_meta) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_join_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            false, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            true, 1, 1);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            false, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            true, 4, 4);
    res->join();
    auto result = res->getResult()->copyResultSafe();
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

// ===== live results accessed via iterators ===================================

TEST(ExternSearcherTest, count_matches_literal_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(pattern, zst_path, zst_meta,
                                                    true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_literal_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(pattern, zst_path, zst_meta,
                                                  true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, literal_icase_count);
  }
}

TEST(ExternSearcherTest, count_matches_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count_match);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_matches>(re_pattern, zst_path,
                                                    zst_meta, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count_match);
  }
}

TEST(ExternSearcherTest, count_lines_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, false, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, true, 1, 1);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, false, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_case_count);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::count_lines>(re_pattern, zst_path,
                                                  zst_meta, true, 4, 4);
    uint64_t result = 0;
    for (auto i : *res->getResult()) {
      result = i;
    }
    ASSERT_EQ(result, regex_icase_count);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_literal_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(pattern, zst_path,
                                                        zst_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_byte_offsets_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_byte_offsets>(re_pattern, zst_path,
                                                        zst_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_literal_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(pattern, zst_path,
                                                         zst_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, match_byte_offsets_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_match_byte_offsets);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::match_byte_offsets>(re_pattern, zst_path,
                                                         zst_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_match_byte_offsets);
  }
}

TEST(ExternSearcherTest, line_indices_literal_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(pattern, zst_path, zst_meta,
                                                   true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, literal_icase_line_indices);
  }
}

TEST(ExternSearcherTest, line_indices_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, false, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, true, 1, 1);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, false, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_case_line_indices);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::line_indices>(re_pattern, zst_path,
                                                   zst_meta, true, 4, 4);
    std::vector<uint64_t> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    ASSERT_EQ(result, regex_icase_line_indices);
  }
}

TEST(ExternSearcherTest, lines_literal_live_zst_meta) {
  {  // case, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_case_lines);
  }
  {  // icase, single thread
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, literal_icase_lines);
  }
  {  // case, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res =
        xs::extern_search<xs::lines>(pattern, zst_path, zst_meta, true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(literal_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}

TEST(ExternSearcherTest, lines_regex_live_zst_meta) {
  {  // case, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            false, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_case_lines);
  }
  {  // icase, single thread
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            true, 1, 1);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    ASSERT_EQ(result, regex_icase_lines);
  }
  {  // case, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            false, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_case_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
  {  // icase, 4 threads
    auto res = xs::extern_search<xs::lines>(re_pattern, zst_path, zst_meta,
                                            true, 4, 4);
    std::vector<std::string> result{};
    for (auto r : *res->getResult()) {
      result.push_back(r);
    }
    std::sort(result.begin(), result.end());
    std::vector<std::string> tmp(regex_icase_lines);
    std::sort(tmp.begin(), tmp.end());
    ASSERT_EQ(result, tmp);
  }
}