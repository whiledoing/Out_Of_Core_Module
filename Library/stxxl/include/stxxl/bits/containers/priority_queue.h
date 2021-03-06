/***************************************************************************
 *  include/stxxl/bits/containers/priority_queue.h
 *
 *  Part of the STXXL. See http://stxxl.sourceforge.net
 *
 *  Copyright (C) 1999 Peter Sanders <sanders@mpi-sb.mpg.de>
 *  Copyright (C) 2003, 2004, 2007 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *  Copyright (C) 2007-2009 Johannes Singler <singler@ira.uka.de>
 *  Copyright (C) 2007-2010 Andreas Beckmann <beckmann@cs.uni-frankfurt.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef STXXL_PRIORITY_QUEUE_HEADER
#define STXXL_PRIORITY_QUEUE_HEADER

#include <vector>

#include <stxxl/bits/deprecated.h>
#include <stxxl/bits/mng/typed_block.h>
#include <stxxl/bits/mng/block_alloc.h>
#include <stxxl/bits/mng/read_write_pool.h>
#include <stxxl/bits/mng/prefetch_pool.h>
#include <stxxl/bits/mng/write_pool.h>
#include <stxxl/bits/common/tmeta.h>
#include <stxxl/bits/algo/sort_base.h>
#include <stxxl/bits/parallel.h>
#include <stxxl/bits/common/is_sorted.h>

#if STXXL_PARALLEL

#if defined(STXXL_PARALLEL_MODE) && ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) < 40400)
#undef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_INTERNAL
#undef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_EXTERNAL
#undef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_INTERNAL 0
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_EXTERNAL 0
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER 0
#endif

// enable/disable parallel merging for certain cases, for performance tuning
#ifndef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_INTERNAL
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_INTERNAL 1
#endif
#ifndef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_EXTERNAL
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_EXTERNAL 1
#endif
#ifndef STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER
#define STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER 1
#endif

#endif //STXXL_PARALLEL

#if STXXL_PARALLEL && STXXL_PARALLEL_PQ_MULTIWAY_MERGE_EXTERNAL
#define STXXL_PQ_EXTERNAL_LOSER_TREE 0 // no loser tree for the external sequences
#else
#define STXXL_PQ_EXTERNAL_LOSER_TREE 1
#endif

#if STXXL_PARALLEL && STXXL_PARALLEL_PQ_MULTIWAY_MERGE_INTERNAL
#define STXXL_PQ_INTERNAL_LOSER_TREE 0 // no loser tree for the internal sequences
#else
#define STXXL_PQ_INTERNAL_LOSER_TREE 1
#endif

#include <stxxl/bits/containers/pq_helpers.h>
#include <stxxl/bits/containers/pq_mergers.h>
#include <stxxl/bits/containers/pq_ext_merger.h>
#include <stxxl/bits/containers/pq_losertree.h>

__STXXL_BEGIN_NAMESPACE

/*
   KNBufferSize1 = 32;
   KNN = 512; // length of group 1 sequences
   KNKMAX = 64;  // maximal arity
   LogKNKMAX = 6;  // ceil(log KNKMAX)
   KNLevels = 4; // overall capacity >= KNN*KNKMAX^KNLevels
 */

// internal memory consumption >= N_*(KMAX_^IntLevels_) + ext

template <
    class Tp_,
    class Cmp_,
    unsigned BufferSize1_ = 32,                    // equalize procedure call overheads etc.
    unsigned N_ = 512,                             // length of group 1 sequences
    unsigned IntKMAX_ = 64,                        // maximal arity for internal mergers
    unsigned IntLevels_ = 4,                       // number of internal groups
    unsigned BlockSize_ = (2 * 1024 * 1024),       // external block size
    unsigned ExtKMAX_ = 64,                        // maximal arity for external mergers
    unsigned ExtLevels_ = 2,                       // number of external groups
    class AllocStr_ = STXXL_DEFAULT_ALLOC_STRATEGY
    >
struct priority_queue_config
{
    typedef Tp_ value_type;
    typedef Cmp_ comparator_type;
    typedef AllocStr_ alloc_strategy_type;
    enum
    {
        delete_buffer_size = BufferSize1_,
        N = N_,
        IntKMAX = IntKMAX_,
        num_int_groups = IntLevels_,
        num_ext_groups = ExtLevels_,
        BlockSize = BlockSize_,
        ExtKMAX = ExtKMAX_,
        element_size = sizeof(Tp_)
    };
};

__STXXL_END_NAMESPACE

namespace std
{
    template <class BlockType_,
              class Cmp_,
              unsigned Arity_,
              class AllocStr_>
    void swap(stxxl::priority_queue_local::ext_merger<BlockType_, Cmp_, Arity_, AllocStr_> & a,
              stxxl::priority_queue_local::ext_merger<BlockType_, Cmp_, Arity_, AllocStr_> & b)
    {
        a.swap(b);
    }
    template <class ValTp_, class Cmp_, unsigned KNKMAX>
    void swap(stxxl::priority_queue_local::loser_tree<ValTp_, Cmp_, KNKMAX> & a,
              stxxl::priority_queue_local::loser_tree<ValTp_, Cmp_, KNKMAX> & b)
    {
        a.swap(b);
    }
}

