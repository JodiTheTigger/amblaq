// -----------------------------------------------------------------------------
// Example code for readme.
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdlib.h> // malloc

#define MY_STRUCT_DATA_COUNT 8

typedef struct My_Struct
{
    uint64_t tag;
    uint8_t  data[MY_STRUCT_DATA_COUNT];
}
My_Struct;

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE My_Struct
#define QUEUE_IMPLEMENTATION
#include <amblaq/queues.h>

int example()
{
    // create the queue.
    Queue_Spsc_My_Struct* queue;
    size_t bytes = 0;

    if (spsc_make_queue_My_Struct(256, NULL, &bytes) != Queue_Result_Ok)
    {
        return 1;
    }

    queue = (Queue_Spsc_My_Struct*) malloc(bytes);

    if (spsc_make_queue_My_Struct(256, queue, &bytes) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // Queue
    My_Struct reference =
    {
          22
        , {1,2,3,4,5,6,7,8}
    };

    if (spsc_enqueue_My_Struct(queue, &reference) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // de-queue
    My_Struct result;

    if (spsc_dequeue_My_Struct(queue, &result) != Queue_Result_Ok)
    {
        free(queue);
        return 1;
    }

    // It's the same right?
    if (result.tag != reference.tag)
    {
        free(queue);
        return 2;
    }

    for (size_t i = 0; i < MY_STRUCT_DATA_COUNT; i++)
    {
        if (result.data[i] != reference.data[i])
        {
            free(queue);
            return 2;
        }
    }

    free(queue);
    return 0;
}

// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(__STDC_NO_THREADS__)
#pragma message("No C11 threading support, skipping thread test (auto pass)")
#define QUEUE_TEST_THREADS 0
#else
#define QUEUE_TEST_THREADS 1
#include <threads.h>
#endif

// -----------------------------------------------------------------------------

#define QUEUE_TEST_THREADS_MAX 16

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
#include <amblaq/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   0
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <amblaq/queues.h>

#define QUEUE_MP   0
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <amblaq/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#define QUEUE_IMPLEMENTATION
#include <amblaq/queues.h>

#define CAST(x, y) ((x) y)

// -----------------------------------------------------------------------------

typedef enum Tag
{
      Spsc
    , Mpsc
    , Spmc
    , Mpmc

    , Max = Mpmc
}
Tag;

static const char* tag_to_name[] =
{
      "Spsc"
    , "Mpsc"
    , "Spmc"
    , "Mpmc"
};

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

#define EXPECT(x) do {if(!(x)) { free(q); return #x; }} while(0)

const char* null_pointers(Tag tag, unsigned count_in, unsigned count_out)
{
    (void) count_in;
    (void) count_out;

    Queue_Result result = make(tag, 0, NULL, NULL);
    void* q = NULL;

    EXPECT(result == Queue_Result_Error_Null_Bytes);

    return NULL;
}

const char* create(Tag tag, unsigned count_in, unsigned count_out)
{
    (void) count_in;
    (void) count_out;

    size_t bytes = -1;
    void* q = NULL;

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

        void* queue = malloc(bytes);

        Queue_Result create = make(tag, 1 << 8, q, &bytes);

        free(queue);

        EXPECT(create == Queue_Result_Ok);
    }

    return NULL;
}

const char* empty(Tag tag, unsigned count_in, unsigned count_out)
{
    (void) count_in;
    (void) count_out;

    size_t bytes = 0;
    void* q = NULL;

    make(tag, 1 << 8, NULL, &bytes);

    EXPECT(bytes > 0);

    q = malloc(bytes);

    make(tag, 1 << 8, q, &bytes);

    {
        Data data = {0};

        Queue_Result result_try_dequeue = try_dequeue(tag, q, &data);

        EXPECT(result_try_dequeue == Queue_Result_Empty);
    }

    {
        Data data = {0};

        Queue_Result result_dequeue = dequeue(tag, q, &data);

        EXPECT(result_dequeue == Queue_Result_Empty);
    }

    free(q);

    return NULL;
}

const char* full(Tag tag, unsigned count_in, unsigned count_out)
{
    (void) count_in;
    (void) count_out;

    size_t bytes = 0;
    void* q = NULL;

    make(tag, 1 << 8, NULL, &bytes);

    EXPECT(bytes > 0);

    q = malloc(bytes);

    make(tag, 1 << 8, q, &bytes);

    for (unsigned i = 0; i < (1 << 8); i++)
    {
        Data data = {0};

        EXPECT(enqueue(tag, q, &data) == Queue_Result_Ok);
    }

    {
        Data data = {0};

        Queue_Result result_try_dequeue = try_enqueue(tag, q, &data);

        EXPECT(result_try_dequeue == Queue_Result_Full);
    }

    {
        Data data = {0};

        Queue_Result result_dequeue = enqueue(tag, q, &data);

        EXPECT(result_dequeue == Queue_Result_Full);
    }

    free(q);

    return NULL;
}

typedef struct Thread_Data
{
    void*          q;
    int            multiplier;
    Tag            tag;
    atomic_size_t* global_count;
    atomic_size_t* done;
}
Thread_Data;

int thread_in(void* data)
{
    Data item =
    {
          11.0f
        , 22
        , {0}
    };

    Thread_Data* info = CAST(Thread_Data*, data);

    unsigned max = 10000 * info->multiplier;

    for (unsigned j = 0; j < max; j++)
    {
        while
        (
               enqueue(info->tag, info->q, &item)
            != Queue_Result_Ok
        );
    }

    atomic_fetch_sub_explicit(info->done, 1, memory_order_relaxed);

    return 0;
}

int thread_out(void* data)
{
    Thread_Data* info = CAST(Thread_Data*, data);

    unsigned max = 10000 * info->multiplier;

    for (unsigned j = 0; j < max; j++)
    {
        Data item = {0};
        while
        (
               dequeue(info->tag, info->q, &item)
            != Queue_Result_Ok
        );

        atomic_fetch_add_explicit
        (
              info->global_count
            , item.b
            , memory_order_relaxed
        );
    }

    atomic_fetch_sub_explicit(info->done, 1, memory_order_relaxed);

    return 0;
}

const char* sums10000(Tag tag, unsigned count_in, unsigned count_out)
{
#if QUEUE_TEST_THREADS

    void* q = NULL;
    {
        size_t bytes = 0;

        make(tag, 1 << 8, NULL, &bytes);

        EXPECT(bytes > 0);

        q = malloc(bytes);

        Queue_Result create = make(tag, 1 << 8, q, &bytes);

        EXPECT(create == Queue_Result_Ok);
    }

    thrd_t in_threads [QUEUE_TEST_THREADS_MAX];
    thrd_t out_threads[QUEUE_TEST_THREADS_MAX];

    atomic_size_t  done_in_count  = ATOMIC_VAR_INIT(count_in);
    atomic_size_t  done_out_count = ATOMIC_VAR_INIT(count_out);
    atomic_size_t  global_count   = ATOMIC_VAR_INIT(0);

    int multiplier_in  = QUEUE_TEST_THREADS_MAX / count_in;
    int multiplier_out = QUEUE_TEST_THREADS_MAX / count_out;

    Thread_Data data_in =
    {
          q
        , multiplier_in
        , tag
        , &global_count
        , &done_in_count
    };
    Thread_Data data_out =
    {
          q
        , multiplier_out
        , tag
        , &global_count
        , &done_out_count
    };

    for (unsigned i = 0; i < count_in; i++)
    {
        int result = thrd_create(&in_threads[i], thread_in, &data_in);
        EXPECT(result == thrd_success);
    }

    for (unsigned i = 0; i < count_out; i++)
    {
        int result = thrd_create(&out_threads[i], thread_out, &data_out);
        EXPECT(result == thrd_success);
    }

    while(atomic_load_explicit(&done_in_count,  memory_order_relaxed));
    while(atomic_load_explicit(&done_out_count, memory_order_relaxed));

    for (unsigned i = 0; i < count_in; i++)
    {
        int ignored = 0;
        int result  = thrd_join(in_threads[i], &ignored);

        EXPECT(result == thrd_success);
    }
    for (unsigned i = 0; i < count_out; i++)
    {
        int ignored = 0;
        int result  = thrd_join(out_threads[i], &ignored);

        EXPECT(result == thrd_success);
    }

    size_t expected_count = 10000ULL * 22 * QUEUE_TEST_THREADS_MAX;

    EXPECT(atomic_load(&global_count) == expected_count);
#else
    (void) count_in;
    (void) count_out;
    (void) tag;
#endif

    return NULL;
}

typedef const char* (*Test)(Tag, unsigned, unsigned);
#define TEST(x) { #x, x }

struct
{
    const char* name;
    Test        test;
}
static tests[] =
{
      TEST(null_pointers)
    , TEST(create)
    , TEST(empty)
    , TEST(full)
    , TEST(sums10000)
};

#if QUEUE_TEST_THREADS
    #define TEST_COUNT 5
#else
    #define TEST_COUNT 4
#endif

int main(int arg_count, char** args)
{
    (void) arg_count;
    (void) args;

    // -------------------------------------------------------------------------
    // test the example first
    // -------------------------------------------------------------------------
    {
        int result = example();

        printf
        (
              "Test: %s: %-20s: %s%s\n"
            , "Spsc"
            , "example"
            , ((result != 0) ? "FAIL: " : "PASS")
            , ((result != 0) ? "Bug" : "")
        );

        if (result != 0)
        {
            return 1;
        }
    }
    // -------------------------------------------------------------------------

    struct
    {
        unsigned count_in;
        unsigned count_out;
    }
    thread_counts[(Max+1)] =
    {
          {                     1,                      1}
        , {QUEUE_TEST_THREADS_MAX,                      1}
        , {                     1, QUEUE_TEST_THREADS_MAX}
        , {QUEUE_TEST_THREADS_MAX, QUEUE_TEST_THREADS_MAX}
    };

    for (unsigned tag = 0; tag < (Max + 1); tag++)
    {
        for (unsigned j = 0; j < TEST_COUNT; j++)
        {
            const char* error = tests[j].test
            (
                  tag
                , thread_counts[tag].count_in
                , thread_counts[tag].count_out
            );

            printf
            (
                  "Test: %s: %-20s: %s%s\n"
                , tag_to_name[tag]
                , tests[j].name
                , (error ? "FAIL: " : "PASS")
                , (error ? error : "")
            );

            fflush(stdout);

            if (error)
            {
                return 1;
            }
        }
    }

    return 0;
}
