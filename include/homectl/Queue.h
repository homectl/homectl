#pragma once

#include <initializer_list>
#include <mutex>

template <typename T, int Capacity>
class Queue {
 public:
  constexpr Queue() : data_{}, end_(data_) {}
  constexpr Queue(std::initializer_list<T> init) : Queue() {
    for (T v : init) {
      add(std::move(v));  // ignores failures
    }
  }

  constexpr bool add(T &&value) {
    if (end_ == data_ + Capacity) {
      return false;
    }
    *end_ = std::move(value);
    ++end_;
    return true;
  }

  constexpr void clear() { end_ = data_; }

  constexpr int size() const { return end() - begin(); }

  constexpr T const *begin() const { return data_; }
  constexpr T const *end() const { return end_; }

 private:
  T data_[Capacity];
  T *end_;
};

template <typename T, int Capacity>
class ThreadSafeQueue {
  using Lock = std::lock_guard<std::mutex>;

 public:
  bool add(T &&value) {
    Lock const guard(mtx_);
    return queue_.add(std::move(value));
  }

  int size() {
    Lock const guard(mtx_);
    return queue_.size();
  }

  template <typename F>
  void consume(F f) {
    Lock const guard(mtx_);
    for (T const &v : queue_) {
      f(v);
    }
    queue_.clear();
  }

 private:
  Queue<T, Capacity> queue_;
  std::mutex mtx_;
};
