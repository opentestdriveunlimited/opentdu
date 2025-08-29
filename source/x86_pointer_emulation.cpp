#include "shared.h"
#include "x86_pointer_emulation.h"

static constexpr uint32_t kPointerRegisterSize = 10000;

static std::array<void*, kPointerRegisterSize> gPointerRegister;
static std::queue<uint32_t> gPointerRegisterFreeList;
static uint32_t gPointerRegisterBackIndex = 0;

OTDUPointer::OTDUPointer()
{
    OTDU_ASSERT_FATAL(gPointerRegisterBackIndex < kPointerRegisterSize && !gPointerRegisterFreeList.empty());

    if (!gPointerRegisterFreeList.empty())
    {
        ID = gPointerRegisterFreeList.front();
        gPointerRegisterFreeList.pop();
    }
    else
    {
        ID = gPointerRegisterBackIndex++;
    }
}

OTDUPointer::~OTDUPointer()
{
    gPointerRegisterFreeList.push(ID);
}

const OTDUPointer& OTDUPointer::operator = (const void* pPointer) const
{
    OTDU_ASSERT(ID != OTDUPointer::kInvalidID);

    gPointerRegister[ID] = const_cast<void*>(pPointer);
    return *this;
}

OTDUPointer::operator void *() const
{
    return gPointerRegister[ID];
}
