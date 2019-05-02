#include <initializer_list>
#include <cstdio>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>

// -----------------------------------------------------------------------------

#define QUEUE_MP   1
#define QUEUE_MC   1
#define QUEUE_TYPE uint64_t
#define QUEUE_IMPLEMENTATION
#include <queues/queues.h>

typedef Queue_Mpmc_uint64_t Queue_Struct;
typedef uint64_t            Queue_Type;

// -----------------------------------------------------------------------------
// http://the-witness.net/news/2012/11/scopeexit-in-c11/
// -----------------------------------------------------------------------------

template <typename F>
struct ScopeExit {
    ScopeExit(F fun) : f(fun) {}
    ~ScopeExit() { f(); }
    F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f) {
    return ScopeExit<F>(f);
};

#define STRING_JOIN2(arg1, arg2) DO_STRING_JOIN2(arg1, arg2)
#define DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define defer(code) \
    auto STRING_JOIN2(scope_exit_, __LINE__) = MakeScopeExit([&](){code;})

// -----------------------------------------------------------------------------

#define EXPECT(x) do {if(!(x)) { return {name, #x}; }} while(0)

// -----------------------------------------------------------------------------

int main(int, char**)
{
    struct Result
    {
        const char* name;
        const char* error;
    };

    std::function<Result()> tests[] =
    {
        []() -> Result
        {
            const char* name = "Null pointers";

            // Just expect not to crash, not testing results

            Queue_Result result = mpmc_make_queue_uint64_t(0, nullptr, nullptr);

            EXPECT(result == Queue_Result_Error_Null_Bytes);

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Create";

            size_t bytes;

            auto make = [](size_t a, Queue_Struct* b, size_t* c) -> Queue_Result
            {
                return mpmc_make_queue_uint64_t(a, b, c);
            };

            EXPECT(make( 0, nullptr, &bytes) == Queue_Result_Error_Too_Small);
            EXPECT(make( 1, nullptr, &bytes) == Queue_Result_Error_Too_Small);
            // min size

            EXPECT(make(13,  nullptr, &bytes) == Queue_Result_Error_Not_Pow2);
            EXPECT(make(255, nullptr, &bytes) == Queue_Result_Error_Not_Pow2);
            // must be pow2

            EXPECT(make(-1, nullptr, &bytes) == Queue_Result_Error_Too_Big);

            EXPECT
            (
                   make(-3000000, nullptr, &bytes)
                == Queue_Result_Error_Too_Big
            );

            EXPECT
            (
                   make(1ULL << 63, nullptr, &bytes)
                == Queue_Result_Error_Too_Big
            );

            EXPECT
            (
                   make(1ULL << 33, nullptr, &bytes)
                == Queue_Result_Error_Too_Big
            );
            // Insane sizes

            {
                mpmc_make_queue_uint64_t(1 << 8, nullptr, &bytes);

                EXPECT(bytes > 0);
                EXPECT(bytes < 100000);

                Queue_Struct* q = static_cast<Queue_Struct*>(malloc(bytes));
                defer(free(q));

                Queue_Result create =
                    mpmc_make_queue_uint64_t(1 << 8, q, &bytes);

                EXPECT(create == Queue_Result_Ok);
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Empty";

            {
                size_t bytes = 0;

                mpmc_make_queue_uint64_t(1 << 8, nullptr, &bytes);

                EXPECT(bytes > 0);

                Queue_Struct* q = static_cast<Queue_Struct*>(malloc(bytes));
                defer(free(q));

                mpmc_make_queue_uint64_t(1 << 8, q, &bytes);

                {
                    Queue_Type data{};

                    Queue_Result try_dequeue =
                        mpmc_try_dequeue_uint64_t(q, &data);

                    EXPECT(try_dequeue == Queue_Result_Empty);
                }

                {
                    Queue_Type data{};

                    Queue_Result dequeue = mpmc_dequeue_uint64_t(q, &data);

                    EXPECT(dequeue == Queue_Result_Empty);
                }
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Full";

            {
                size_t bytes = 0;

                mpmc_make_queue_uint64_t(1 << 8, nullptr, &bytes);

                EXPECT(bytes > 0);

                Queue_Struct* q = static_cast<Queue_Struct*>(malloc(bytes));
                defer(free(q));

                for (unsigned i = 0; i < (1 << 8); i++)
                {
                    Queue_Type data{};

                    EXPECT(mpmc_enqueue_uint64_t(q, &data) == Queue_Result_Ok);
                }

                {
                    Queue_Type data{};

                    Queue_Result try_dequeue =
                        mpmc_try_enqueue_uint64_t(q, &data);

                    EXPECT(try_dequeue == Queue_Result_Full);
                }

                {
                    Queue_Type data{};

                    Queue_Result dequeue = mpmc_enqueue_uint64_t(q, &data);

                    EXPECT(dequeue == Queue_Result_Full);
                }
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "10,000 sums";

            unsigned in_thread_count  = 1;
            unsigned out_thread_count = 1;

            Queue_Struct* q;
            {
                size_t bytes = 0;

                mpmc_make_queue_uint64_t(1 << 8, nullptr, &bytes);

                EXPECT(bytes > 0);

                q = static_cast<Queue_Struct*>(malloc(bytes));

                Queue_Result create =
                    mpmc_make_queue_uint64_t(1 << 8, q, &bytes);

                EXPECT(create == Queue_Result_Ok);
            }
            defer(free(q));

            std::vector<std::thread> in_threads;
            std::vector<std::thread> out_threads;

            in_threads.reserve(in_thread_count);
            out_threads.reserve(out_thread_count);

            std::atomic<unsigned> in_done_count {in_thread_count};
            std::atomic<unsigned> out_done_count{out_thread_count};
            std::atomic<size_t>   global_count  {};

            for (unsigned i = 0; i < in_thread_count; i++)
            {
                in_threads.emplace_back
                (
                    [q, &in_done_count]()
                    {
                        Queue_Type item{22};

                        for (unsigned j = 0; j < 10000; j++)
                        {
                            while
                            (
                                   mpmc_enqueue_uint64_t(q, &item)
                                != Queue_Result_Ok
                            );
                        }

                        in_done_count.fetch_sub(1, std::memory_order_relaxed);
                    }
                );
            }

            for (unsigned i = 0; i < out_thread_count; i++)
            {
                out_threads.emplace_back
                (
                    [q, &out_done_count, &global_count]()
                    {
                        Queue_Type item{0};

                        for (unsigned j = 0; j < 10000; j++)
                        {
                            while
                            (
                                   mpmc_dequeue_uint64_t(q, &item)
                                != Queue_Result_Ok
                            );

                            global_count.fetch_add
                            (
                                item, std::memory_order_relaxed
                            );
                        }

                        out_done_count.fetch_sub(1, std::memory_order_relaxed);
                    }
                );
            }

            while( in_done_count.load(std::memory_order_relaxed));
            while(out_done_count.load(std::memory_order_relaxed));

            for (unsigned i = 0; i < in_thread_count; i++)
            {
                in_threads[i].join();
            }
            for (unsigned i = 0; i < out_thread_count; i++)
            {
                out_threads[i].join();
            }

            EXPECT(global_count.load() == (10000ULL*22*out_thread_count));

            return {name, nullptr};
        }
    };

    int fail_count = 0;

    for(const auto& test : tests)
    {
        Result result = test();

        fail_count += result.error ? 1 : 0;

        printf
        (
            "Test: %-20s: %s%s\n"
            , result.name
            , (result.error ? "FAIL: " : "PASS")
            , (result.error ? result.error : "")
        );

        fflush(stdout);

        if (result.error)
        {
            return 1;
        }
    }

    return 0;
}
