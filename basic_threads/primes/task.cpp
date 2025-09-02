#include "task.h"

#include <mutex>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cmath>
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
#include <stdexcept>
#include <vector>
=======
>>>>>>> 310b68f (try2)
=======
#include <vector>
>>>>>>> 1be963e (try4)
=======
>>>>>>> f21bcc7 (try_reload)
=======
#include <stdexcept>
#include <vector>
>>>>>>> 3b6235c (try_master)

bool IsPrimeNumber(uint64_t number) {
    if(number == 1 || number == 0){
        return false;
    }
    for (int i = 2; i <= sqrt(number); i++) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}

PrimeNumbersSet::PrimeNumbersSet() : nanoseconds_under_mutex_(0), nanoseconds_waiting_mutex_(0) {}

bool PrimeNumbersSet::IsPrime(uint64_t number) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    return primes_.find(number) != primes_.end();
}

uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    auto it = primes_.upper_bound(number);
    if (it != primes_.end()) {
        return *it;
    } else {
<<<<<<< HEAD
<<<<<<< HEAD
        throw std::invalid_argument("Next prime is invalid");;
=======
        return 0;
>>>>>>> 310b68f (try2)
=======
        throw std::invalid_argument("Next prime is invalid");;
>>>>>>> 3b6235c (try_master)
    }
}

void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to) {
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    std::vector<uint64_t> local_primes;

=======
>>>>>>> f21bcc7 (try_reload)
=======
    std::vector<uint64_t> local_primes;

>>>>>>> 3b6235c (try_master)
    for (uint64_t i = from; i < to; i++) {
        if (IsPrimeNumber(i)) {
            local_primes.push_back(i);
        }
    }
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 3b6235c (try_master)

    auto start_wait_time = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(set_mutex_);
    auto end_wait_time = std::chrono::steady_clock::now();

    auto start_under_mutex_time = std::chrono::steady_clock::now();
    primes_.insert(local_primes.begin(), local_primes.end());
    auto end_under_mutex_time = std::chrono::steady_clock::now();

    nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_wait_time - start_wait_time).count();
    nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_under_mutex_time - start_under_mutex_time).count();
<<<<<<< HEAD
=======
=======
    std::vector<uint64_t> local_primes;

>>>>>>> 1be963e (try4)
    for (uint64_t i = from; i < to; i++) {
        if (IsPrimeNumber(i)) {
            local_primes.push_back(i);
        }
    }
<<<<<<< HEAD
>>>>>>> 310b68f (try2)
=======

    auto start_wait_time = std::chrono::steady_clock::now();
    std::unique_lock<std::mutex> lock(set_mutex_);
    auto end_wait_time = std::chrono::steady_clock::now();

    auto start_under_mutex_time = std::chrono::steady_clock::now();
    primes_.insert(local_primes.begin(), local_primes.end());
    auto end_under_mutex_time = std::chrono::steady_clock::now();

    nanoseconds_waiting_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_wait_time - start_wait_time).count();
    nanoseconds_under_mutex_ += std::chrono::duration_cast<std::chrono::nanoseconds>(end_under_mutex_time - start_under_mutex_time).count();
>>>>>>> 1be963e (try4)
=======
>>>>>>> f21bcc7 (try_reload)
=======
>>>>>>> 3b6235c (try_master)
}

size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    size_t primes_in_range = std::distance(primes_.lower_bound(from), primes_.lower_bound(to));
    return primes_in_range;
}

uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const {
    std::lock_guard<std::mutex> lock(set_mutex_);
    if (primes_.empty()) {
        return 0;
    } else {
<<<<<<< HEAD
<<<<<<< HEAD
        return *primes_.rbegin();
=======
        *primes_.rbegin();
>>>>>>> 310b68f (try2)
=======
        return *primes_.rbegin();
>>>>>>> e56c4e7 (try3)
    }
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const {
<<<<<<< HEAD
<<<<<<< HEAD
    auto time_under_wait = (std::chrono::nanoseconds) nanoseconds_waiting_mutex_.load();
=======
    std::chrono::nanoseconds time_under_wait = (std::chrono::nanoseconds) nanoseconds_waiting_mutex_.load();
>>>>>>> 310b68f (try2)
=======
    auto time_under_wait = (std::chrono::nanoseconds) nanoseconds_waiting_mutex_.load();
>>>>>>> 3b6235c (try_master)
    return time_under_wait;
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const {
<<<<<<< HEAD
<<<<<<< HEAD
    auto time_under_mutex = (std::chrono::nanoseconds) nanoseconds_under_mutex_.load();
=======
    std::chrono::nanoseconds time_under_mutex = (std::chrono::nanoseconds) nanoseconds_under_mutex_.load();
>>>>>>> 310b68f (try2)
=======
    auto time_under_mutex = (std::chrono::nanoseconds) nanoseconds_under_mutex_.load();
>>>>>>> 3b6235c (try_master)
    return time_under_mutex;
}
