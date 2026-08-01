#include "thread_pool.h"

ThreadPool::ThreadPool(const size_t num_threads) : barrier_(num_threads + 1)
{
    for (size_t i = 0; i < num_threads; ++i)
    {
        threads_.emplace_back([this, i]() { RunThread(i); });
    }
}

ThreadPool::~ThreadPool()
{
    running_ = false;
    barrier_.arrive_and_wait(); // wait till each thread exist the loop
}

void ThreadPool::ForEach(const int count, const std::function<void(int)>& task)
{
    max_index_ = count;
    task_ = task;
    current_index_ = 0;

    barrier_.arrive_and_wait(); // activating threads

    barrier_.arrive_and_wait(); // waiting for threads to finish
}

void ThreadPool::RunThread(int thread_id)
{
    while (running_)
    {
        // waiting for the main thread to give a sign to start working
        barrier_.arrive_and_wait();
        if (!running_)
            break;

        while (true)
        {
            // scheduling
            int current_index = current_index_.fetch_add(1, std::memory_order_relaxed);
            if (current_index >= max_index_)
                break;

            // do work
            task_(current_index);
        }

        barrier_.arrive_and_wait();
    }
}
