#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

/// Thread safe queue implementation
/// Fuck C++
template <class T>
class Queue {
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable conditionVariable;

public:
    Queue() : queue(), mutex(), conditionVariable() {}

    /// Add an element to the queue.
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(t));
        conditionVariable.notify_one();
    }

    /// Get the "front"-element.
    /// If the queue is empty, wait till a element is avaiable.
    T dequeue() {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.empty()) {
            // release lock as long as the wait and reaquire it afterwards.
            conditionVariable.wait(lock);
        }

        auto val = std::move(queue.front());
        queue.pop();
        return std::move(val);
    }

    /// Determine if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }
};
