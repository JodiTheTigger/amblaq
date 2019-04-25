// adapted from http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#if !defined(QUEUE_TYPE) || !defined(QUEUE_MP) || !defined(QUEUE_MC)
    #error("Please define QUEUE_TYPE, QUEUE_MP and QUEUE_MC")
#endif

// -----------------------------------------------------------------------------


#if !defined(__cplusplus)

    #include <stdatomic.h>

    #if defined(__STDC_NO_ATOMICS__)
        #error("Oh no, your C compiler doesn't support C11 atomics :-(")
    #endif

    #define QUEUE_ATOMIC_SIZE_T atomic_size_t
    #define QUEUE_ORDER_RELAXED memory_order_relaxed
    #define QUEUE_ORDER_RELEASE memory_order_release
    #define QUEUE_ORDER_ACQUIRE memory_order_acquire
    #define QUEUE_ATOMIC_STORE  atomic_store_explicit
    #define QUEUE_ATOMIC_LOAD   atomic_load_explicit

#else

    #include <atomic>

    #define QUEUE_ATOMIC_SIZE_T std::atomic_size_t
    #define QUEUE_ORDER_RELAXED std::memory_order_relaxed
    #define QUEUE_ORDER_RELEASE std::memory_order_release
    #define QUEUE_ORDER_ACQUIRE std::memory_order_acquire
    #define QUEUE_ATOMIC_STORE  std::atomic_store_explicit<size_t>
    #define QUEUE_ATOMIC_LOAD   std::atomic_load_explicit

#endif

// -----------------------------------------------------------------------------

#if (QUEUE_MP)
    #define QUEUE_P_NAME_FN        mp
    #define QUEUE_P_NAME_TYPE      Mp
    #define QUEUE_P_TYPE           QUEUE_ATOMIC_SIZE_T
    #define QUEUE_P_SETUP(a, b, c) QUEUE_ATOMIC_STORE(&a, b, c)
    #define QUEUE_P_LOAD(a, b)     QUEUE_ATOMIC_LOAD (&a, b)

    #define QUEUE_P_IF_CAS(a, b, c, d, e)                                      \
        if                                                                     \
        (                                                                      \
            atomic_compare_exchange_weak_explicit                              \
            (                                                                  \
                  &a                                                           \
                , &b                                                           \
                , c                                                            \
                , d                                                            \
                , e                                                            \
            )                                                                  \
        )
#else
    #define QUEUE_P_NAME                  sp
    #define QUEUE_P_TYPE                  size_t
    #define QUEUE_P_SETUP(a, b, c)
    #define QUEUE_P_LOAD(a, b)            a
    #define QUEUE_P_IF_CAS(a, b, c, d, e) a = c;
#endif

#if (QUEUE_MC)
    #define QUEUE_C_NAME           mc
    #define QUEUE_C_TYPE           QUEUE_ATOMIC_SIZE_T
    #define QUEUE_C_SETUP(a, b, c) QUEUE_ATOMIC_STORE(&a, b, c)
    #define QUEUE_C_LOAD(a, b)     QUEUE_ATOMIC_LOAD (&a, b)

    #define QUEUE_C_IF_CAS(a, b, c, d, e)                                      \
        if                                                                     \
        (                                                                      \
            atomic_compare_exchange_weak_explicit                              \
            (                                                                  \
                  &a                                                           \
                , &b                                                           \
                , c                                                            \
                , d                                                            \
                , e                                                            \
            )                                                                  \
        )
#else
    #define QUEUE_C_NAME                  sc
    #define QUEUE_C_TYPE                  size_t
    #define QUEUE_C_SETUP(a, b, c)
    #define QUEUE_C_LOAD(a, b)            a
    #define QUEUE_C_IF_CAS(a, b, c, d, e) a = c;
#endif

#define QUEUE_MACRO_MERGE_BASE(a, b) a ## b
#define QUEUE_MACRO_MERGE(a, b) QUEUE_MACRO_MERGE_BASE(a, b)

