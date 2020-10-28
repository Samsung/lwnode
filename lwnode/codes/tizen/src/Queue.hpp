#ifndef _QUEUE_HPP_
#define _QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>

namespace nescargot {

template <typename T>
class Queue {
public:
  Queue() = default;
  Queue(const Queue&) = delete;
  Queue& operator=(const Queue&) = delete;
  Queue(const Queue&&) = delete;
  Queue& operator=(const Queue&&) = delete;

  void push(const T& item)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(item);
    lock.unlock();
    m_cv.notify_one();
  }

  T pop()
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
      m_cv.wait(lock);
    }
    T item = m_queue.front();
    m_queue.pop();
    return item;
  }

  bool pop(T& item, const unsigned int millisecondWaitingPeriod)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
      auto timeout = std::chrono::milliseconds(millisecondWaitingPeriod);
      if (m_cv.wait_for(lock, timeout) == std::cv_status::timeout) {
        return false;
      }
    }

    item = m_queue.front();
    m_queue.pop();
    return true;
  }

  bool empty() {
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cv;
};

} // namespace nescargot

#endif // _QUEUE_HPP_
