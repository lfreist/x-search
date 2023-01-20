// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/ExternSearcher.h>

/*
TEST(ExternSearcherTest, count) {
  {  // plain text
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 1);
    ASSERT_EQ(res, 152);
  }
  {  // regex
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 1, 1);
    ASSERT_EQ(res, 156);
  }
  {  // multiple threads
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 1);
    ASSERT_EQ(res, 152);
  }
  {  // multiple readers
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 2);
    ASSERT_EQ(res, 152);
  }
  {  // multiple readers and threads
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 2);
    ASSERT_EQ(res, 152);
  }
  {  // multiple readers and threads and regex pattern
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 4, 2);
    ASSERT_EQ(res, 156);
  }
  {  // compression
    size_t res = xs::ExternSearcher<xs::restype::count>::search(
        "test/files/dummy.sflz4", "test/files/dummy.sflz4.meta", "ov[e|i]r", 4,
        2);
    ASSERT_EQ(res, 156);
  }
}

TEST(ExternSearcherTest, count_lines) {
  {  // plain text
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 1);
    ASSERT_EQ(res, 133);
  }
  {  // regex
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 1, 1);
    ASSERT_EQ(res, 137);
  }
  {  // multiple threads
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 1);
    ASSERT_EQ(res, 133);
  }
  {  // multiple readers
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 2);
    ASSERT_EQ(res, 133);
  }
  {  // multiple readers and threads
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 2);
    ASSERT_EQ(res, 133);
  }
  {  // multiple readers and threads and regex pattern
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 4, 2);
    ASSERT_EQ(res, 137);
  }
  {  // compression
    size_t res = xs::ExternSearcher<xs::restype::count_lines>::search(
        "test/files/dummy.sfzst", "test/files/dummy.sfzst.meta", "ov[e|i]r", 4,
        2);
    ASSERT_EQ(res, 137);
  }
}


template <typename T>
void print(std::vector<T>& v) {
  std::cout << "{";
  for (auto& e : v) {
    std::cout << e << ", ";
  }
  std::cout << std::endl;
}


TEST(ExternSearcherTest, byte_positions) {
  std::vector<size_t> plain_res = {
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
  std::vector<size_t> regex_res = {
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
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 1);
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // regex
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 1,
            1);
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
  {  // multiple threads
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 1);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 2);
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 152);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads and regex pattern
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|qi]r", 4,
            2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
  {  // compression
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::byte_positions>::search(
            "test/files/dummy.sflz4", "test/files/dummy.sflz4.meta",
            "ov[e|qi]r", 4, 2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 156);
    ASSERT_EQ(res, regex_res);
  }
}

TEST(ExternSearcherTest, line_numbers) {
  std::vector<size_t> plain_res = {
      2,   6,   9,   11,  13,  14,  15,  36,  38,  45,  48,  55,  66,  72,  80,
      87,  94,  96,  101, 103, 104, 113, 121, 132, 138, 141, 142, 153, 161, 163,
      165, 167, 170, 176, 186, 187, 190, 199, 204, 210, 221, 226, 228, 232, 236,
      241, 259, 261, 265, 270, 282, 290, 311, 319, 324, 327, 330, 331, 333, 336,
      340, 349, 350, 352, 358, 360, 362, 364, 367, 368, 372, 374, 378, 381, 386,
      391, 404, 407, 409, 412, 421, 422, 427, 430, 441, 454, 460, 466, 467, 468,
      477, 481, 486, 489, 493, 498, 506, 507, 509, 516, 524, 526, 529, 535, 538,
      542, 543, 549, 557, 559, 561, 562, 569, 574, 577, 580, 581, 587, 588, 593,
      595, 597, 600, 611, 612, 626, 627, 635, 646, 659, 660, 662, 665};
  std::vector<size_t> regex_res = {
      2,   6,   9,   11,  13,  14,  15,  36,  38,  45,  48,  55,  66,  72,
      80,  82,  87,  94,  96,  101, 103, 104, 113, 121, 132, 138, 141, 142,
      153, 161, 163, 165, 167, 170, 176, 186, 187, 190, 199, 204, 210, 221,
      226, 228, 232, 236, 241, 259, 261, 265, 270, 282, 290, 303, 311, 319,
      324, 327, 330, 331, 333, 336, 340, 349, 350, 352, 358, 360, 362, 364,
      367, 368, 372, 374, 378, 381, 386, 391, 404, 407, 409, 412, 421, 422,
      427, 430, 441, 454, 460, 466, 467, 468, 477, 481, 486, 489, 493, 498,
      506, 507, 509, 516, 524, 526, 529, 535, 538, 542, 543, 549, 557, 559,
      561, 562, 569, 574, 577, 580, 581, 587, 588, 593, 595, 597, 600, 611,
      612, 626, 627, 635, 646, 653, 654, 659, 660, 662, 665};
  {  // plain text
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 1);
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // regex
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|i]r", 1,
            1);
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // multiple threads
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 1);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 1, 2);
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "over", 4, 2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 133);
    ASSERT_EQ(res, plain_res);
  }
  {  // multiple readers and threads and regex pattern
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.txt", "test/files/dummy.sf.meta", "ov[e|qi]r", 4,
            2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
  {  // compression
    std::vector<size_t> res =
        xs::ExternSearcher<xs::restype::line_numbers>::search(
            "test/files/dummy.sfzst", "test/files/dummy.sfzst.meta",
            "ov[e|qi]r", 4, 2);
    std::sort(res.begin(), res.end());
    ASSERT_EQ(res.size(), 137);
    ASSERT_EQ(res, regex_res);
  }
}
*/