/**
 * \file unorderedset-array-of-queue.hpp
 *
 * \author Tony Zhang
 *
 * \brief Provides unorderedset-array-of-queue<Key>
 */

#ifndef UNORDEREDSETARRAYOFQUEUEDYNAMIC_HPP_INCLUDED
#define UNORDEREDSETARRAYOFQUEUEDYNAMIC_HPP_INCLUDED 1


// Any header files that are needed to typecheck the class
// declaration should be #included here.
#include "unorderedset.hpp"
#include <cstddef>

namespace htm_ds 
{
     // Templated interfaces (e.g., the HashSet class declarations) go here
    template <typename Key>
    class UnorderedSetArrayOfQueueDynamic : public htm_ds::UnorderedSet<Key> {
    public:

        UnorderedSetArrayOfQueueDynamic();   //< Default Constructor
        UnorderedSetArrayOfQueueDynamic(const size_t n);

        ~UnorderedSetArrayOfQueueDynamic();  //< Destructor
        UnorderedSetArrayOfQueueDynamic(const UnorderedSetArrayOfQueueDynamic<Key>& rhs) = default;

        /*
         * Modifiers:
         */
        // void insert(const Key& k);  ///< Add an element

        void insert(const Key& k, const size_t threadId);
        
        //Key remove();              ///< Remove one element

        Key remove(const size_t threadId);    ///< Remove an element given the particular pattern

        bool empty() const;

    private:
        class TMSafeQueue {
        public:
            TMSafeQueue();
            ~TMSafeQueue();
            TMSafeQueue(const TMSafeQueue & rhs) = default;

            // Queue operations
            void enq(const Key& k);
            Key deq();
            bool empty();

            static const size_t MAX_ELEM = 2500000;

        private:
            TMSafeQueue& operator=(const TMSafeQueue& rhs);

            // A simple node implementation
            struct ListNode
            {
                Key val_;
                ListNode* next_;
                bool deleted_;
                ListNode() : val_(), next_(NULL) {}
                ListNode(Key val) : val_(val), next_(NULL) {}
            };

            ListNode* head_;
            ListNode* tail_;
            ListNode* firstExisted_;

            Key lazyDeq(); 

            friend class UnorderedSetArrayOfQueueDynamic;
        };

        // A memory space container for a 
        struct QueueContainer {
            TMSafeQueue queue;
            // Dummy space for preventing false sharing
            unsigned char placeHolder[64];
        };

        // Disable assignment operator
        UnorderedSetArrayOfQueueDynamic& operator=(const UnorderedSetArrayOfQueueDynamic& rhs);

        // The default table of Contents      
        static const size_t DEFAULT_SIZE = 96;

        QueueContainer * table_;
        size_t size_;
    };
}

#include "unorderedset-array-of-queue-dynamic-private.hpp"

#endif
