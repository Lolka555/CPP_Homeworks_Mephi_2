#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount) : active(true) {
        for (size_t i = 0; i < threadCount; ++i) {
            threads.emplace_back([this]() { WorkerThread(); });
        }
    }

    ~ThreadPool() {
        Terminate(true);
    }

    void PushTask(const std::function<void()>& task) {
        std::unique_lock<std::mutex> lock(mtxQueue);
        if (!active) {
            throw std::runtime_error("Exit");
        }
        taskQueue.push(task);
        cvQueue.notify_one();
    }

    void Terminate(bool wait) {
        {
            std::unique_lock<std::mutex> lock(mtxQueue);
            active = false;
            if (!wait) {
                std::queue<std::function<void()>>().swap(taskQueue);
            }
        }
        cvQueue.notify_all();
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    bool IsActive() const {
        return active.load();
    }

    size_t QueueSize() const {
        std::unique_lock<std::mutex> lock(mtxQueue);
        return taskQueue.size();
    }

private:
    void WorkerThread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mtxQueue);
                cvQueue.wait(lock, [this]() { return !active || !taskQueue.empty(); });
                if (!active && taskQueue.empty()) {
                    return;
                }
                task = std::move(taskQueue.front());
                taskQueue.pop();
            }
            task();
        }
    }

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    mutable std::mutex mtxQueue;
    std::condition_variable cvQueue;
<<<<<<< HEAD
<<<<<<< HEAD
    
=======
>>>>>>> a8ebe90 (thread_pool)
=======
    
>>>>>>> 5d144a8 (thread_pool)
    std::atomic<bool> active;
};

// Код, помогающий в отладке

/*
 * Складывает числа на полуинтервале [from, to)
 */