__STXXL_BEGIN_NAMESPACE

//! \brief External priority queue data structure
template <class Config_>
class priority_queue : private noncopyable
{
public:
    typedef Config_ Config;
    enum
    {
        delete_buffer_size = Config::delete_buffer_size,
        N = Config::N,
        IntKMAX = Config::IntKMAX,
        num_int_groups = Config::num_int_groups,
        num_ext_groups = Config::num_ext_groups,
        total_num_groups = Config::num_int_groups + Config::num_ext_groups,
        BlockSize = Config::BlockSize,
        ExtKMAX = Config::ExtKMAX
    };

    //! \brief The type of object stored in the \b priority_queue
    typedef typename Config::value_type value_type;
    //! \brief Comparison object
    typedef typename Config::comparator_type comparator_type;
    typedef typename Config::alloc_strategy_type alloc_strategy_type;
    //! \brief An unsigned integral type (64 bit)
    typedef stxxl::uint64 size_type;
    typedef typed_block<BlockSize, value_type> block_type;
    typedef read_write_pool<block_type> pool_type;

protected:
    typedef priority_queue_local::internal_priority_queue<value_type, std::vector<value_type>, comparator_type>
    insert_heap_type;

    typedef priority_queue_local::loser_tree<
        value_type,
        comparator_type,
        IntKMAX> int_merger_type;

    typedef priority_queue_local::ext_merger<
        block_type,
        comparator_type,
        ExtKMAX,
        alloc_strategy_type> ext_merger_type;


    int_merger_type int_mergers[num_int_groups];
    pool_type * pool;
    bool pool_owned;
    ext_merger_type * ext_mergers;

    // one delete buffer for each tree => group buffer
    value_type group_buffers[total_num_groups][N + 1];          // tree->group_buffers->delete_buffer (extra space for sentinel)
    value_type * group_buffer_current_mins[total_num_groups];   // group_buffer_current_mins[i] is current start of group_buffers[i], end is group_buffers[i] + N

    // overall delete buffer
    value_type delete_buffer[delete_buffer_size + 1];
    value_type * delete_buffer_current_min;                     // current start of delete_buffer
    value_type * delete_buffer_end;                             // end of delete_buffer

    comparator_type cmp;

    // insert buffer
    insert_heap_type insert_heap;

    // how many groups are active
    unsigned_type num_active_groups;

    // total size not counting insert_heap and delete_buffer
    size_type size_;

private:
    void init();

    void refill_delete_buffer();
    unsigned_type refill_group_buffer(unsigned_type k);

    unsigned_type make_space_available(unsigned_type level);
    void empty_insert_heap();

    value_type get_supremum() const { return cmp.min_value(); } //{ return group_buffers[0][KNN].key; }
    unsigned_type current_delete_buffer_size() const { return delete_buffer_end - delete_buffer_current_min; }
    unsigned_type current_group_buffer_size(unsigned_type i) const { return &(group_buffers[i][N]) - group_buffer_current_mins[i]; }

public:
    //! \brief Constructs external priority queue object
    //! \param pool_ pool of blocks that will be used
    //! for data writing and prefetching for the disk<->memory transfers
    //! happening in the priority queue. Larger pool size
    //! helps to speed up operations.
    priority_queue(pool_type & pool_);

    //! \brief Constructs external priority queue object
    //! \param p_pool_ pool of blocks that will be used
    //! for data prefetching for the disk<->memory transfers
    //! happening in the priority queue. Larger pool size
    //! helps to speed up operations.
    //! \param w_pool_ pool of blocks that will be used
    //! for writing data for the memory<->disk transfers
    //! happening in the priority queue. Larger pool size
    //! helps to speed up operations.
    _STXXL_DEPRECATED(priority_queue(prefetch_pool<block_type> & p_pool_, write_pool<block_type> & w_pool_));

    //! \brief Constructs external priority queue object
    //! \param p_pool_mem memory (in bytes) for prefetch pool that will be used
    //! for data prefetching for the disk<->memory transfers
    //! happening in the priority queue. Larger pool size
    //! helps to speed up operations.
    //! \param w_pool_mem memory (in bytes) for buffered write pool that will be used
    //! for writing data for the memory<->disk transfers
    //! happening in the priority queue. Larger pool size
    //! helps to speed up operations.
    priority_queue(unsigned_type p_pool_mem, unsigned_type w_pool_mem);

