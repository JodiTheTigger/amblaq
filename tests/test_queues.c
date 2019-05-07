#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <threads.h>

#if defined(__STDC_NO_THREADS__)
#pramga message No C11 threading support, skipping threading tests (auto pass)
#define QUEUE_TEST_THREADS 0
#else
#define QUEUE_TEST_THREADS 1
#endif

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

const char* sums10000(Tag tag, unsigned count_in, unsigned count_out)
{
#if QUEUE_TEST_THREADS
    // Stubbed for now!
    (void) count_in;
    (void) count_out;
    (void) tag;
    return "Stubbed out (TODO)";
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

    struct
    {
        unsigned count_in;
        unsigned count_out;
    }
    thread_counts[(Max+1)] =
    {
          { 1,  1}
        , {16,  1}
        , { 1, 16}
        , {16, 16}
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
