#pragma once
#ifndef FG_INC_QUEUE
#define FG_INC_QUEUE
#include <queue>

/**
 * Wrapper around the std::queue container with additional functions for
 * getting the constant iterators.
 */
template <typename TValueType, typename TContainer = ::std::deque<TValueType>>
class Queue : public ::std::queue<TValueType, TContainer>
{
public:
    typedef Queue self_type;
    typedef ::std::queue<TValueType, TContainer> base_type;

public:
    using iterator = typename TContainer::iterator;
    using const_iterator = typename TContainer::const_iterator;
    using reverse_iterator = typename TContainer::reverse_iterator;
    using const_reverse_iterator = typename TContainer::const_reverse_iterator;

public:
    const_iterator begin(void) const { return this->c.begin(); }
    const_iterator end(void) const { return this->c.end(); }
    iterator begin(void) { return this->c.begin(); }
    iterator end(void) { return this->c.end(); }

    const_reverse_iterator rbegin(void) const { return this->c.rbegin(); }
    const_reverse_iterator rend(void) const { return this->c.rend(); }
    reverse_iterator rbegin(void) { return this->c.rbegin(); }
    reverse_iterator rend(void) { return this->c.rend(); }

    void clear(void)
    {
        while (!this->empty())
            this->pop();
    }
};

/**
 * A wrapper class around the std::priority_queue class with additional
 * functions for getting begin/end iterators. With these iterators it is
 * possible to traverse the queue without removing the top element every time.
 *
 */
template <typename TValueType, typename TSequence = ::std::vector<TValueType>,
          typename TCompare = ::std::less<typename TSequence::value_type>>
class PriorityQueue : public ::std::priority_queue<TValueType, TSequence, TCompare>
{
public:
    using self_type = PriorityQueue;
    using base_type = ::std::priority_queue<TValueType, TSequence, TCompare>;

public:
    typedef typename TSequence::iterator iterator;
    typedef typename TSequence::const_iterator const_iterator;
    typedef typename TSequence::reverse_iterator reverse_iterator;
    typedef typename TSequence::const_reverse_iterator const_reverse_iterator;

public:
    iterator begin(void) { return this->c.begin(); }
    iterator end(void) { return this->c.end(); }
    const_iterator begin(void) const { return this->c.begin(); }
    const_iterator end(void) const { return this->c.end(); }
    const_reverse_iterator rbegin(void) const { return this->c.rbegin(); }
    const_reverse_iterator rend(void) const { return this->c.rend(); }

    void clear(void)
    {
        while (!this->empty())
            this->pop();
    }
};
#endif //> FG_INC_QUEUE