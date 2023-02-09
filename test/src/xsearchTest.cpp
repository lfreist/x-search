// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/xsearch.h>

static const std::string pattern("over");
static const std::string re_pattern("ov[e|i]r");
static const std::string file_path("test/files/dummy.txt");
static const std::string meta_file_path("test/files/dummy.xs.meta");

TEST(ExternSearcherTest, count) {
  {  // plain text
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 152);
  }
  {  // regex
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 156);
  }
  {  // multiple threads
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, 4, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 152);
  }
  {  // multiple readers
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, 1, 2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 152);
  }
  {  // multiple readers and threads
    auto res = xs::extern_search<xs::count_matches>(pattern, file_path,
                                                    meta_file_path, 4, 2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 152);
  }
  {  // multiple readers and threads and regex pattern
    auto res = xs::extern_search<xs::count_matches>(re_pattern, file_path,
                                                    meta_file_path, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 156);
  }
  {  // compression
    auto res = xs::extern_search<xs::count_matches>(
        re_pattern, "test/files/dummy.xslz4", "test/files/dummy.xslz4.meta", 4,
        2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 156);
  }
}

TEST(ExternSearcherTest, count_lines) {
  {  // plain text
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 133);
  }
  {  // regex
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, 1, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 137);
  }
  {  // multiple threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, 4, 1);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 133);
  }
  {  // multiple readers
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, 1, 2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 133);
  }
  {  // multiple readers and threads
    auto res = xs::extern_search<xs::count_lines>(pattern, file_path,
                                                  meta_file_path, 4, 2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 133);
  }
  {  // multiple readers and threads and regex pattern
    auto res = xs::extern_search<xs::count_lines>(re_pattern, file_path,
                                                  meta_file_path, 4, 4);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 137);
  }
  {  // compression
    auto res =
        xs::extern_search<xs::count_lines>(re_pattern, "test/files/dummy.xslz4",
                                           "test/files/dummy.xslz4.meta", 4, 2);
    res->join();
    ASSERT_EQ(res->getResult()->size(), 137);
  }
}