    //! \brief swap this priority queue with another one
    //! Implementation correctness is questionable.
    void swap(priority_queue & obj)
    {
        //swap_1D_arrays(int_mergers,obj.int_mergers,num_int_groups); // does not work in g++ 3.4.3 :( bug?
        for (unsigned_type i = 0; i < num_int_groups; ++i)
            std::swap(int_mergers[i], obj.int_mergers[i]);

        //std::swap(pool,obj.pool);
        //std::swap(pool_owned, obj.pool_owned);
        std::swap(ext_mergers, obj.ext_mergers);
        for (unsigned_type i1 = 0; i1 < total_num_groups; ++i1)
            for (unsigned_type i2 = 0; i2 < (N + 1); ++i2)
                std::swap(group_buffers[i1][i2], obj.group_buffers[i1][i2]);

        swap_1D_arrays(group_buffer_current_mins, obj.group_buffer_current_mins, total_num_groups);
        swap_1D_arrays(delete_buffer, obj.delete_buffer, delete_buffer_size + 1);
        std::swap(delete_buffer_current_min, obj.delete_buffer_current_min);
        std::swap(delete_buffer_end, obj.delete_buffer_end);
        std::swap(cmp, obj.cmp);
        std::swap(insert_heap, obj.insert_heap);
        std::swap(num_active_groups, obj.num_active_groups);
        std::swap(size_, obj.size_);
    }

    virtual ~priority_queue();

    //! \brief Returns number of elements contained
    //! \return number of elements contained
    size_type size() const;

    //! \brief Returns true if queue has no elements
    //! \return \b true if queue has no elements, \b false otherwise
    bool empty() const { return (size() == 0); }

    //! \brief Returns "largest" element
    //!
    //! Returns a const reference to the element at the
    //! top of the priority_queue. The element at the top is
    //! guaranteed to be the largest element in the \b priority queue,
    //! as determined by the comparison function \b Config_::comparator_type
    //! (the same as the second parameter of PRIORITY_QUEUE_GENERATOR utility
    //! class). That is,
    //! for every other element \b x in the priority_queue,
    //! \b Config_::comparator_type(Q.top(), x) is false.
    //! Precondition: \c empty() is false.
    const value_type & top() const;

    //! \brief Removes the element at the top
    //!
    //! Removes the element at the top of the priority_queue, that
    //! is, the largest element in the \b priority_queue.
    //! Precondition: \c empty() is \b false.
    //! Postcondition: \c size() will be decremented by 1.
    void pop();

    //! \brief Inserts x into the priority_queue.
    //!
    //! Inserts x into the priority_queue. Postcondition:
    //! \c size() will be incremented by 1.
    void push(const value_type & obj);

    //! \brief Returns number of bytes consumed by
    //! the \b priority_queue
    //! \brief number of bytes consumed by the \b priority_queue from
    //! the internal memory not including pools (see the constructor)
    unsigned_type mem_cons() const
    {
        unsigned_type dynam_alloc_mem = 0;
        //dynam_alloc_mem += w_pool.mem_cons();
        //dynam_alloc_mem += p_pool.mem_cons();
        for (int i = 0; i < num_int_groups; ++i)
            dynam_alloc_mem += int_mergers[i].mem_cons();

        for (int i = 0; i < num_ext_groups; ++i)
            dynam_alloc_mem += ext_mergers[i].mem_cons();


        return (sizeof(*this) +
                sizeof(ext_merger_type) * num_ext_groups +
                dynam_alloc_mem);
    }
};


template <class Config_>
inline typename priority_queue<Config_>::size_type priority_queue<Config_>::size() const
{
    return size_ +
           insert_heap.size() - 1 +
           (delete_buffer_end - delete_buffer_current_min);
}


template <class Config_>
inline const typename priority_queue<Config_>::value_type & priority_queue<Config_>::top() const
{
    assert(!insert_heap.empty());

    const typename priority_queue<Config_>::value_type & t = insert_heap.top();
    if (/*(!insert_heap.empty()) && */ cmp(*delete_buffer_current_min, t))
        return t;
    else
        return *delete_buffer_current_min;
}

template <class Config_>
inline void priority_queue<Config_>::pop()
{
    //STXXL_VERBOSE1("priority_queue::pop()");
    assert(!insert_heap.empty());

    if (/*(!insert_heap.empty()) && */ cmp(*delete_buffer_current_min, insert_heap.top()))
        insert_heap.pop();
    else
    {
        assert(delete_buffer_current_min < delete_buffer_end);
        ++delete_buffer_current_min;
        if (delete_buffer_current_min == delete_buffer_end)
            refill_delete_buffer();
    }
}

template <class Config_>
inline void priority_queue<Config_>::push(const value_type & obj)
{
    //STXXL_VERBOSE3("priority_queue::push("<< obj <<")");
    assert(int_mergers->not_sentinel(obj));
    if (insert_heap.size() == N + 1)
        empty_insert_heap();


    assert(!insert_heap.empty());

    insert_heap.push(obj);
}


////////////////////////////////////////////////////////////////

