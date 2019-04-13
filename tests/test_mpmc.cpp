#include <atomic-queues/mpmc.h>

#include <initializer_list>
#include <cstdio>
#include <functional>

#define EXPECT(x) do {if(!(x)) { return {name, #x}; }} while(0)

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

                // RAM:: TODO: Malloc
            }

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Empty";

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Full";

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "10,000 sums";

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
