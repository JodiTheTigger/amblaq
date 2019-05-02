#include <array>

using namespace std;

// -----------------------------------------------------------------------------

struct Data
{
    float                   a;
    uint32_t                b;
    std::array<uint8_t, 16> bytes;
};

#define QUEUE_MP   0
#define QUEUE_MC   0
#define QUEUE_TYPE Data
#include <queues/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   0
#define QUEUE_TYPE Data
#include <queues/queues.h>

#define QUEUE_MP   0
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#include <queues/queues.h>

#define QUEUE_MP   1
#define QUEUE_MC   1
#define QUEUE_TYPE Data
#include <queues/queues.h>

// -----------------------------------------------------------------------------

#define EXPECT(x) do {if(!(x)) { return #x; }} while(0)

const char* null_pointers()
{
    Queue_Result result = mpmc_make_queue_Data(0, nullptr, nullptr);

    EXPECT(result == Queue_Result_Error_Null_Bytes);

    return nullptr;
}