template <class Config_>
priority_queue<Config_>::priority_queue(pool_type & pool_) :
    pool(&pool_),
    pool_owned(false),
    delete_buffer_end(delete_buffer + delete_buffer_size),
    insert_heap(N + 2),
    num_active_groups(0), size_(0)
{
    STXXL_VERBOSE2("priority_queue::priority_queue(pool)");
    init();
}

// DEPRECATED
template <class Config_>
priority_queue<Config_>::priority_queue(prefetch_pool<block_type> & p_pool_, write_pool<block_type> & w_pool_) :
    pool(new pool_type(p_pool_, w_pool_)),
    pool_owned(true),
    delete_buffer_end(delete_buffer + delete_buffer_size),
    insert_heap(N + 2),
    num_active_groups(0), size_(0)
{
    STXXL_VERBOSE2("priority_queue::priority_queue(p_pool, w_pool)");
    init();
}

template <class Config_>
priority_queue<Config_>::priority_queue(unsigned_type p_pool_mem, unsigned_type w_pool_mem) :
    pool(new pool_type(p_pool_mem / BlockSize, w_pool_mem / BlockSize)),
    pool_owned(true),
    delete_buffer_end(delete_buffer + delete_buffer_size),
    insert_heap(N + 2),
    num_active_groups(0), size_(0)
{
    STXXL_VERBOSE2("priority_queue::priority_queue(pool sizes)");
    init();
}

template <class Config_>
void priority_queue<Config_>::init()
{
    assert(!cmp(cmp.min_value(), cmp.min_value())); // verify strict weak ordering

    ext_mergers = new ext_merger_type[num_ext_groups];
    for (unsigned_type j = 0; j < num_ext_groups; ++j)
        ext_mergers[j].set_pool(pool);

    value_type sentinel = cmp.min_value();
    insert_heap.push(sentinel);                                // always keep the sentinel
    delete_buffer[delete_buffer_size] = sentinel;              // sentinel
    delete_buffer_current_min = delete_buffer_end;             // empty
    for (unsigned_type i = 0; i < total_num_groups; i++)
    {
        group_buffers[i][N] = sentinel;                        // sentinel
        group_buffer_current_mins[i] = &(group_buffers[i][N]); // empty
    }
}

template <class Config_>
priority_queue<Config_>::~priority_queue()
{
    STXXL_VERBOSE2("priority_queue::~priority_queue()");
    if (pool_owned)
        delete pool;

    delete[] ext_mergers;
}

//--------------------- Buffer refilling -------------------------------

// refill group_buffers[j] and return number of elements found
template <class Config_>
unsigned_type priority_queue<Config_>::refill_group_buffer(unsigned_type group)
{
    STXXL_VERBOSE2("priority_queue::refill_group_buffer(" << group << ")");

    value_type * target;
    unsigned_type length;
    size_type group_size = (group < num_int_groups) ?
                           int_mergers[group].size() :
                           ext_mergers[group - num_int_groups].size();                         //elements left in segments
    unsigned_type left_elements = group_buffers[group] + N - group_buffer_current_mins[group]; //elements left in target buffer
    if (group_size + left_elements >= size_type(N))
    {                                                                                          // buffer will be filled completely
        target = group_buffers[group];
        length = N - left_elements;
    }
    else
    {
        target = group_buffers[group] + N - group_size - left_elements;
        length = group_size;
    }

    if (length > 0)
    {
        // shift remaininig elements to front
        memmove(target, group_buffer_current_mins[group], left_elements * sizeof(value_type));
        group_buffer_current_mins[group] = target;

        // fill remaining space from group
        if (group < num_int_groups)
            int_mergers[group].multi_merge(target + left_elements, length);
        else
            ext_mergers[group - num_int_groups].multi_merge(
                target + left_elements,
                target + left_elements + length);
    }

    //STXXL_MSG(length + left_elements);
    //std::copy(target,target + length + left_elements,std::ostream_iterator<value_type>(std::cout, "\n"));
#if STXXL_CHECK_ORDER_IN_SORTS
    priority_queue_local::invert_order<typename Config::comparator_type, value_type, value_type> inv_cmp(cmp);
    if (!stxxl::is_sorted(group_buffer_current_mins[group], group_buffers[group] + N, inv_cmp))
    {
        STXXL_VERBOSE2("length: " << length << " left_elements: " << left_elements);
        for (value_type * v = group_buffer_current_mins[group] + 1; v < group_buffer_current_mins[group] + left_elements; ++v)
        {
            if (inv_cmp(*v, *(v - 1)))
            {
                STXXL_MSG("Error in buffer " << group << " at position " << (v - group_buffer_current_mins[group] - 1) << "/" << (v - group_buffer_current_mins[group]) << "   " << *(v - 2) << " " << *(v - 1) << " " << *v << " " << *(v + 1));
            }
        }
        assert(false);
    }
#endif

    return length + left_elements;
}


