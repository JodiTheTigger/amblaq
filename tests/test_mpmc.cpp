#include <atomic-queues/mpmc.h>

#include <initializer_list>
#include <cstdio>
#include <functional>
#include <vector>
#include <thread>
#include <atomic>

#define EXPECT(x) do {if(!(x)) { return {name, #x}; }} while(0)


// -----------------------------------------------------------------------------
// http://the-witness.net/news/2012/11/scopeexit-in-c11/
// -----------------------------------------------------------------------------

template <typename F>
struct ScopeExit {
    ScopeExit(F f) : f(f) {}
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

            mpmc_make_queue(0, nullptr);
            mpmc_try_enqueue(nullptr, nullptr);
            mpmc_try_dequeue(nullptr, nullptr);
            mpmc_enqueue(nullptr, nullptr);
            mpmc_dequeue(nullptr, nullptr);

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Create";

            EXPECT(mpmc_make_queue( 0,       nullptr) < 0);
            EXPECT(mpmc_make_queue( 1,       nullptr) < 0);
            EXPECT(mpmc_make_queue(-1,       nullptr) < 0);
            EXPECT(mpmc_make_queue(-3000000, nullptr) < 0);
            // min size

            EXPECT(mpmc_make_queue(13,  nullptr) < 0);
            EXPECT(mpmc_make_queue(255, nullptr) < 0);
            // must be pow2

            EXPECT(mpmc_make_queue(2^63, nullptr) < 0);
            EXPECT(mpmc_make_queue(2^33, nullptr) < 0);
            // Insane sizes

            {
                size_t bytes = mpmc_make_queue(2^8, nullptr);

                EXPECT(bytes > 0);

                Queue2_Mpmc* q = static_cast<Queue2_Mpmc*>(malloc(bytes));
                defer(free(q));

                size_t bytes2 = mpmc_make_queue(2^8, q);

                EXPECT(bytes2 == bytes);
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Empty";

            {
                size_t bytes = mpmc_make_queue(2^8, nullptr);

                EXPECT(bytes > 0);

                Queue2_Mpmc* q = static_cast<Queue2_Mpmc*>(malloc(bytes));
                defer(free(q));

                mpmc_make_queue(2^8, q);

                {
                    QUEUE2_MPMC_TYPE data{};

                    Queue2_Result try_dequeue = mpmc_try_dequeue(q, &data);

                    EXPECT(try_dequeue == Queue2_Result_Empty);
                }

                {
                    QUEUE2_MPMC_TYPE data{};

                    Queue2_Result dequeue = mpmc_dequeue(q, &data);

                    EXPECT(dequeue == Queue2_Result_Empty);
                }
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Full";

            {
                size_t bytes = mpmc_make_queue(2^8, nullptr);

                EXPECT(bytes > 0);

                Queue2_Mpmc* q = static_cast<Queue2_Mpmc*>(malloc(bytes));
                defer(free(q));

                mpmc_make_queue(2^8, q);

                for (unsigned i = 0; i < (2^8); i++)
                {
                    QUEUE2_MPMC_TYPE data{};

                    EXPECT(mpmc_enqueue(q, &data) == Queue2_Result_Ok);
                }

                {
                    QUEUE2_MPMC_TYPE data{};

                    Queue2_Result try_dequeue = mpmc_try_enqueue(q, &data);

                    EXPECT(try_dequeue == Queue2_Result_Full);
                }

                {
                    QUEUE2_MPMC_TYPE data{};

                    Queue2_Result dequeue = mpmc_enqueue(q, &data);

                    EXPECT(dequeue == Queue2_Result_Full);
                }
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "10,000 sums";

            unsigned in_thread_count  = 2;
            unsigned out_thread_count = 2;

            Queue2_Mpmc* q;
            {
                size_t bytes = mpmc_make_queue(2^8, nullptr);

                EXPECT(bytes > 0);

                Queue2_Mpmc* q = static_cast<Queue2_Mpmc*>(malloc(bytes));
                mpmc_make_queue(2^4, q);
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
                        QUEUE2_MPMC_TYPE item{22};

                        for (unsigned i = 0; i < 10000; i++)
                        {
                            while(mpmc_enqueue(q, &item) != Queue2_Result_Ok);
                        }

                        in_done_count.fetch_add(1, std::memory_order_relaxed);
                    }
                );
            }

            for (unsigned i = 0; i < out_thread_count; i++)
            {
                out_threads.emplace_back
                (
                    [q, &out_done_count, &global_count]()
                    {
                        QUEUE2_MPMC_TYPE item{0};

                        for (unsigned i = 0; i < 10000; i++)
                        {
                            while(mpmc_enqueue(q, &item) != Queue2_Result_Ok);

                            global_count.fetch_add
                            (
                                item, std::memory_order_relaxed
                            );
                        }

                        out_done_count.fetch_add(1, std::memory_order_relaxed);
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
                in_threads[i].join();
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
    }


    return fail_count;
}
