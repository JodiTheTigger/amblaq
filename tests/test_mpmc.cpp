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

            auto result = mpmc_make_queue(0, nullptr);
            EXPECT(result < 0);

            return {name, nullptr};
        }
        ,
        []() -> Result
        {
            const char* name = "Create/Destroy";

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
