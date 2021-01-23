#include "homectl/Queue.h"

static constexpr bool testQueueAdd() {
  Queue<int, 5> queue;
  if (!queue.add(1)) return false;
  if (!queue.add(2)) return false;
  if (!queue.add(3)) return false;
  if (!queue.add(4)) return false;
  if (!queue.add(5)) return false;
  if (queue.add(6)) return false;

  return true;
}

static_assert(testQueueAdd(), "testQueueAdd failed");

static constexpr bool testQueueIter() {
  Queue<int, 5> queue{1, 2, 3, 4, 5};

  int last = 0;
  for (int v : queue) {
    if (v != last + 1) return false;
    last = v;
  }
  return true;
}

static_assert(testQueueIter(), "testQueueIter failed");

static constexpr bool testQueueSize() {
  Queue<int, 5> queue;

  if (queue.size() != 0) return false;
  queue.add(1);
  if (queue.size() != 1) return false;
  queue.add(1);
  if (queue.size() != 2) return false;

  return true;
}

static_assert(testQueueSize(), "testQueueSize failed");

static constexpr bool testQueueMove() {
  Queue<int, 5> queue{1, 2, 3, 4, 5};

  Queue<int, 5> queue2 = std::move(queue);
  if (queue.size() != 0) return false;
  if (queue2.size() != 5) return false;

  return true;
}

static_assert(testQueueMove(), "testQueueMove failed");
