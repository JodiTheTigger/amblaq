#pragma once

#include <stddef.h>
#include <stdint.h>

// -----------------------------------------------------------------------------

#if !defined(QUEUE2_MPMC_TYPE)
    #define QUEUE2_MPMC_TYPE uint64_t
#endif

#if defined( __cplusplus)
extern "C" {
#endif

// -----------------------------------------------------------------------------

typedef enum Queue2_Result
{
      Queue2_Result_Ok
    , Queue2_Result_Full
    , Queue2_Result_Empty
    , Queue2_Result_Contention
    , Queue2_Result_Error
}
Queue2_Result;

typedef struct Queue2_Mpmc Queue2_Mpmc;

// -----------------------------------------------------------------------------

size_t mpmc_make_queue(size_t cell_count, Queue2_Mpmc* queue);
Queue2_Result mpmc_try_enqueue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE const* data);
Queue2_Result mpmc_try_dequeue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE* data);
Queue2_Result mpmc_enqueue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE const* data);
Queue2_Result mpmc_dequeue(Queue2_Mpmc* queue, QUEUE2_MPMC_TYPE* data);

// -----------------------------------------------------------------------------

#if defined(__cplusplus)
}
#endif
