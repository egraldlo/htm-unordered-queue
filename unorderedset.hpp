/**
 * \file unorderedset.hpp
 *
 * \author Tony Zhang
 *
 * \brief An interface for unordered set, providing all the essential 
 *        operations for an unordered set
 */

#ifndef UNORDEREDSET_HPP_INCLUDED
#define UNORDEREDSET_HPP_INCLUDED 1


// Any header files that are needed to typecheck the class
// declaration should be #included here.
#include <cstddef>

namespace htm_ds 
{
     // Templated interfaces (e.g., the UnorderedSet class declarations) go here
    template <typename Key>
    class UnorderedSet {
    public:

        virtual ~UnorderedSet() = default;      ///< Virtual destructor

        /*
         * Modifiers:
         */
        virtual void insert(const Key& k, const size_t id) = 0;  ///< Add an element
                
        virtual Key remove(const size_t id) = 0;        ///< Remove one element

        virtual bool empty() const = 0;
    };
}

#endif