template <class Config_>
void priority_queue<Config_>::refill_delete_buffer()
{
    STXXL_VERBOSE2("priority_queue::refill_delete_buffer()");

    size_type total_group_size = 0;
    //num_active_groups is <= 4
    for (int i = num_active_groups - 1; i >= 0; i--)
    {
        if ((group_buffers[i] + N) - group_buffer_current_mins[i] < delete_buffer_size)
        {
            unsigned_type length = refill_group_buffer(i);
            // max active level dry now?
            if (length == 0 && unsigned(i) == num_active_groups - 1)
                --num_active_groups;

            total_group_size += length;
        }
        else
            total_group_size += delete_buffer_size;  // actually only a sufficient lower bound
    }

    unsigned_type length;
    if (total_group_size >= delete_buffer_size)      // buffer can be filled completely
    {
        length = delete_buffer_size;                 // amount to be copied
        size_ -= size_type(delete_buffer_size);      // amount left in group_buffers
    }
    else
    {
        length = total_group_size;
        assert(size_ == size_type(length)); // trees and group_buffers get empty
        size_ = 0;
    }

    priority_queue_local::invert_order<typename Config::comparator_type, value_type, value_type> inv_cmp(cmp);

    // now call simplified refill routines
    // which can make the assumption that
    // they find all they are asked in the buffers
    delete_buffer_current_min = delete_buffer_end - length;
    STXXL_VERBOSE2("Active groups = " << num_active_groups);
    switch (num_active_groups)
    {
    case 0:
        break;
    case 1:
        std::copy(group_buffer_current_mins[0], group_buffer_current_mins[0] + length, delete_buffer_current_min);
        group_buffer_current_mins[0] += length;
        break;
    case 2:
#if STXXL_PARALLEL && STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER
        {
            std::pair<value_type *, value_type *> seqs[2] =
            {
                std::make_pair(group_buffer_current_mins[0], group_buffers[0] + N),
                std::make_pair(group_buffer_current_mins[1], group_buffers[1] + N)
            };
            parallel::multiway_merge_sentinel(seqs, seqs + 2, delete_buffer_current_min, inv_cmp, length); //sequence iterators are progressed appropriately

            group_buffer_current_mins[0] = seqs[0].first;
            group_buffer_current_mins[1] = seqs[1].first;
        }
#else
        priority_queue_local::merge_iterator(
            group_buffer_current_mins[0],
            group_buffer_current_mins[1], delete_buffer_current_min, length, cmp);
#endif
        break;
    case 3:
#if STXXL_PARALLEL && STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER
        {
            std::pair<value_type *, value_type *> seqs[3] =
            {
                std::make_pair(group_buffer_current_mins[0], group_buffers[0] + N),
                std::make_pair(group_buffer_current_mins[1], group_buffers[1] + N),
                std::make_pair(group_buffer_current_mins[2], group_buffers[2] + N)
            };
            parallel::multiway_merge_sentinel(seqs, seqs + 3, delete_buffer_current_min, inv_cmp, length); //sequence iterators are progressed appropriately

            group_buffer_current_mins[0] = seqs[0].first;
            group_buffer_current_mins[1] = seqs[1].first;
            group_buffer_current_mins[2] = seqs[2].first;
        }
#else
        priority_queue_local::merge3_iterator(
            group_buffer_current_mins[0],
            group_buffer_current_mins[1],
            group_buffer_current_mins[2], delete_buffer_current_min, length, cmp);
#endif
        break;
    case 4:
#if STXXL_PARALLEL && STXXL_PARALLEL_PQ_MULTIWAY_MERGE_DELETE_BUFFER
        {
            std::pair<value_type *, value_type *> seqs[4] =
            {
                std::make_pair(group_buffer_current_mins[0], group_buffers[0] + N),
                std::make_pair(group_buffer_current_mins[1], group_buffers[1] + N),
                std::make_pair(group_buffer_current_mins[2], group_buffers[2] + N),
                std::make_pair(group_buffer_current_mins[3], group_buffers[3] + N)
            };
            parallel::multiway_merge_sentinel(seqs, seqs + 4, delete_buffer_current_min, inv_cmp, length); //sequence iterators are progressed appropriately

            group_buffer_current_mins[0] = seqs[0].first;
            group_buffer_current_mins[1] = seqs[1].first;
            group_buffer_current_mins[2] = seqs[2].first;
            group_buffer_current_mins[3] = seqs[3].first;
        }
#else
        priority_queue_local::merge4_iterator(
            group_buffer_current_mins[0],
            group_buffer_current_mins[1],
            group_buffer_current_mins[2],
            group_buffer_current_mins[3], delete_buffer_current_min, length, cmp); //side effect free
#endif
        break;
    default:
        STXXL_THROW(std::runtime_error, "priority_queue<...>::refill_delete_buffer()",
                    "Overflow! The number of buffers on 2nd level in stxxl::priority_queue is currently limited to 4");
    }

#if STXXL_CHECK_ORDER_IN_SORTS
    if (!stxxl::is_sorted(delete_buffer_current_min, delete_buffer_end, inv_cmp))
    {
        for (value_type * v = delete_buffer_current_min + 1; v < delete_buffer_end; ++v)
        {
            if (inv_cmp(*v, *(v - 1)))
            {
                STXXL_MSG("Error at position " << (v - delete_buffer_current_min - 1) << "/" << (v - delete_buffer_current_min) << "   " << *(v - 1) << " " << *v);
            }
        }
        assert(false);
    }
#endif
    //std::copy(delete_buffer_current_min,delete_buffer_current_min + length,std::ostream_iterator<value_type>(std::cout, "\n"));
}

