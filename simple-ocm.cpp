#include <cstddef>
#include <cstdlib>
#include <cassert>
#include <chrono>
#include <cstdio>

#include "simple-ocm.hpp"

using namespace std;

int main(int argc, char **argv)
{
    int result = 0;
    ocm::internals::begin_exclusive();
    ocm::thread main_thread([&]{result = ocm_main(argc, argv);});
    main_thread.join();
    ocm::internals::end_exclusive();
    return result;
}

namespace ocm {

#if NO_HLE
    std::mutex internals::exclusivity;
#else
    unsigned int internals::cache_padding_1[64];
    unsigned int internals::exclusivity_spinlock;
    unsigned int internals::cache_padding_2[64];
#endif

// On OS X, C++11 thread_local support isn't in Xcode's version of clang.
#if defined(__apple_build_version__)
    __thread thread* thread::self_;
#else
    thread_local thread* thread::self_;
#endif

void thread::join()
{
    internals::end_exclusive();
    thread_.join();
    internals::begin_exclusive();
}

void thread::yield()
{
    internals::end_exclusive();
    // std::this_thread::yield();
    internals::begin_exclusive();
}

void thread::sleep(double seconds)
{
    internals::end_exclusive();
    std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
    internals::begin_exclusive();
}

void thread::suspend()
{
    // associate with mutex *without* locking it (it's already locked)
    std::unique_lock<std::mutex> lock(susp_mutex_, std::adopt_lock);
    state_ = SUSPENDED;
    internals::end_exclusive();
    susp_cv_.wait(lock, [this]{
        return state_ == WOKEN;
    });
    internals::begin_exclusive();
    state_ = RUNNING;
    // disassociate from mutex *without* unlocking it (leave it locked)
    lock.release();
}

void thread::resume()
{
    std::unique_lock<std::mutex> lock(susp_mutex_);
    state_ = WOKEN;
    susp_cv_.notify_one();
}


barrier::barrier(size_t count)
{
    waitingThreads_.reserve(count);
}

void barrier::arrive()
{
    --arrivalsNeeded_;
    if (arrivalsNeeded_ == 0) {
        release();
        thread::yield();
    } else {
        thread* currentThread = thread::current();
        waitingThreads_.push_back(currentThread);
        currentThread->suspend();
    }
}

void barrier::release()
{
    arrivalsNeeded_ = 0;
    for (auto tp : waitingThreads_)
        tp->resume();
    waitingThreads_.clear();
}

ssize_t barrier::adjust(ssize_t count)
{
    arrivalsNeeded_ += count;
    if (arrivalsNeeded_ == 0)
        release();
    return arrivalsNeeded_;
}

void barrier::reset(size_t count)
{
    release();
    arrivalsNeeded_ = count;
    waitingThreads_.reserve(count);
}

// Semaphore implementation

semaphore::semaphore(size_t count)
    : count_(count)
{
    // Nothing (else) to do
}

void semaphore::wait()
{
    if (count_ > 0) {
        --count_;
        return;
    }
    thread* currentThread = thread::current();
    waitingThreads_.push(currentThread);
    currentThread->suspend();
    wait();
}

void semaphore::post()
{
    ++count_;
    if (!waitingThreads_.empty()) {
         thread* waiter = waitingThreads_.front();
         waitingThreads_.pop();
         waiter->resume();
    }
}

void semaphore::reset(size_t count)
{
    assert(waitingThreads_.empty());
    count_ = count;
}

// Condition implementation

void condition::wait()
{
    thread* currentThread = thread::current();
    waitingThreads_.push(currentThread);
    currentThread->suspend();
}

void condition::signal()
{
    if (!waitingThreads_.empty()) {
         thread* waiter = waitingThreads_.front();
         waitingThreads_.pop();
         waiter->resume();
    }
}

void condition::broadcast()
{
    while (!waitingThreads_.empty()) {
         thread* waiter = waitingThreads_.front();
         waitingThreads_.pop();
         waiter->resume();
    }
}

}