TEST(ExternSearcherTest, match_byte_offsets) {
  const std::vector<size_t> plain_res = {
      567,    1524,   2618,   3259,   3540,   4009,   4071,   8395,   9045,
      10770,  11290,  13386,  16684,  18207,  20585,  22272,  24272,  24713,
      25667,  25823,  26496,  26787,  29117,  32111,  35073,  36484,  36695,
      37541,  37568,  37921,  40577,  40661,  42145,  42628,  43104,  43838,
      43968,  44851,  44910,  46793,  48951,  48998,  49613,  51773,  52911,
      53066,  54435,  56924,  58697,  59452,  60569,  62030,  62061,  63528,
      68569,  68828,  69216,  69624,  70802,  71856,  74898,  76786,  82871,
      84551,  84659,  85904,  87073,  87706,  88132,  89065,  90223,  91563,
      93953,  94393,  94898,  96407,  96590,  97176,  97813,  99014,  99258,
      100294, 100566, 101352, 101988, 103264, 103380, 104975, 105041, 108304,
      109036, 109812, 111008, 113091, 113300, 115122, 116301, 118928, 122337,
      123475, 125210, 125472, 125535, 125801, 128326, 129939, 131143, 131818,
      132707, 132745, 133594, 136407, 137060, 137870, 139918, 142424, 142919,
      143707, 145296, 146293, 147033, 147452, 149181, 150752, 151190, 151419,
      151458, 151609, 153092, 154041, 155175, 156048, 156403, 158137, 158682,
      160076, 160518, 161341, 162334, 165204, 165324, 165580, 165623, 169338,
      169844, 171450, 174841, 178365, 178859, 179278, 179375, 180045};
  const std::vector<size_t> regex_res = {
      567,    1524,   2618,   3259,   3540,   4009,   4071,   8395,   9045,
      10770,  11290,  13386,  16684,  18207,  20585,  21221,  22272,  24272,
      24713,  25667,  25823,  26496,  26787,  29117,  32111,  35073,  36484,
      36695,  37541,  37568,  37921,  40577,  40661,  42145,  42628,  43104,
      43838,  43968,  44851,  44910,  46793,  48951,  48998,  49613,  51773,
      52911,  53066,  54435,  56924,  58697,  59452,  60569,  62030,  62061,
      63528,  68569,  68828,  69216,  69624,  70802,  71856,  74898,  76786,
      81033,  82871,  84551,  84659,  85904,  87073,  87706,  88132,  89065,
      90223,  91563,  93953,  94393,  94898,  96407,  96590,  97176,  97813,
      99014,  99258,  100294, 100566, 101352, 101988, 103264, 103380, 104975,
      105041, 108304, 109036, 109812, 111008, 113091, 113300, 115122, 116301,
      118928, 122337, 123475, 125210, 125472, 125535, 125801, 128326, 129939,
      131143, 131818, 132707, 132745, 133594, 136407, 137060, 137870, 139918,
      142424, 142919, 143707, 145296, 146293, 147033, 147452, 149181, 150752,
      151190, 151419, 151458, 151609, 153092, 154041, 155175, 156048, 156403,
      158137, 158682, 160076, 160518, 161341, 162334, 165204, 165324, 165580,
      165623, 169338, 169844, 171450, 174841, 176726, 177359, 178365, 178859,
      179278, 179375, 180045};
  {  // plain text
    auto _xs = xs::extern_search<xs::match_byte_offsets>(pattern, file_path,
                                                         meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // regex
    auto _xs = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
  {  // multiple threads
    auto _xs = xs::extern_search<xs::match_byte_offsets>(pattern, file_path,
                                                         meta_file_path, 4, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers
    auto _xs = xs::extern_search<xs::match_byte_offsets>(pattern, file_path,
                                                         meta_file_path, 1, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads
    auto _xs = xs::extern_search<xs::match_byte_offsets>(pattern, file_path,
                                                         meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads and regex pattern
    auto _xs = xs::extern_search<xs::match_byte_offsets>(re_pattern, file_path,
                                                         meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
  {  // compression
    auto _xs = xs::extern_search<xs::match_byte_offsets>(
        re_pattern, "test/files/dummy.xslz4", "test/files/dummy.xslz4.meta", 4,
        2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
}

TEST(ExternSearcherTest, line_byte_offsets) {
  const std::vector<size_t> plain_res = {
      421,    1375,   2507,   3251,   3521,   3626,   4031,   8348,   8704,
      10683,  11266,  13230,  16464,  17925,  20440,  22120,  24177,  24506,
      25651,  26404,  26629,  28994,  31761,  34689,  36324,  37384,  37687,
      40361,  42130,  42513,  43078,  43828,  44749,  46614,  48608,  48973,
      49557,  51679,  52849,  54244,  56808,  58435,  59113,  60474,  61639,
      63252,  68337,  69165,  70554,  71778,  74667,  76677,  82749,  84440,
      85752,  87014,  87690,  88123,  89065,  90107,  91395,  93943,  94104,
      94684,  96004,  96539,  97104,  97765,  98737,  99227,  100051, 100452,
      101299, 101975, 103234, 104926, 108146, 108916, 109771, 110946, 113041,
      113125, 115093, 116154, 118494, 122205, 123440, 124857, 125265, 125677,
      128227, 129668, 130974, 131790, 132671, 133527, 136283, 136778, 137752,
      139849, 142222, 142736, 143634, 145272, 146003, 146848, 147212, 149071,
      150699, 151182, 151378, 151600, 153031, 154035, 155153, 155884, 156096,
      158024, 158318, 160013, 160457, 161091, 162073, 165069, 165272, 169177,
      169663, 171346, 174725, 178340, 178565, 179272, 179847};
  const std::vector<size_t> regex_res = {
      421,    1375,   2507,   3251,   3521,   3626,   4031,   8348,   8704,
      10683,  11266,  13230,  16464,  17925,  20440,  21153,  22120,  24177,
      24506,  25651,  26404,  26629,  28994,  31761,  34689,  36324,  37384,
      37687,  40361,  42130,  42513,  43078,  43828,  44749,  46614,  48608,
      48973,  49557,  51679,  52849,  54244,  56808,  58435,  59113,  60474,
      61639,  63252,  68337,  69165,  70554,  71778,  74667,  76677,  80671,
      82749,  84440,  85752,  87014,  87690,  88123,  89065,  90107,  91395,
      93943,  94104,  94684,  96004,  96539,  97104,  97765,  98737,  99227,
      100051, 100452, 101299, 101975, 103234, 104926, 108146, 108916, 109771,
      110946, 113041, 113125, 115093, 116154, 118494, 122205, 123440, 124857,
      125265, 125677, 128227, 129668, 130974, 131790, 132671, 133527, 136283,
      136778, 137752, 139849, 142222, 142736, 143634, 145272, 146003, 146848,
      147212, 149071, 150699, 151182, 151378, 151600, 153031, 154035, 155153,
      155884, 156096, 158024, 158318, 160013, 160457, 161091, 162073, 165069,
      165272, 169177, 169663, 171346, 174725, 176685, 177091, 178340, 178565,
      179272, 179847};
  {  // plain text
    auto _xs = xs::extern_search<xs::line_byte_offsets>(pattern, file_path,
                                                        meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // regex
    auto _xs = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // multiple threads
    auto _xs = xs::extern_search<xs::line_byte_offsets>(pattern, file_path,
                                                        meta_file_path, 4, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers
    auto _xs = xs::extern_search<xs::line_byte_offsets>(pattern, file_path,
                                                        meta_file_path, 1, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads
    auto _xs = xs::extern_search<xs::line_byte_offsets>(pattern, file_path,
                                                        meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads and regex pattern
    auto _xs = xs::extern_search<xs::line_byte_offsets>(re_pattern, file_path,
                                                        meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // compression
    auto _xs = xs::extern_search<xs::line_byte_offsets>(
        re_pattern, "test/files/dummy.xslz4", "test/files/dummy.xslz4.meta", 4,
        2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
}

TEST(ExternSearcherTest, line_indices) {
  const std::vector<size_t> plain_res = {
      1,   5,   8,   10,  12,  13,  14,  35,  37,  44,  47,  54,  65,  71,  79,
      86,  93,  95,  100, 102, 103, 112, 120, 131, 137, 140, 141, 152, 160, 162,
      164, 166, 169, 175, 185, 186, 189, 198, 203, 209, 220, 225, 227, 231, 235,
      240, 258, 260, 264, 269, 281, 289, 310, 318, 323, 326, 329, 330, 332, 335,
      339, 348, 349, 351, 357, 359, 361, 363, 366, 367, 371, 373, 377, 380, 385,
      390, 403, 406, 408, 411, 420, 421, 426, 429, 440, 453, 459, 465, 466, 467,
      476, 480, 485, 488, 492, 497, 505, 506, 508, 515, 523, 525, 528, 534, 537,
      541, 542, 548, 556, 558, 560, 561, 568, 573, 576, 579, 580, 586, 587, 592,
      594, 596, 599, 610, 611, 625, 626, 634, 645, 658, 659, 661, 664};
  const std::vector<size_t> regex_res = {
      1,   5,   8,   10,  12,  13,  14,  35,  37,  44,  47,  54,  65,  71,
      79,  81,  86,  93,  95,  100, 102, 103, 112, 120, 131, 137, 140, 141,
      152, 160, 162, 164, 166, 169, 175, 185, 186, 189, 198, 203, 209, 220,
      225, 227, 231, 235, 240, 258, 260, 264, 269, 281, 289, 302, 310, 318,
      323, 326, 329, 330, 332, 335, 339, 348, 349, 351, 357, 359, 361, 363,
      366, 367, 371, 373, 377, 380, 385, 390, 403, 406, 408, 411, 420, 421,
      426, 429, 440, 453, 459, 465, 466, 467, 476, 480, 485, 488, 492, 497,
      505, 506, 508, 515, 523, 525, 528, 534, 537, 541, 542, 548, 556, 558,
      560, 561, 568, 573, 576, 579, 580, 586, 587, 592, 594, 596, 599, 610,
      611, 625, 626, 634, 645, 652, 653, 658, 659, 661, 664};
  {  // plain text
    auto _xs = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // regex
    auto _xs = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // multiple threads
    auto _xs = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, 4, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers
    auto _xs = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, 1, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads
    auto _xs = xs::extern_search<xs::line_indices>(pattern, file_path,
                                                   meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads and regex pattern
    auto _xs = xs::extern_search<xs::line_indices>(re_pattern, file_path,
                                                   meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // compression
    auto _xs = xs::extern_search<xs::line_indices>(
        re_pattern, "test/files/dummy.xslz4", "test/files/dummy.xslz4.meta", 4,
        2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
}

TEST(ExternSearcherTest, lines) {
  {  // plain text
    auto _xs =
        xs::extern_search<xs::lines>(pattern, file_path, meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 133);
  }
  {  // regex
    auto _xs = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, 1, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 137);
  }
  {  // multiple threads
    auto _xs =
        xs::extern_search<xs::lines>(pattern, file_path, meta_file_path, 4, 1);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 133);
  }
  {  // multiple readers
    auto _xs =
        xs::extern_search<xs::lines>(pattern, file_path, meta_file_path, 1, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 133);
  }
  {  // multiple readers and threads
    auto _xs =
        xs::extern_search<xs::lines>(pattern, file_path, meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 133);
  }
  {  // multiple readers and threads and regex pattern
    auto _xs = xs::extern_search<xs::lines>(re_pattern, file_path,
                                            meta_file_path, 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 137);
  }
  {  // compression
    auto _xs =
        xs::extern_search<xs::lines>(re_pattern, "test/files/dummy.xslz4",
                                     "test/files/dummy.xslz4.meta", 4, 2);
    _xs->join();
    auto res = _xs->getResult()->copyResultSafe();
    ASSERT_EQ(res.size(), 137);
  }
}

/*
TEST(ExternSearcherTest, full) {
  const std::vector<size_t> line_indices = {
      1,   5,   8,   10,  12,  13,  14,  35,  37,  44,  47,  54,  65,  71,  79,
      86,  93,  95,  100, 102, 103, 112, 120, 131, 137, 140, 141, 152, 160, 162,
      164, 166, 169, 175, 185, 186, 189, 198, 203, 209, 220, 225, 227, 231, 235,
      240, 258, 260, 264, 269, 281, 289, 310, 318, 323, 326, 329, 330, 332, 335,
      339, 348, 349, 351, 357, 359, 361, 363, 366, 367, 371, 373, 377, 380, 385,
      390, 403, 406, 408, 411, 420, 421, 426, 429, 440, 453, 459, 465, 466, 467,
      476, 480, 485, 488, 492, 497, 505, 506, 508, 515, 523, 525, 528, 534, 537,
      541, 542, 548, 556, 558, 560, 561, 568, 573, 576, 579, 580, 586, 587, 592,
      594, 596, 599, 610, 611, 625, 626, 634, 645, 658, 659, 661, 664};
  const std::vector<size_t> regex_line_indices = {
      1,   5,   8,   10,  12,  13,  14,  35,  37,  44,  47,  54,  65,  71,
      79,  81,  86,  93,  95,  100, 102, 103, 112, 120, 131, 137, 140, 141,
      152, 160, 162, 164, 166, 169, 175, 185, 186, 189, 198, 203, 209, 220,
      225, 227, 231, 235, 240, 258, 260, 264, 269, 281, 289, 302, 310, 318,
      323, 326, 329, 330, 332, 335, 339, 348, 349, 351, 357, 359, 361, 363,
      366, 367, 371, 373, 377, 380, 385, 390, 403, 406, 408, 411, 420, 421,
      426, 429, 440, 453, 459, 465, 466, 467, 476, 480, 485, 488, 492, 497,
      505, 506, 508, 515, 523, 525, 528, 534, 537, 541, 542, 548, 556, 558,
      560, 561, 568, 573, 576, 579, 580, 586, 587, 592, 594, 596, 599, 610,
      611, 625, 626, 634, 645, 652, 653, 658, 659, 661, 664};
  const std::vector<size_t> match_byte_offsets = {
      567,    1524,   2618,   3259,   3540,   4009,   4071,   8395,   9045,
      10770,  11290,  13386,  16684,  18207,  20585,  22272,  24272,  24713,
      25667,  25823,  26496,  26787,  29117,  32111,  35073,  36484,  36695,
      37541,  37568,  37921,  40577,  40661,  42145,  42628,  43104,  43838,
      43968,  44851,  44910,  46793,  48951,  48998,  49613,  51773,  52911,
      53066,  54435,  56924,  58697,  59452,  60569,  62030,  62061,  63528,
      68569,  68828,  69216,  69624,  70802,  71856,  74898,  76786,  82871,
      84551,  84659,  85904,  87073,  87706,  88132,  89065,  90223,  91563,
      93953,  94393,  94898,  96407,  96590,  97176,  97813,  99014,  99258,
      100294, 100566, 101352, 101988, 103264, 103380, 104975, 105041, 108304,
      109036, 109812, 111008, 113091, 113300, 115122, 116301, 118928, 122337,
      123475, 125210, 125472, 125535, 125801, 128326, 129939, 131143, 131818,
      132707, 132745, 133594, 136407, 137060, 137870, 139918, 142424, 142919,
      143707, 145296, 146293, 147033, 147452, 149181, 150752, 151190, 151419,
      151458, 151609, 153092, 154041, 155175, 156048, 156403, 158137, 158682,
      160076, 160518, 161341, 162334, 165204, 165324, 165580, 165623, 169338,
      169844, 171450, 174841, 178365, 178859, 179278, 179375, 180045};
  const std::vector<size_t> regex_match_byte_offsets = {
      567,    1524,   2618,   3259,   3540,   4009,   4071,   8395,   9045,
      10770,  11290,  13386,  16684,  18207,  20585,  21221,  22272,  24272,
      24713,  25667,  25823,  26496,  26787,  29117,  32111,  35073,  36484,
      36695,  37541,  37568,  37921,  40577,  40661,  42145,  42628,  43104,
      43838,  43968,  44851,  44910,  46793,  48951,  48998,  49613,  51773,
      52911,  53066,  54435,  56924,  58697,  59452,  60569,  62030,  62061,
      63528,  68569,  68828,  69216,  69624,  70802,  71856,  74898,  76786,
      81033,  82871,  84551,  84659,  85904,  87073,  87706,  88132,  89065,
      90223,  91563,  93953,  94393,  94898,  96407,  96590,  97176,  97813,
      99014,  99258,  100294, 100566, 101352, 101988, 103264, 103380, 104975,
      105041, 108304, 109036, 109812, 111008, 113091, 113300, 115122, 116301,
      118928, 122337, 123475, 125210, 125472, 125535, 125801, 128326, 129939,
      131143, 131818, 132707, 132745, 133594, 136407, 137060, 137870, 139918,
      142424, 142919, 143707, 145296, 146293, 147033, 147452, 149181, 150752,
      151190, 151419, 151458, 151609, 153092, 154041, 155175, 156048, 156403,
      158137, 158682, 160076, 160518, 161341, 162334, 165204, 165324, 165580,
      165623, 169338, 169844, 171450, 174841, 176726, 177359, 178365, 178859,
      179278, 179375, 180045};
  const std::vector<size_t> line_byte_offsets = {
      421,    1375,   2507,   3251,   3521,   3626,   4031,   8348,   8704,
      10683,  11266,  13230,  16464,  17925,  20440,  22120,  24177,  24506,
      25651,  26404,  26629,  28994,  31761,  34689,  36324,  37384,  37687,
      40361,  42130,  42513,  43078,  43828,  44749,  46614,  48608,  48973,
      49557,  51679,  52849,  54244,  56808,  58435,  59113,  60474,  61639,
      63252,  68337,  69165,  70554,  71778,  74667,  76677,  82749,  84440,
      85752,  87014,  87690,  88123,  89065,  90107,  91395,  93943,  94104,
      94684,  96004,  96539,  97104,  97765,  98737,  99227,  100051, 100452,
      101299, 101975, 103234, 104926, 108146, 108916, 109771, 110946, 113041,
      113125, 115093, 116154, 118494, 122205, 123440, 124857, 125265, 125677,
      128227, 129668, 130974, 131790, 132671, 133527, 136283, 136778, 137752,
      139849, 142222, 142736, 143634, 145272, 146003, 146848, 147212, 149071,
      150699, 151182, 151378, 151600, 153031, 154035, 155153, 155884, 156096,
      158024, 158318, 160013, 160457, 161091, 162073, 165069, 165272, 169177,
      169663, 171346, 174725, 178340, 178565, 179272, 179847};
  const std::vector<size_t> regex_line_byte_offsets = {
      421,    1375,   2507,   3251,   3521,   3626,   4031,   8348,   8704,
      10683,  11266,  13230,  16464,  17925,  20440,  21153,  22120,  24177,
      24506,  25651,  26404,  26629,  28994,  31761,  34689,  36324,  37384,
      37687,  40361,  42130,  42513,  43078,  43828,  44749,  46614,  48608,
      48973,  49557,  51679,  52849,  54244,  56808,  58435,  59113,  60474,
      61639,  63252,  68337,  69165,  70554,  71778,  74667,  76677,  80671,
      82749,  84440,  85752,  87014,  87690,  88123,  89065,  90107,  91395,
      93943,  94104,  94684,  96004,  96539,  97104,  97765,  98737,  99227,
      100051, 100452, 101299, 101975, 103234, 104926, 108146, 108916, 109771,
      110946, 113041, 113125, 115093, 116154, 118494, 122205, 123440, 124857,
      125265, 125677, 128227, 129668, 130974, 131790, 132671, 133527, 136283,
      136778, 137752, 139849, 142222, 142736, 143634, 145272, 146003, 146848,
      147212, 149071, 150699, 151182, 151378, 151600, 153031, 154035, 155153,
      155884, 156096, 158024, 158318, 160013, 160457, 161091, 162073, 165069,
      165272, 169177, 169663, 171346, 174725, 176685, 177091, 178340, 178565,
      179272, 179847};
  {  // plain text
    auto _xs =
        xs::extern_search<xs::full>(pattern, file_path, meta_file_path, 1, 1);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 152);
    ASSERT_EQ(match_bo, match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 133);
    ASSERT_EQ(line_bo, line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 133);
    ASSERT_EQ(line_ind, line_indices);
    ASSERT_EQ(lines.size(), 133);
  }
  {  // regex
    auto _xs = xs::extern_search<xs::full>(re_pattern, file_path,
                                           meta_file_path, 1, 1);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 156);
    ASSERT_EQ(match_bo, regex_match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 137);
    ASSERT_EQ(line_bo, regex_line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 137);
    ASSERT_EQ(line_ind, regex_line_indices);
    ASSERT_EQ(lines.size(), 137);
  }
  {  // multiple threads
    auto _xs =
        xs::extern_search<xs::full>(pattern, file_path, meta_file_path, 4, 1);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 152);
    ASSERT_EQ(match_bo, match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 133);
    ASSERT_EQ(line_bo, line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 133);
    ASSERT_EQ(line_ind, line_indices);
    ASSERT_EQ(lines.size(), 133);
  }
  {  // multiple readers
    auto _xs =
        xs::extern_search<xs::full>(pattern, file_path, meta_file_path, 1, 2);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 152);
    ASSERT_EQ(match_bo, match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 133);
    ASSERT_EQ(line_bo, line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 133);
    ASSERT_EQ(line_ind, line_indices);
    ASSERT_EQ(lines.size(), 133);
  }
  {  // multiple readers and threads
    auto _xs =
        xs::extern_search<xs::full>(pattern, file_path, meta_file_path, 4, 2);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 152);
    ASSERT_EQ(match_bo, match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 133);
    ASSERT_EQ(line_bo, line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 133);
    ASSERT_EQ(line_ind, line_indices);
    ASSERT_EQ(lines.size(), 133);
  }
  {  // multiple readers and threads and regex pattern
    auto _xs = xs::extern_search<xs::full>(re_pattern, file_path,
                                           meta_file_path, 4, 2);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 156);
    ASSERT_EQ(match_bo, regex_match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 137);
    ASSERT_EQ(line_bo, regex_line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 137);
    ASSERT_EQ(line_ind, regex_line_indices);
    ASSERT_EQ(lines.size(), 137);
  }
  {  // compression
    auto _xs = xs::extern_search<xs::full>(re_pattern, "test/files/dummy.xslz4",
                                           "test/files/dummy.xslz4.meta", 4, 2);
    _xs->join();
    std::vector<size_t> match_bo;
    std::vector<size_t> line_bo;
    std::vector<size_t> line_ind;
    std::vector<std::string> lines;
    for (auto& r : *_xs->getResult()->getLockedResult()) {
      match_bo.insert(match_bo.end(), r._byte_offsets_match.begin(),
                      r._byte_offsets_match.end());
      line_bo.insert(line_bo.end(), r._byte_offsets_line.begin(),
                     r._byte_offsets_line.end());
      line_ind.insert(line_ind.end(), r._line_indices.begin(),
                      r._line_indices.end());
      lines.insert(lines.end(), r._lines.begin(), r._lines.end());
    }
    std::sort(match_bo.begin(), match_bo.end());
    std::sort(line_bo.begin(), line_bo.end());
    std::sort(line_ind.begin(), line_ind.end());
    std::sort(lines.begin(), lines.end());
    ASSERT_EQ(match_bo.size(), 156);
    ASSERT_EQ(match_bo, regex_match_byte_offsets);
    ASSERT_EQ(line_bo.size(), 137);
    ASSERT_EQ(line_bo, regex_line_byte_offsets);
    ASSERT_EQ(line_ind.size(), 137);
    ASSERT_EQ(line_ind, regex_line_indices);
    ASSERT_EQ(lines.size(), 137);
  }
}
 */