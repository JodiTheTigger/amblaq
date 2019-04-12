#include <atomic-queues/mpmc.h>

#include <initializer_list>
#include <cstdio>
#include <functional>


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

    for(const auto& test : tests)
    {
        Result result = test();

        printf
        (
            "Test: %-20s: %s%s\n"
            , result.name
            , (result.error ? "FAIL: " : "PASS")
            , (result.error ? result.error : "")
        );
    }


    return 0;
}
