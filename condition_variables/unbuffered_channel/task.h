#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>


class TimeOut : public std::exception {
    const char* what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T& data) {
        std::unique_lock lock(mtx_);
        cv_.wait(lock, [this] { return !data_.has_value(); });
        data_ = data;
        cv_.notify_all();
        cv_.wait(lock);
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock lock(mtx_);
        if (!cv_.wait_for(lock, timeout, [this] { return data_.has_value(); })) {
            throw TimeOut();
        }
        // cv_.wait_for(lock, timeout, [this] { return data_.has_value();});
        T get_ans = data_.value();
        data_.reset();
        cv_.notify_all();
        return get_ans;
    }

private:
    std::optional<T> data_;
    std::mutex mtx_;
    std::condition_variable cv_; 
};
