/**
 * Copyright 2023, Leon Freist (https://github.com/lfreist)
 * Author: Leon Freist <freist.leon@gmail.com>
 *
 * This file is part of x-search.
 */

#pragma once

#include <concepts>
#include <vector>
#include <optional>
#include <iostream>

namespace xs {

template <typename DefaultDataT>
concept DefaultDataC = requires(DefaultDataT data, size_t size) {
  { std::is_default_constructible_v<DefaultDataT> };
  { data.resize(size) };
  { data.data() } -> std::convertible_to<char*>;
};

template <typename Task, typename DataT>
concept ReaderC = requires(Task task) {
  { task.read() } -> std::same_as<std::optional<DataT>>;
  { std::is_move_constructible<Task>::value };
  { std::is_move_assignable<Task>::value };
};

template <typename Res, typename PartRes>
concept ResultC = requires(Res result, PartRes& partial_result) {
  { result.add(partial_result) };
  { result.get() } -> std::same_as<const std::vector<PartRes>&>;
  { std::is_move_constructible<Res>::value };
  { std::is_move_assignable<Res>::value };
};

template <typename Task, typename PartRes, typename DataT>
concept SearcherC = requires(Task task, DataT* data) {
  { task.search(data) } -> std::same_as<std::optional<PartRes>>;
  { std::is_move_constructible<Task>::value };
  { std::is_move_assignable<Task>::value };
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