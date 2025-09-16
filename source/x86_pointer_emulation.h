#pragma once

// Object to represent a 64bits pointer as a 32bits integer.
// We need to use this for x64 targets (as several asset format store 32bit pointers side by side).
struct OTDUPointer
{
    static constexpr uint32_t kInvalidID = 0xffffffff;
    uint32_t ID = kInvalidID;
    
    OTDUPointer();
    ~OTDUPointer();

    const OTDUPointer& operator = (const void* pPointer) const;
    operator void*() const;

    bool operator == (std::nullptr_t) {
        const void* thisPtr = this;
        return thisPtr == nullptr;
    }

    bool operator != (std::nullptr_t) {
        const void* thisPtr = this;
        return thisPtr != nullptr;
    }

    template<typename T> operator T*() const { 
        const void* thisPtr = this; 
        return (T*)thisPtr;
    }
};

#if OTDU_64_BITS
using x86Pointer_t = OTDUPointer;
#else
using x86Pointer_t = void*;
#endif

static_assert(sizeof(x86Pointer_t) == 4, "Must match to ensure correct assets loading!");