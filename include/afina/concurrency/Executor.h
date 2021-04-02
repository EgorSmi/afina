#ifndef AFINA_CONCURRENCY_EXECUTOR_H
#define AFINA_CONCURRENCY_EXECUTOR_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <iostream>
#include <chrono>
#include <algorithm>

namespace Afina {
namespace Concurrency {

/**
 * # Thread pool
 */
class Executor {
    enum class State {
        // Threadpool is fully operational, tasks could be added and get executed
        kRun,

        // Threadpool is on the way to be shutdown, no ned task could be added, but existing will be
        // completed as requested
        kStopping,

        // Threadppol is stopped
        kStopped
    };
public:
    Executor(std::string name, std::size_t size, std::size_t low_watermark, std::size_t high_watermark,
             std::size_t idle_time): state(State::kRun), _name(name), _max_queue_size(size), _low_watermark(low_watermark),
                                     _high_watermark(high_watermark), _idle_time(idle_time), _cur_queue_size(0), _working_threads(0)
    {
        std::unique_lock<std::mutex> lock(pool_mutex);
        for (std::size_t t=0; t < _low_watermark; t++)
        {
            threads.emplace_back(std::thread([this] {return perform(this); }));
        }
    }

    ~Executor()
    {
        this->Stop(true);
    }

    /**
     * Signal thread pool to stop, it will stop accepting new jobs and close threads just after each become
     * free. All enqueued jobs will be complete.
     *
     * In case if await flag is true, call won't return until all background jobs are done and all threads are stopped
     */
    void Stop(bool await = false)
    {
        std::unique_lock<std::mutex> state_lock(mutex);
        state = State::kStopping;
        empty_condition.notify_all(); // to stop
        if (await == true)
        {
            stop_condition.wait(state_lock);
        }
        if (threads.empty())
        {
            state = State::kStopped;
        }
    }

    /**
     * Add function to be executed on the threadpool. Method returns true in case if task has been placed
     * onto execution queue, i.e scheduled for execution and false otherwise.
     *
     * That function doesn't wait for function result. Function could always be written in a way to notify caller about
     * execution finished by itself
     */
    template <typename F, typename... Types> bool Execute(F &&func, Types... args) {
        // Prepare "task"
        auto exec = std::bind(std::forward<F>(func), std::forward<Types>(args)...);

        std::unique_lock<std::mutex> lock(mutex);
        if (state != State::kRun || _cur_queue_size >= _max_queue_size) {
            return false;
        }

        // Enqueue new task
        if ((threads.size() < _high_watermark) && (_cur_queue_size++ >= 0) && (threads.size() == _working_threads))
        {
            threads.emplace_back(std::thread([this] {return perform(this); }));
        }
        tasks.push_back(exec);
        empty_condition.notify_one();
        return true;
    }

private:
    // No copy/move/assign allowed
    Executor(const Executor &);            // = delete;
    Executor(Executor &&);                 // = delete;
    Executor &operator=(const Executor &); // = delete;
    Executor &operator=(Executor &&);      // = delete;

    /**
     * Main function that all pool threads are running. It polls internal task queue and execute tasks
     */
    friend void perform(Executor *executor)
    {
        std::unique_lock<std::mutex> lock(executor->mutex);
        bool goto_delete = false;
        while ((executor->state == State::kRun) || (!executor->tasks.empty()))
        {
            const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            std::function<void()> task; // task to perform
            while (executor->tasks.empty())
            {
                // wait cv
                auto cv_flag = executor->empty_condition.wait_until(lock,
                                                                    now + std::chrono::milliseconds(executor->_idle_time));
                if (((cv_flag == std::cv_status::timeout) && (executor->tasks.empty()) &&
                (executor->threads.size() > executor->_low_watermark))
                 || (executor->state != State::kRun))
                {
                    if (executor->state != State::kStopping)
                    {
                        // need to delete if not stopping
                        auto thread_id = std::this_thread::get_id();
                        auto it = std::find_if(executor->threads.begin(), executor->threads.end(),
                                               [thread_id] (std::thread &t) { return t.get_id() == thread_id; });
                        (*it).detach();
                        executor->threads.erase(it);
                        return;
                    }
                    else
                    {
                        goto_delete = true;
                        break;
                    }
                }
            }
            if (goto_delete)
            {
                continue;
            }
            task = std::move(executor->tasks.front()); // take task from queue
            executor->_cur_queue_size--;
            executor->tasks.pop_front(); // destroy first element
            executor->_working_threads++;
            lock.unlock();
            try
            {
                task();
            }
            catch(const std::exception& e)
            {
                std::cout<<e.what()<<std::endl;
            }
            lock.lock();
            executor->_working_threads--;
        }
        // Stopping thread pool
        auto thread_id = std::this_thread::get_id();
        auto it = std::find_if(executor->threads.begin(), executor->threads.end(),
                               [thread_id] (std::thread &t) { return t.get_id() == thread_id; });
        (*it).detach();
        executor->threads.erase(it);
        if (executor->threads.empty())
        {
            executor->state = State::kStopped;
            executor->stop_condition.notify_all();
        }
    }

    /**
     * Mutex to protect state below from concurrent modification
     */
    std::mutex mutex;

    /**
     * Conditional variable to await new data in case of empty queue
     */
    std::condition_variable empty_condition;

    std::condition_variable stop_condition;
    /**
     * Vector of actual threads that perform execution
     */
    std::vector<std::thread> threads;

    /**
     * Task queue
     */
    std::deque<std::function<void()>> tasks;

    /**
     * Flag to stop bg threads
     */
    State state;
    std::mutex pool_mutex;
    std::string _name; // name?? for what
    std::size_t _max_queue_size;
    std::size_t _low_watermark;
    std::size_t _high_watermark;
    std::size_t _idle_time;
    std::size_t _cur_queue_size;
    std::size_t _working_threads;
};

} // namespace Concurrency
} // namespace Afina

#endif // AFINA_CONCURRENCY_EXECUTOR_H
