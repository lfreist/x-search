/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <xsearch/DataChunk.h>

#include <concepts>

namespace xs {

template <typename Task, typename DataT = DataChunk>
concept ReaderC = requires(Task task) {
  { task.read() } -> std::same_as<std::optional<DataT>>;
  { std::is_move_constructible<Task>::value };
  { std::is_move_constructible<Task>::value };
};

template <typename Res, typename PartRes, typename ResIterator>
concept ResultC = requires(Res result, PartRes& partial_result) {
  { result.add(partial_result) } -> std::same_as<void>;
  { result.get() } -> std::same_as<std::vector<PartRes>&>;
  { std::is_move_constructible<Res>::value };
  { std::is_move_constructible<Res>::value };
  // { result.begin() } -> std::same_as<ResIterator>;
  // { result.end() } -> std::same_as<ResIterator>;
};

template <typename Task, typename PartRes, typename DataT = DataChunk>
concept SearcherC = requires(Task task, DataT* data) {
  { task.search(data) } -> std::same_as<std::optional<PartRes>>;
  { std::is_move_constructible<Task>::value };
  { std::is_move_constructible<Task>::value };
};

template <typename T>
concept InputStreamable = requires(std::istream& is, T& t) {
  { is >> t } -> std::convertible_to<std::istream &>;
};

template <typename T>
concept OutputStreamable = requires(std::ostream& os, T& data) {
  { os << data } -> std::convertible_to<std::ostream &>;
};

template <typename T>
concept Lockable = requires(T t) {
  { t.lock() } -> std::same_as<void>;
  { t.unlock() } -> std::same_as<void>;
};

template <typename T>
concept SharedLockable = requires(T t) {
  { t.lock_shared() } -> std::same_as<void>;
  { t.unlock_shared() } -> std::same_as<void>;
};

}  // namespace xs