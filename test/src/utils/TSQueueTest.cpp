// Copyright 2022, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <gtest/gtest.h>
#include <xsearch/utils/TSQueue.h>

#include <string>
#include <thread>

namespace xs::utils {

void add_to_queue(int val, int num, TSQueue<int>* q) {
  for (int i = 0; i < num; ++i) {
    q->push(val);
  }
}

void pop_all_from_queue(TSQueue<int>* q, std::vector<int>* res) {
  while (true) {
    auto opt_val = q->pop();
    if (!opt_val.has_value()) {
      break;
    }
    res->push_back(opt_val.value());
  }
}

TEST(TSQueueTest, constructor) {
  {
    TSQueue<int> q;

    ASSERT_EQ(q._maxSize, 100);
    ASSERT_TRUE(q._queue.empty());
    ASSERT_FALSE(q._closed);
  }
  {
    TSQueue<int> q(5);

    ASSERT_EQ(q._maxSize, 5);
    ASSERT_TRUE(q._queue.empty());
    ASSERT_FALSE(q._closed);
  }
}

TEST(TSQueueTest, push) {
  {  // single thread
    TSQueue<int> q;
    q.push(5);

    ASSERT_EQ(q.size(), 1);
    ASSERT_EQ(q.pop(), 5);
  }
  {
    TSQueue<int> q(10001);

    std::thread t0 = std::thread(add_to_queue, 0, 5000, &q);
    std::thread t1 = std::thread(add_to_queue, 1, 5000, &q);

    t0.join();
    t1.join();

    ASSERT_EQ(q.size(), 10000);

    q.close();

    int count_zero = 0;
    int count_one = 0;
    while (true) {
      auto opt_value = q.pop();
      if (!opt_value.has_value()) {
        break;
      }
      if (opt_value.value() == 0) {
        count_zero++;
      } else if (opt_value.value() == 1) {
        count_one++;
      }
    }
    ASSERT_EQ(count_zero, 5000);
    ASSERT_EQ(count_one, 5000);
  }
  {
    TSQueue<int> q(20000);

    std::thread t0 = std::thread(add_to_queue, 0, 5000, &q);
    std::thread t1 = std::thread(add_to_queue, 1, 5000, &q);
    std::thread t2 = std::thread(add_to_queue, 2, 5000, &q);
    std::thread t3 = std::thread(add_to_queue, 3, 5000, &q);

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    ASSERT_EQ(q.size(), 20000);

    q.close();

    int count_zero = 0;
    int count_one = 0;
    int count_two = 0;
    int count_three = 0;

    while (true) {
      auto opt_value = q.pop();
      if (!opt_value.has_value()) {
        break;
      }
      if (opt_value.value() == 0) {
        count_zero++;
      } else if (opt_value.value() == 1) {
        count_one++;
      } else if (opt_value.value() == 2) {
        count_two++;
      } else if (opt_value.value() == 3) {
        count_three++;
      }
    }
    ASSERT_EQ(count_zero, 5000);
    ASSERT_EQ(count_one, 5000);
    ASSERT_EQ(count_two, 5000);
    ASSERT_EQ(count_three, 5000);
  }
}

TEST(TSQueueTest, pop) {
  {
    TSQueue<int> q;
    q.push(5);

    ASSERT_EQ(q.pop(), 5);
  }
  {
    TSQueue<int> q(10000);
    add_to_queue(0, 5000, &q);
    add_to_queue(1, 5000, &q);

    q.close();

    std::vector<int> v0;
    std::vector<int> v1;

    std::thread t0 = std::thread(pop_all_from_queue, &q, &v0);
    std::thread t1 = std::thread(pop_all_from_queue, &q, &v1);

    t0.join();
    t1.join();

    int count_zero = 0;
    int count_one = 0;
    for (auto v : v0) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      }
    }
    for (auto v : v1) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      }
    }

    ASSERT_EQ(count_zero, 5000);
    ASSERT_EQ(count_one, 5000);
  }
  {
    TSQueue<int> q(20000);
    add_to_queue(0, 5000, &q);
    add_to_queue(1, 5000, &q);
    add_to_queue(2, 5000, &q);
    add_to_queue(3, 5000, &q);

    q.close();

    std::vector<int> v0;
    std::vector<int> v1;
    std::vector<int> v2;
    std::vector<int> v3;

    std::thread t0(pop_all_from_queue, &q, &v0);
    std::thread t1(pop_all_from_queue, &q, &v1);
    std::thread t2(pop_all_from_queue, &q, &v2);
    std::thread t3(pop_all_from_queue, &q, &v3);

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    int count_zero = 0;
    int count_one = 0;
    int count_two = 0;
    int count_three = 0;
    for (auto v : v0) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      } else if (v == 2) {
        count_two++;
      } else if (v == 3) {
        count_three++;
      }
    }
    for (auto v : v1) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      } else if (v == 2) {
        count_two++;
      } else if (v == 3) {
        count_three++;
      }
    }
    for (auto v : v2) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      } else if (v == 2) {
        count_two++;
      } else if (v == 3) {
        count_three++;
      }
    }
    for (auto v : v3) {
      if (v == 0) {
        count_zero++;
      } else if (v == 1) {
        count_one++;
      } else if (v == 2) {
        count_two++;
      } else if (v == 3) {
        count_three++;
      }
    }

    ASSERT_EQ(count_zero, 5000);
    ASSERT_EQ(count_one, 5000);
    ASSERT_EQ(count_two, 5000);
    ASSERT_EQ(count_three, 5000);
  }
}

TEST(TSQueueTest, empty) {
  TSQueue<int> q;
  ASSERT_TRUE(q.empty());

  q.push(5);
  ASSERT_FALSE(q.empty());

  q.pop();
  ASSERT_TRUE(q.empty());

  q.push(5);
  ASSERT_FALSE(q.empty());

  q.pop();
  ASSERT_TRUE(q.empty());
}

TEST(TSQueueTest, size) {
  TSQueue<int> q(100);
  ASSERT_EQ(q.size(), 0);

  q.push(5);
  ASSERT_EQ(q.size(), 1);

  add_to_queue(1, 50, &q);
  ASSERT_EQ(q.size(), 51);

  add_to_queue(1, 49, &q);
  ASSERT_EQ(q.size(), 100);

  q.pop();
  ASSERT_EQ(q.size(), 99);
}

TEST(TSQueueTest, close_isClose) {
  TSQueue<int> q;
  ASSERT_FALSE(q.isClosed());

  q.close();
  ASSERT_TRUE(q.isClosed());
}

TEST(TSQueueTest, reset) {
  TSQueue<int> q;
  q.push(5);
  ASSERT_FALSE(q.empty());

  q.reset();
  ASSERT_TRUE(q.empty());

  q.close();
  ASSERT_TRUE(q.isClosed());

  q.reset();
  ASSERT_FALSE(q.isClosed());
}

}  // namespace xs::utils