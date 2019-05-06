#include <stdint.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------

typedef struct Data
{
    float    a;
    uint32_t b;
    uint8_t  bytes[16];
}
Data;

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <queues/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   0
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <queues/queues.h>

#define QUEUE_MP   0
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <queues/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <queues/queues.h>

#define CAST(x, y) ((x) y)

// -----------------------------------------------------------------------------

typedef enum Tag
{
      Spsc
    , Mpsc
    , Spmc
    , Mpmc
}
Tag;

Queue_Result make(Tag tag, size_t cell_count, void* queue, size_t* bytes)
{
    switch (tag)
    {
        case Spsc:
        {
            return spsc_make_queue_Data
            (
                  cell_count
                , CAST(Queue_Spsc_Data*, queue)
                , bytes
            );
        }
        case Mpsc:
        {
            return mpsc_make_queue_Data
            (
                  cell_count
                , CAST(Queue_Mpsc_Data*, queue)
                , bytes
            );
        }
        case Spmc:
        {
            return spmc_make_queue_Data
            (
                  cell_count
                , CAST(Queue_Spmc_Data*, queue)
                , bytes
            );
        }
        case Mpmc:
        {
            return mpmc_make_queue_Data
            (
                  cell_count
                , CAST(Queue_Mpmc_Data*, queue)
                , bytes
            );
        }
    }

    return Queue_Result_Error;
}

Queue_Result try_enqueue(Tag tag, void* q, Data const* d)
{
    switch (tag)
    {
        case Spsc: return spsc_try_enqueue_Data(CAST(Queue_Spsc_Data*, q), d);
        case Mpsc: return mpsc_try_enqueue_Data(CAST(Queue_Mpsc_Data*, q), d);
        case Spmc: return spmc_try_enqueue_Data(CAST(Queue_Spmc_Data*, q), d);
        case Mpmc: return mpmc_try_enqueue_Data(CAST(Queue_Mpmc_Data*, q), d);
    }

    return Queue_Result_Error;
}
Queue_Result try_dequeue(Tag tag, void* q, Data* d)
{
    switch (tag)
    {
        case Spsc: return spsc_try_dequeue_Data(CAST(Queue_Spsc_Data*, q), d);
        case Mpsc: return mpsc_try_dequeue_Data(CAST(Queue_Mpsc_Data*, q), d);
        case Spmc: return spmc_try_dequeue_Data(CAST(Queue_Spmc_Data*, q), d);
        case Mpmc: return mpmc_try_dequeue_Data(CAST(Queue_Mpmc_Data*, q), d);
    }

    return Queue_Result_Error;
}
Queue_Result enqueue(Tag tag, void* q, Data const* d)
{
    switch (tag)
    {
        case Spsc: return spsc_enqueue_Data(CAST(Queue_Spsc_Data*, q), d);
        case Mpsc: return mpsc_enqueue_Data(CAST(Queue_Mpsc_Data*, q), d);
        case Spmc: return spmc_enqueue_Data(CAST(Queue_Spmc_Data*, q), d);
        case Mpmc: return mpmc_enqueue_Data(CAST(Queue_Mpmc_Data*, q), d);
    }

    return Queue_Result_Error;
}
Queue_Result dequeue(Tag tag, void* q, Data* d)
{
    switch (tag)
    {
        case Spsc: return spsc_dequeue_Data(CAST(Queue_Spsc_Data*, q), d);
        case Mpsc: return mpsc_dequeue_Data(CAST(Queue_Mpsc_Data*, q), d);
        case Spmc: return spmc_dequeue_Data(CAST(Queue_Spmc_Data*, q), d);
        case Mpmc: return mpmc_dequeue_Data(CAST(Queue_Mpmc_Data*, q), d);
    }

    return Queue_Result_Error;
}

// -----------------------------------------------------------------------------

#define EXPECT(x) do {if(!(x)) { return #x; }} while(0)

const char* null_pointers(Tag tag)
{
    Queue_Result result = make(tag, 0, NULL, NULL);

    EXPECT(result == Queue_Result_Error_Null_Bytes);

    return NULL;
}

const char* create(Tag tag)
{
    size_t bytes = -1;

    EXPECT(make(tag, 0, NULL, &bytes) == Queue_Result_Error_Too_Small);
    EXPECT(make(tag, 1, NULL, &bytes) == Queue_Result_Error_Too_Small);
    // min size

    EXPECT(make(tag, 13,  NULL, &bytes) == Queue_Result_Error_Not_Pow2);
    EXPECT(make(tag, 255, NULL, &bytes) == Queue_Result_Error_Not_Pow2);
    // must be pow2

    EXPECT(make(tag, -1, NULL, &bytes) == Queue_Result_Error_Too_Big);

    EXPECT
    (
           make(tag, -3000000, NULL, &bytes)
        == Queue_Result_Error_Too_Big
    );

    EXPECT
    (
           make(tag, 1ULL << 63, NULL, &bytes)
        == Queue_Result_Error_Too_Big
    );

    EXPECT
    (
           make(tag, 1ULL << 33, NULL, &bytes)
        == Queue_Result_Error_Too_Big
    );
    // Insane sizes

    {
        make(tag, 1 << 8, NULL, &bytes);

        EXPECT(bytes > 0);
        EXPECT(bytes < 100000);

        void* q = malloc(bytes);

        Queue_Result create = make(tag, 1 << 8, q, &bytes);

        free(q);

        EXPECT(create == Queue_Result_Ok);
    }

    return NULL;
}
