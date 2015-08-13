#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <type_traits>
#include <vector>
#include <queue>
#include <cstddef>

int ocm_main(int argc, char** argv);

namespace ocm {

class thread {
public:
    template<typename Fun, typename... Args>
    thread(Fun f, Args&&... args);

    // Threads can't be copied or moved, nor default constructed.
    thread() = delete;
    thread(const thread&) = delete;
    thread(thread&&) = delete;

    // Allow us to find the current thread
    static thread* current()
    {
        return self_;
    }

    // Actions that threads perform on themselves, implicitly apply to
    // "the current thread"
    static void yield();
    static void sleep(double seconds);

    // Actions that threads perform on themselves
    void suspend();

    // Actions threads perform on other threads.
    void join();
    void resume();

private:
    // On OS X, C++11 thread_local support isn't in Xcode's version of clang.
    #if defined(__apple_build_version__)
        static __thread thread* self_;
    #else
        static thread_local thread* self_;
    #endif

    enum state_t {
        RUNNING,
        STARTING,
        SUSPENDED,
    WOKEN,
        DONE
    };

    volatile state_t state_;

    // To support suspend/resume
    std::mutex susp_mutex_;
    std::condition_variable susp_cv_;

    // Underlying OS thread
    std::thread thread_;
};

// Simple barrier class (only uses public API of ocm::thread)
class barrier {
public:
    barrier(size_t count = 0);
    void arrive();
    void release();
    ssize_t adjust(ssize_t count);
    void reset(size_t count);

    // Barriers can't be copied or moved.
    barrier(const barrier&) = delete;
    barrier(barrier&&) = delete;

private:
    ssize_t arrivalsNeeded_ = 0;
    std::vector<thread*> waitingThreads_;
};

// Simple semaphore class (only uses public API of ocm::thread)
class semaphore {
public:
    semaphore(size_t count = 0);

    // Semaphores can't be copied or moved.
    semaphore(const semaphore&) = delete;
    semaphore(semaphore&&) = delete;

    void post();
    void wait();
    void reset(size_t count);

private:
    size_t count_ = 0;
    std::queue<thread*> waitingThreads_;
};

// Simple condition class (only uses public API of ocm::thread)
class condition {
public:
    condition() = default;

    // condition can't be copied or moved.
    condition(const condition&) = delete;
    condition(condition&&) = delete;

    void signal();
    void broadcast();
    void wait();

private:
    std::queue<thread*> waitingThreads_;
};


// Simple buffered channel class (only uses public API of ocm::thread)
template <typename T>
class channel {
public:
    channel() = default;

    // Channels can't be copied or moved.
    channel(const channel&) = delete;
    channel(channel&&) = delete;

    void send(T value);
    T receive();

private:
    std::queue<T> buffer_;
    std::queue<thread*> waitingReaders_;
};

// Simple buffered channel class (only uses public API of ocm::thread)
template <typename T>
class bounded_channel {
public:
    bounded_channel(size_t size = 0);

    // Channels can't be copied or moved.
    bounded_channel(const bounded_channel&) = delete;
    bounded_channel(bounded_channel&&) = delete;

    void send(T value);
    T receive();

private:
    channel<T> channel_;
    semaphore limit_;
};


namespace internals {

#if NO_HLE
    extern std::mutex exclusivity;
#else
    extern unsigned int cache_padding_1[64];
    extern unsigned int exclusivity_spinlock;
    extern unsigned int cache_padding_2[64];
#endif    
inline void begin_exclusive();
inline void end_exclusive();

template <class T>
static inline typename std::decay<T>::type decay_copy(T&& v);

}

}

#include "simple-ocm-private.hpp"