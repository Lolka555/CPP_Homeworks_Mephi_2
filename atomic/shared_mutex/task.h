<<<<<<< HEAD
<<<<<<< HEAD
#pragma once

<<<<<<< HEAD
#include <atomic>
#include <thread>
=======
=======
>>>>>>> e5a2033 (shared)
#include <mutex>
<<<<<<< HEAD
>>>>>>> 76efc17 (check)

class SharedMutex {
public:
    SharedMutex() : reader_count(0), writer_flag(false) {}

    void lock() {
        while (true) {
            bool expected = false;
            if (writer_flag.compare_exchange_strong(expected, true)) {
                while (reader_count.load() != 0) {
                    std::this_thread::yield();
                }
                return;
            }
            std::this_thread::yield();
        }
    }

    void unlock() {
        writer_flag.store(false);
    }

    void lock_shared() {
        while (true) {
            while (writer_flag.load()) {
                std::this_thread::yield();
            }

            reader_count.fetch_add(1);
            if (!writer_flag.load()) {
                break;
            }
            reader_count.fetch_sub(1);
            std::this_thread::yield();
        }
    }

    void unlock_shared() {
        reader_count.fetch_sub(1);
    }
<<<<<<< HEAD

private:
    std::atomic<int> reader_count;
    std::atomic<bool> writer_flag;   
=======
private:
    std::mutex mtx;
>>>>>>> 76efc17 (check)
=======
#include <condition_variable>
=======
#pragma once

#include <atomic>
#include <thread>
>>>>>>> 781de90 (fix of shared)

class SharedMutex {
public:
    SharedMutex() : reader_count(0), writer_flag(false) {}

    void lock() {
        while (true) {
            bool expected = false;
            if (writer_flag.compare_exchange_strong(expected, true)) {
                while (reader_count.load() != 0) {
                    std::this_thread::yield();
                }
                return;
            }
            std::this_thread::yield();
        }
    }

    void unlock() {
        writer_flag.store(false);
    }

    void lock_shared() {
        while (true) {
            while (writer_flag.load()) {
                std::this_thread::yield();
            }

            reader_count.fetch_add(1);
            if (!writer_flag.load()) {
                break;
            }
            reader_count.fetch_sub(1);
            std::this_thread::yield();
        }
    }

    void unlock_shared() {
        reader_count.fetch_sub(1);
    }

private:
<<<<<<< HEAD
    std::mutex mtx;
    std::condition_variable cv;
    int reader_count;
    bool writer_active;
>>>>>>> 25ad917 (shared)
=======
    std::atomic<int> reader_count;
    std::atomic<bool> writer_flag;   
>>>>>>> 781de90 (fix of shared)
};