//--------------------------------------------------------------------

// check if space is available on level k and
// empty this level if necessary leading to a recursive call.
// return the level where space was finally available
template <class Config_>
unsigned_type priority_queue<Config_>::make_space_available(unsigned_type level)
{
    STXXL_VERBOSE2("priority_queue::make_space_available(" << level << ")");
    unsigned_type finalLevel;
    assert(level < total_num_groups);
    assert(level <= num_active_groups);

    if (level == num_active_groups)
        ++num_active_groups;

    const bool spaceIsAvailable_ =
        (level < num_int_groups) ? int_mergers[level].is_space_available()
        : ((level == total_num_groups - 1) ? true : (ext_mergers[level - num_int_groups].is_space_available()));

    if (spaceIsAvailable_)
    {
        finalLevel = level;
    }
    else
    {
        finalLevel = make_space_available(level + 1);

        if (level < num_int_groups - 1)                                  // from internal to internal tree
        {
            unsigned_type segmentSize = int_mergers[level].size();
            value_type * newSegment = new value_type[segmentSize + 1];
            int_mergers[level].multi_merge(newSegment, segmentSize);     // empty this level

            newSegment[segmentSize] = delete_buffer[delete_buffer_size]; // sentinel
            // for queues where size << #inserts
            // it might make sense to stay in this level if
            // segmentSize < alpha * KNN * k^level for some alpha < 1
            int_mergers[level + 1].insert_segment(newSegment, segmentSize);
        }
        else
        {
            if (level == num_int_groups - 1) // from internal to external tree
            {
                const unsigned_type segmentSize = int_mergers[num_int_groups - 1].size();
                STXXL_VERBOSE1("Inserting segment into first level external: " << level << " " << segmentSize);
                ext_mergers[0].insert_segment(int_mergers[num_int_groups - 1], segmentSize);
            }
            else // from external to external tree
            {
                const size_type segmentSize = ext_mergers[level - num_int_groups].size();
                STXXL_VERBOSE1("Inserting segment into second level external: " << level << " " << segmentSize);
                ext_mergers[level - num_int_groups + 1].insert_segment(ext_mergers[level - num_int_groups], segmentSize);
            }
        }
    }
    return finalLevel;
}


// empty the insert heap into the main data structure
template <class Config_>
void priority_queue<Config_>::empty_insert_heap()
{
    STXXL_VERBOSE2("priority_queue::empty_insert_heap()");
    assert(insert_heap.size() == (N + 1));

    const value_type sup = get_supremum();

    // build new segment
    value_type * newSegment = new value_type[N + 1];
    value_type * newPos = newSegment;

    // put the new data there for now
    //insert_heap.sortTo(newSegment);
    value_type * SortTo = newSegment;

    insert_heap.sort_to(SortTo);

    SortTo = newSegment + N;
    insert_heap.clear();
    insert_heap.push(*SortTo);

    assert(insert_heap.size() == 1);

    newSegment[N] = sup; // sentinel

    // copy the delete_buffer and group_buffers[0] to temporary storage
    // (the temporary can be eliminated using some dirty tricks)
    const unsigned_type tempSize = N + delete_buffer_size;
    value_type temp[tempSize + 1];
    unsigned_type sz1 = current_delete_buffer_size();
    unsigned_type sz2 = current_group_buffer_size(0);
    value_type * pos = temp + tempSize - sz1 - sz2;
    std::copy(delete_buffer_current_min, delete_buffer_current_min + sz1, pos);
    std::copy(group_buffer_current_mins[0], group_buffer_current_mins[0] + sz2, pos + sz1);
    temp[tempSize] = sup; // sentinel

    // refill delete_buffer
    // (using more complicated code it could be made somewhat fuller
    // in certain circumstances)
    priority_queue_local::merge_iterator(pos, newPos, delete_buffer_current_min, sz1, cmp);

    // refill group_buffers[0]
    // (as above we might want to take the opportunity
    // to make group_buffers[0] fuller)
    priority_queue_local::merge_iterator(pos, newPos, group_buffer_current_mins[0], sz2, cmp);

    // merge the rest to the new segment
    // note that merge exactly trips into the footsteps
    // of itself
    priority_queue_local::merge_iterator(pos, newPos, newSegment, N, cmp);

    // and insert it
    unsigned_type freeLevel = make_space_available(0);
    assert(freeLevel == 0 || int_mergers[0].size() == 0);
    int_mergers[0].insert_segment(newSegment, N);

    // get rid of invalid level 2 buffers
    // by inserting them into tree 0 (which is almost empty in this case)
    if (freeLevel > 0)
    {
        for (int_type i = freeLevel; i >= 0; i--)
        {
            // reverse order not needed
            // but would allow immediate refill

            newSegment = new value_type[current_group_buffer_size(i) + 1]; // with sentinel
            std::copy(group_buffer_current_mins[i], group_buffer_current_mins[i] + current_group_buffer_size(i) + 1, newSegment);
            int_mergers[0].insert_segment(newSegment, current_group_buffer_size(i));
            group_buffer_current_mins[i] = group_buffers[i] + N;           // empty
        }
    }

    // update size
    size_ += size_type(N);

    // special case if the tree was empty before
    if (delete_buffer_current_min == delete_buffer_end)
        refill_delete_buffer();
}

