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
