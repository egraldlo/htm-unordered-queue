
/**
 * \file unorderedset-array-of-queue-private.hpp
 *
 * \author Tony Zhang
 *
 * \brief Provides implementatin for the implementation with an array of queues
 */


// Includes required for your templated code go here

using namespace std;

// Templated code for member functions goes here

template<typename Key>
htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::UnorderedSetArrayOfQueueDynamic()
    : table_{new QueueContainer[DEFAULT_SIZE]}, 
      size_{DEFAULT_SIZE}
{
    // Nothing to do here
}

template<typename Key>
htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::UnorderedSetArrayOfQueueDynamic(const size_t n)
    : table_{new QueueContainer[n]},
      size_{n}
{
    // Nothing to do here
}

template<typename Key>
htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::~UnorderedSetArrayOfQueueDynamic()
{
    delete [] table_;
}

template<typename Key>
void htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::insert(const Key& k, const size_t threadId)
{    
    table_[threadId - 1].queue.enq(k);
}

template<typename Key>
Key htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::remove(const size_t threadId)
{
    size_t index = threadId - 1;

    if (!table_[index].queue.empty())
        return table_[index].queue.deq();

    const size_t RETRY_TIMES = size_ / 2;
    size_t tmpIndex = (threadId < size_) ? threadId : 0; // Get the subsequent queue index
    size_t i = 0;

    while (tmpIndex != index && i < RETRY_TIMES) {
        for (size_t j = 0; j < 1/*size_t(20000 / size_)*/; ++j) {
            if (!table_[tmpIndex].queue.empty())
                table_[index].queue.enq(table_[tmpIndex].queue.lazyDeq());
            else
               break;
        }

        if (++tmpIndex >= size_)
            tmpIndex = 0;
        ++i;
    }

    return table_[index].queue.deq();
}

template<typename Key>
bool htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::empty() const
{
    for (size_t i = 0; i < size_; ++i) {
        if (!table_[i].queue.empty())
            return false;
    }
    return true;
}

// Queue

template<typename Key>
htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::TMSafeQueue()
    : head_{NULL},
      tail_{NULL},
      firstExisted_{NULL}
{
    // Nothing to do here
}

template<typename Key>
htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::~TMSafeQueue()
{
    // Deallocate dynamically allocated the queue.
    ListNode* ptr = head_;
    while (ptr) {
        ListNode* tmp = ptr;
        ptr = ptr->next_;
        delete tmp;
    }

    head_ = nullptr;
    tail_ = nullptr;
}

template<typename Key>
void htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::enq(const Key& k)
{
    if (!head_) {
        tail_ = new ListNode(k);
        head_ = tail_;
        firstExisted_ = tail_;
    } else {
        ListNode* p = new ListNode(k);
        tail_->next_ = p;
        tail_ = p;
    }    
}

template<typename Key>
Key htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::deq()
{
    Key result;

    while (head_ && head_->deleted_) {
        ListNode* tmp = head_;
        ListNode* p = head_->next_;
        head_ = p;
        delete tmp;
    }
    firstExisted_ = head_;

    if (head_) {
        ListNode* ptr = head_;
        result = head_->val_;

        ListNode* p = head_->next_;
        head_ = p;
        
        delete ptr;
    } else {
        tail_ = nullptr; // Avoid dangling pointer
    }

    return result;
}

template<typename Key>
bool htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::empty()
{
    return firstExisted_ == nullptr;
}

template<typename Key>
Key htm_ds::UnorderedSetArrayOfQueueDynamic<Key>::TMSafeQueue::lazyDeq()
{        
    if (firstExisted_) {
        firstExisted_->deleted_ = true;
        Key result = firstExisted_->val_;

        ListNode* p = firstExisted_->next_;
        firstExisted_ = p;

        return result;
    } else {
        return Key();
    }
}