namespace priority_queue_local
{
    struct Parameters_for_priority_queue_not_found_Increase_IntM
    {
        enum { fits = false };
        typedef Parameters_for_priority_queue_not_found_Increase_IntM result;
    };

    struct dummy
    {
        enum { fits = false };
        typedef dummy result;
    };

    template <unsigned_type E_, unsigned_type IntM_, unsigned_type MaxS_, unsigned_type B_, unsigned_type m_, bool stop = false>
    struct find_B_m
    {
        typedef find_B_m<E_, IntM_, MaxS_, B_, m_, stop> Self;
        enum {
            k = IntM_ / B_,    // number of blocks that fit into M
            element_size = E_, // element size
            IntM = IntM_,
            B = B_,            // block size
            m = m_,            // number of blocks fitting into buffers
            c = k - m_,
            // memory occ. by block must be at least 10 times larger than size of ext sequence
            // && satisfy memory req && if we have two ext mergers their degree must be at least 64=m/2
            fits = c > 10 && ((k - m) * (m) * (m * B / (element_size * 4 * 1024))) >= MaxS_
                   && ((MaxS_ < ((k - m) * m / (2 * element_size)) * 1024) || m >= 128),
            step = 1
        };

        typedef typename find_B_m<element_size, IntM, MaxS_, B, m + step, fits || (m >= k - step)>::result candidate1;
        typedef typename find_B_m<element_size, IntM, MaxS_, B / 2, 1, fits || candidate1::fits>::result candidate2;
        typedef typename IF<fits, Self, typename IF<candidate1::fits, candidate1, candidate2>::result>::result result;
    };

    // specialization for the case when no valid parameters are found
    template <unsigned_type E_, unsigned_type IntM_, unsigned_type MaxS_, bool stop>
    struct find_B_m<E_, IntM_, MaxS_, 2048, 1, stop>
    {
        enum { fits = false };
        typedef Parameters_for_priority_queue_not_found_Increase_IntM result;
    };

    // to speedup search
    template <unsigned_type E_, unsigned_type IntM_, unsigned_type MaxS_, unsigned_type B_, unsigned_type m_>
    struct find_B_m<E_, IntM_, MaxS_, B_, m_, true>
    {
        enum { fits = false };
        typedef dummy result;
    };

    // E_ size of element in bytes
    template <unsigned_type E_, unsigned_type IntM_, unsigned_type MaxS_>
    struct find_settings
    {
        // start from block size (8*1024*1024) bytes
        typedef typename find_B_m<E_, IntM_, MaxS_, (8 * 1024 * 1024), 1>::result result;
    };

    struct Parameters_not_found_Try_to_change_the_Tune_parameter
    {
        typedef Parameters_not_found_Try_to_change_the_Tune_parameter result;
    };


    template <unsigned_type AI_, unsigned_type X_, unsigned_type CriticalSize_>
    struct compute_N
    {
        typedef compute_N<AI_, X_, CriticalSize_> Self;
        enum
        {
            X = X_,
            AI = AI_,
            N = X / (AI * AI) // two stage internal
        };
        typedef typename IF<(N >= CriticalSize_), Self, typename compute_N<AI / 2, X, CriticalSize_>::result>::result result;
    };

    template <unsigned_type X_, unsigned_type CriticalSize_>
    struct compute_N<1, X_, CriticalSize_>
    {
        typedef Parameters_not_found_Try_to_change_the_Tune_parameter result;
    };
}

//! \}

//! \addtogroup stlcont
//! \{

//! \brief Priority queue type generator

