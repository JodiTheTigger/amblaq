// adapted from http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#include <atomic-queues/mpmc.h>

#include <stdatomic.h>
#include <string.h>

#if defined(__STDC_NO_ATOMICS__)
    #error("Oh no, your C compiler doesn't support C11 atomics :-(")
#endif

#define QUEUE2_CACHELINE_BYTES 64

// -----------------------------------------------------------------------------

typedef struct Queue2_Cell
{
    atomic_size_t    sequence;
    QUEUE2_MPMC_TYPE data;
}
Queue2_Cell;

typedef struct Queue2_Mpmc
{
    uint8_t        pad0[QUEUE2_CACHELINE_BYTES];

    atomic_size_t  enqueue_index;
    uint8_t        pad2[QUEUE2_CACHELINE_BYTES - sizeof(atomic_size_t)];

    atomic_size_t  dequeue_index;
    uint8_t        pad3[QUEUE2_CACHELINE_BYTES - sizeof(atomic_size_t)];

    size_t         cell_mask;
    uint8_t        pad4[QUEUE2_CACHELINE_BYTES - sizeof(size_t)];

    Queue2_Cell    cells[];
}
Queue2_Mpmc;

// -----------------------------------------------------------------------------

size_t mpmc_make_queue(size_t cell_count, Queue2_Mpmc* queue)
{
    if (cell_count < 2)
    {
        return -1;
    }

    if (cell_count & (cell_count - 1))
    {
        // must be pow2.
        return -1;
    }

    size_t bytes = sizeof(Queue2_Mpmc) +  (sizeof(Queue2_Cell) * cell_count);

    if (!queue)
    {
        return bytes;
    }

    memset(queue, 0, bytes);    

    queue->cell_mask = cell_count - 1;

    for (size_t i = 0; i < cell_count; i++)
    {
        atomic_store_explicit
        (
              &queue->cells[i].sequence
            , i
            , memory_order_relaxed
        );
    }

    atomic_store_explicit(&queue->enqueue_index, 0, memory_order_relaxed);
    atomic_store_explicit(&queue->dequeue_index, 0, memory_order_relaxed);
}

Queue2_Result mpmc_try_enqueue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE const* data)
{
    size_t pos =
        atomic_load_explicit(&queue->enqueue_index, memory_order_relaxed);

    Queue2_Cell* cell = &queue->cells[pos & queue->cell_mask];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t) pos;

    if (!difference)
    {
        if
        (
            atomic_compare_exchange_weak_explicit
            (
                  &queue->enqueue_index
                , &pos
                , pos + 1
                , memory_order_relaxed
                , memory_order_relaxed
            )
        )
        {
            cell->data = *data;

            atomic_store_explicit
            (
                  &cell->sequence
                , pos + 1
                , memory_order_release
            );

            return Queue2_Result_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue2_Result_Full;
    }

    return Queue2_Result_Contention;
}

Queue2_Result mpmc_try_dequeue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE* data)
{
    size_t pos =
        atomic_load_explicit(&queue->dequeue_index, memory_order_relaxed);

    Queue2_Cell* cell = &queue->cells[pos & queue->cell_mask];

    size_t sequence =
        atomic_load_explicit(&cell->sequence, memory_order_acquire);

    intptr_t difference = (intptr_t) sequence - (intptr_t)(pos + 1);

    if (!difference)
    {
        if
        (
            atomic_compare_exchange_weak_explicit
            (
                  &queue->dequeue_index
                , &pos
                , pos + 1
                , memory_order_relaxed
                , memory_order_relaxed
            )
        )
        {
            *data = cell->data;

            atomic_store_explicit
            (
                  &cell->sequence
                , pos + queue->cell_mask + 1
                , memory_order_release
            );

            return Queue2_Result_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue2_Result_Empty;
    }

    return Queue2_Result_Contention;
}

Queue2_Result mpmc_enqueue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE const* data)
{
    Queue2_Result result;

    do
    {
        result = mpmc_try_enqueue(queue, data);
    }
    while (result == Queue2_Result_Contention);

    return result;
}

Queue2_Result mpmc_dequeue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE* data)
{
    Queue2_Result result;

    do
    {
        result = mpmc_try_dequeue(queue, data);
    }
    while (result == Queue2_Result_Contention);

    return result;
}


