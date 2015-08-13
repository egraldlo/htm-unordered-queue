// inline (but private) functions

#include <utility>
#include <immintrin.h>

namespace ocm {

#if defined(__ATOMIC_HLE_ACQUIRE) && defined(__ATOMIC_HLE_RELEASE)
    #define OCM_HLE_ACQUIRE __ATOMIC_HLE_ACQUIRE
    #define OCM_HLE_RELEASE __ATOMIC_HLE_RELEASE
#else
    #define OCM_HLE_ACQUIRE 0
    #define OCM_HLE_RELEASE 0
#endif

void internals::begin_exclusive()
{
#if NO_HLE
    exclusivity.lock();
#elif USE_RTM
    int retries = 0;
    for (;;) {
        while (__atomic_load_n(&exclusivity_spinlock, __ATOMIC_CONSUME) == 1)
        _mm_pause();
        uint32_t status = _xbegin(); 
    if (status == 0xffffffffU) {
        if (__atomic_load_n(&exclusivity_spinlock, __ATOMIC_CONSUME) == 1)
        _xabort(42);
        break;
    }
    ++retries;
    if (retries > 128 || (status & 0x3) == 0) {
        // Acquire lock
        while (__atomic_exchange_n(&exclusivity_spinlock, 1,
                       __ATOMIC_ACQUIRE) != 0) { 
        // Wait for lock to become free again before retrying.
        do { 
            _mm_pause(); 
        } while (__atomic_load_n(&exclusivity_spinlock, __ATOMIC_CONSUME) == 1);
        }
        break;
    }
    }
#else
    // Acquire lock
    while (__atomic_exchange_n(&exclusivity_spinlock, 1,
                    __ATOMIC_ACQUIRE | OCM_HLE_ACQUIRE) != 0) { 
        // Wait for lock to become free again before retrying.
        do { 
            _mm_pause(); 
        } while (__atomic_load_n(&exclusivity_spinlock, __ATOMIC_CONSUME) == 1); 
    }
#endif
}

void internals::end_exclusive()
{
#if NO_HLE
    exclusivity.unlock();
#elif USE_RTM
    if (_xtest()) {
    _xend();
    } else {
    __atomic_store_n(&exclusivity_spinlock, 0, __ATOMIC_RELEASE);
    }
#else
    __atomic_store_n(&exclusivity_spinlock, 0,
                     __ATOMIC_RELEASE | OCM_HLE_RELEASE);
#endif
}

// template functions

template <class T>
typename std::decay<T>::type internals::decay_copy(T&& v)
{
    return std::forward<T>(v);
}

template<typename Fun, typename... Args>
thread::thread(Fun f, Args&&... args)
    : state_{STARTING},
      thread_([this,f](typename std::decay<Args>::type... params) {
         self_ = this;
     susp_mutex_.lock();
         internals::begin_exclusive();
         state_ = RUNNING;
         f(std::forward<typename std::decay<Args>::type>(params)...);
         state_ = DONE;
         internals::end_exclusive();
     susp_mutex_.unlock();
      }, internals::decay_copy(std::forward<Args>(args))...)
{
    // Nothing else to do...
}

template <typename T>
void channel<T>::send(T value)
{
    buffer_.push(std::move(value));
    if (!waitingReaders_.empty()) {
         thread* reader = waitingReaders_.front();
         waitingReaders_.pop();
         reader->resume();
    }
}

template <typename T>
T channel<T>::receive()
{
    if (buffer_.empty()) {
        thread* currentThread = thread::current();
        do {
            waitingReaders_.push(currentThread);
            currentThread->suspend();
        } while (buffer_.empty());
    }
    T result = std::move(buffer_.front());
    buffer_.pop();
    return result;
}

template <typename T>
bounded_channel<T>::bounded_channel(size_t size)
    : channel_{}, limit_{size}
{
    // Nothing (else) to do
}

template <typename T>
void bounded_channel<T>::send(T value)
{
    limit_.wait();
    channel_.send(std::move(value));
}

template <typename T>
T bounded_channel<T>::receive()
{
    limit_.post();
    return channel_.receive();
}



}