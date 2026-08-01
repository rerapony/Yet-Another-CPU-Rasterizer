#pragma once
#include <barrier>
#include <functional>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency() - 1);
    ~ThreadPool();

    void ForEach(int count, const std::function<void(int)>& task);

private:
    void RunThread(int thread_id);

    std::barrier<> barrier_;
    std::vector<std::jthread> threads_;
    std::atomic<bool> running_ = true;

    int max_index_ = 0;
    std::atomic<int> current_index_ = 0;

    std::function<void(int)> task_;
};