//! Implements a data structure from "Peter Sanders. Fast Priority Queues
//! for Cached Memory. ALENEX'99" for external memory.
//! <BR>
//! \tparam type of the contained objects (POD with no references to internal memory)
//! \tparam the comparison type used to determine
//! whether one element is smaller than another element.
//! If Cmp_(x,y) is true, then x is smaller than y. The element
//! returned by Q.top() is the largest element in the priority
//! queue. That is, it has the property that, for every other
//! element \b x in the priority queue, Cmp_(Q.top(), x) is false.
//! Cmp_ must also provide min_value method, that returns value of type Tp_ that is
//! smaller than any element of the queue \b x , i.e. Cmp_(Cmp_.min_value(),x) is
//! always \b true . <BR>
//! <BR>
//! Example: comparison object for priority queue
//! where \b top() returns the \b smallest contained integer:
//! \verbatim
//! struct CmpIntGreater
//! {
//!   bool operator () (const int & a, const int & b) const { return a>b; }
//!   int min_value() const  { return std::numeric_limits<int>::max(); }
//! };
//! \endverbatim
//! Example: comparison object for priority queue
//! where \b top() returns the \b largest contained integer:
//! \verbatim
//! struct CmpIntLess
//! {
//!   bool operator () (const int & a, const int & b) const { return a<b; }
//!   int min_value() const  { return std::numeric_limits<int>::min(); }
//! };
//! \endverbatim
//! Note that Cmp_ must define strict weak ordering.
//! (<A HREF="http://www.sgi.com/tech/stl/StrictWeakOrdering.html">see what it is</A>)
//! - \c IntM_ upper limit for internal memory consumption in bytes.
//! - \c MaxS_ upper limit for number of elements contained in the priority queue (in 1024 units).
//! Example: if you are sure that priority queue contains no more than
//! one million elements in a time, then the right parameter is (1000000/1024)= 976 .
//! - \c Tune_ tuning parameter. Try to play with it if the code does not compile
//! (larger than default values might help). Code does not compile
//! if no suitable internal parameters were found for given IntM_ and MaxS_.
//! It might also happen that given IntM_ is too small for given MaxS_, try larger values.
//! <BR>
//! \c PRIORITY_QUEUE_GENERATOR is template meta program that searches
//! for \b 7 configuration parameters of \b stxxl::priority_queue that both
//! minimize internal memory consumption of the priority queue to
//! match IntM_ and maximize performance of priority queue operations.
//! Actual memory consumption might be larger (use
//! \c stxxl::priority_queue::mem_cons() method to track it), since the search
//! assumes rather optimistic schedule of push'es and pop'es for the
//! estimation of the maximum memory consumption. To keep actual memory
//! requirements low increase the value of MaxS_ parameter.
//! <BR>
//! For functioning a priority queue object requires two pools of blocks
//! (See constructor of \c priority_queue ). To construct \c \<stxxl\> block
//! pools you might need \b block \b type that will be used by priority queue.
//! Note that block's size and hence it's type is generated by
//! the \c PRIORITY_QUEUE_GENERATOR in compile type from IntM_, MaxS_ and sizeof(Tp_) and
//! not given directly by user as a template parameter. Block type can be extracted as
//! \c PRIORITY_QUEUE_GENERATOR<some_parameters>::result::block_type .
//! For an example see p_queue.cpp .
//! Configured priority queue type is available as \c PRIORITY_QUEUE_GENERATOR<>::result. <BR> <BR>
template <class Tp_, class Cmp_, unsigned_type IntM_, unsigned MaxS_, unsigned Tune_ = 6>
class PRIORITY_QUEUE_GENERATOR
{
public:
    // actual calculation of B, m, k and element_size
    typedef typename priority_queue_local::find_settings<sizeof(Tp_), IntM_, MaxS_>::result settings;
    enum {
        B = settings::B,
        m = settings::m,
        X = B * (settings::k - m) / settings::element_size,  // interpretation of result
        Buffer1Size = 32                                     // fixed
    };
    // derivation of N, AI, AE
    typedef typename priority_queue_local::compute_N<(1 << Tune_), X, 4 * Buffer1Size>::result ComputeN;
    enum
    {
        N = ComputeN::N,
        AI = ComputeN::AI,
        AE = (m / 2 < 2) ? 2 : (m / 2)            // at least 2
    };
    enum {
        // Estimation of maximum internal memory consumption (in bytes)
        EConsumption = X * settings::element_size + settings::B * AE + ((MaxS_ / X) / AE) * settings::B * 1024
    };
    /*
        unsigned BufferSize1_ = 32, // equalize procedure call overheads etc.
        unsigned N_ = 512,          // bandwidth
        unsigned IntKMAX_ = 64,     // maximal arity for internal mergers
        unsigned IntLevels_ = 4,
        unsigned BlockSize_ = (2*1024*1024),
        unsigned ExtKMAX_ = 64,     // maximal arity for external mergers
        unsigned ExtLevels_ = 2,
     */
    typedef priority_queue<priority_queue_config<Tp_, Cmp_, Buffer1Size, N, AI, 2, B, AE, 2> > result;
};

//! \}

__STXXL_END_NAMESPACE


namespace std
{
    template <class Config_>
    void swap(stxxl::priority_queue<Config_> & a,
              stxxl::priority_queue<Config_> & b)
    {
        a.swap(b);
    }
}

#endif // !STXXL_PRIORITY_QUEUE_HEADER
// vim: et:ts=4:sw=4