#define QUEUE_FN_A QUEUE_MACRO_MERGE(QUEUE_P_NAME_FN, QUEUE_C_NAME)
#define QUEUE_FN(name) QUEUE_MACRO_MERGE(QUEUE_MACRO_MERGE(QUEUE_FN_A, _),     \
                                         QUEUE_MACRO_MERGE(name##_, QUEUE_TYPE))

#define QUEUE_STRUCT_A QUEUE_MACRO_MERGE(QUEUE_P_NAME_TYPE, QUEUE_C_NAME)
#define QUEUE_STRUCT   QUEUE_MACRO_MERGE(Queue_, QUEUE_STRUCT_A)

#define QUEUE_CACHELINE_BYTES 64
#define QUEUE_TOO_BIG         (1024ULL * 1024ULL * 256ULL)

// -----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Cell
{
    QUEUE_ATOMIC_SIZE_T sequence;
    QUEUE_TYPE          data;
}
Cell;

typedef struct QUEUE_STRUCT
{
    uint8_t        pad0[QUEUE_CACHELINE_BYTES];

    QUEUE_P_TYPE   enqueue_index;
    uint8_t        pad2[QUEUE_CACHELINE_BYTES - sizeof(QUEUE_P_TYPE)];

    QUEUE_C_TYPE   dequeue_index;
    uint8_t        pad3[QUEUE_CACHELINE_BYTES - sizeof(QUEUE_C_TYPE)];

    size_t         cell_mask;
    uint8_t        pad4[QUEUE_CACHELINE_BYTES - sizeof(size_t)];

    Cell           cells[];
}
QUEUE_STRUCT;

typedef enum Queue_Result
{
      Queue_Result_Ok
    , Queue_Result_Full
    , Queue_Result_Empty
    , Queue_Result_Contention

    , Queue_Result_Error = 128
    , Queue_Result_Error_Too_Small
    , Queue_Result_Error_Too_Big
    , Queue_Result_Error_Not_Pow2
    , Queue_Result_Error_Not_Aligned_16_Bytes
    , Queue_Result_Error_Null_Bytes
    , Queue_Result_Error_Bytes_Smaller_Than_Needed
}
Queue_Result;


Queue_Result QUEUE_FN(make_queue)
(
      size_t        cell_count
    , QUEUE_STRUCT* queue
    , size_t**      bytes
)
{
    if (cell_count < 2)
    {
        return Queue_Result_Error_Too_Small;
    }

    if (cell_count > QUEUE_TOO_BIG)
    {
        return Queue_Result_Error_Too_Big;
    }

    if (cell_count & (cell_count - 1))
    {
        return Queue_Result_Error_Not_Pow2;
    }

    if (!bytes)
    {
        return Queue_Result_Error_Null_Bytes;
    }

    if (!(*bytes))
    {
        return Queue_Result_Error_Null_Bytes;
    }

    size_t bytes_local = sizeof(QUEUE_STRUCT) +  (sizeof(Cell) * cell_count);

    if (!queue)
    {
        **bytes = bytes_local;
        return Queue_Result_Ok;
    }

    if (**bytes < bytes_local)
    {
        return Queue_Result_Error_Bytes_Smaller_Than_Needed;
    }

    {
        intptr_t queue_value = (intptr_t) queue;

        if (queue_value & 0x0F)
        {
            return Queue_Result_Error_Not_Aligned_16_Bytes;
        }
    }

    memset(queue, 0, bytes_local);

    queue->cell_mask = cell_count - 1;

    for (size_t i = 0; i < cell_count; i++)
    {
        QUEUE_ATOMIC_STORE
        (
              &queue->cells[i].sequence
            , i
            , QUEUE_ORDER_RELAXED
        );
    }

    QUEUE_P_SETUP(queue->enqueue_index, 0, QUEUE_ORDER_RELAXED);
    QUEUE_C_SETUP(queue->dequeue_index, 0, QUEUE_ORDER_RELAXED);

    return Queue_Result_Ok;
}

Queue_Result QUEUE_FN(try_enqueue)(QUEUE_STRUCT* queue, QUEUE_TYPE const* data)
{
    size_t pos =    
        QUEUE_P_LOAD(queue->enqueue_index, QUEUE_ORDER_RELAXED);

    Cell* cell = &queue->cells[pos & queue->cell_mask];

    size_t sequence =
        QUEUE_ATOMIC_LOAD(&cell->sequence, QUEUE_ORDER_ACQUIRE);

    intptr_t difference = (intptr_t) sequence - (intptr_t) pos;

    if (!difference)
    {
        QUEUE_P_IF_CAS
        (
              queue->enqueue_index
            , pos
            , pos + 1
            , QUEUE_ORDER_RELAXED
            , QUEUE_ORDER_RELAXED
        )
        {
            cell->data = *data;

            QUEUE_ATOMIC_STORE
            (
                  &cell->sequence
                , pos + 1
                , QUEUE_ORDER_RELEASE
            );

            return Queue_Result_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue_Result_Full;
    }

    return Queue_Result_Contention;
}

Queue_Result QUEUE_FN(try_dequeue)(QUEUE_STRUCT* queue, QUEUE_TYPE* data)
{
    size_t pos =
        QUEUE_C_LOAD(queue->dequeue_index, QUEUE_ORDER_RELAXED);

    Cell* cell = &queue->cells[pos & queue->cell_mask];

    size_t sequence =
        QUEUE_ATOMIC_LOAD(&cell->sequence, QUEUE_ORDER_ACQUIRE);

    intptr_t difference = (intptr_t) sequence - (intptr_t)(pos + 1);

    if (!difference)
    {
        QUEUE_C_IF_CAS
        (
              queue->dequeue_index
            , pos
            , pos + 1
            , QUEUE_ORDER_RELAXED
            , QUEUE_ORDER_RELAXED
        )
        {
            *data = cell->data;

            QUEUE_ATOMIC_STORE
            (
                  &cell->sequence
                , pos + queue->cell_mask + 1
                , QUEUE_ORDER_RELEASE
            );

            return Queue_Result_Ok;
        }
    }

    if (difference < 0)
    {
        return Queue_Result_Empty;
    }

    return Queue_Result_Contention;
}

Queue_Result QUEUE_FN(enqueue)(QUEUE_STRUCT* queue, QUEUE_TYPE const* data)
{
    Queue_Result result;

    do
    {
        result = QUEUE_FN(try_enqueue)(queue, data);
    }
    while (result == Queue_Result_Contention);

    return result;
}

Queue_Result QUEUE_FN(dequeue)(QUEUE_STRUCT* queue, QUEUE_TYPE* data)
{
    Queue_Result result;

    do
    {
        result = QUEUE_FN(try_dequeue)(queue, data);
    }
    while (result == Queue_Result_Contention);

    return result;
}

#ifdef __cplusplus
}
#endif

#undef QUEUE_TYPE
#undef QUEUE_MP
#undef QUEUE_MC
