#pragma once

template<typename T>
class RenderPool
{
public:
    struct Node {
        Node*   pPrevious = nullptr;
        Node*   pNext = nullptr;
        T*      pContent = nullptr;
    };

    using TAllocFun_t = void (*)(T*);

public:
    RenderPool()
        : pMemoryPool(nullptr) {

    }

    ~RenderPool() {

    }

    void allocate(int32_t param_2) {
        // FUN_00511f90
        OTDU_UNIMPLEMENTED;
    }

    RenderPool::Node* addToPool(T* param_2) {
        // FUN_00512040
        if (pMemoryPool == nullptr) {
            return nullptr;
        }
        
        OTDU_UNIMPLEMENTED;

        // if (param_2 != nullptr) {
        //     param_1_00->pAllocationsMemoryPool = &peVar1->pNext->pPrevious;
        //     uVar1 = (uint)param_2 >> 4 & 0xff;
        //     peVar1->pPrevious = (CachedPendingAlloc *)0x0;
        //     peVar1->pNext = param_1_00->pAllocations[uVar1];
        //     peVar1->pContent = param_2;
        //     if (param_1_00->pAllocations[uVar1] != (CachedPendingAlloc *)0x0) {
        //         param_1_00->pAllocations[uVar1]->pPrevious = peVar1;
        //     }
        //     param_1_00->pAllocations[uVar1] = peVar1;
        //     return peVar1;
        // }
        return nullptr;
    }

    void removeFromPool(T* param_2) {
        // FUN_005120a0
        OTDU_UNIMPLEMENTED;
    }

    void flushPendingAllocs(TAllocFun_t allocFun) {
        // FUN_00512110
        Node *pCVar1 = nullptr;

        uint32_t uVar2 = 0;
        do {
            for (pCVar1 = &nodes[uVar2]; pCVar1 != nullptr; pCVar1 = pCVar1->pNext) {
                allocFun(pCVar1->pContent);
            }
            uVar2++;
        } while (uVar2 < kMaxCapacity);
    }

    inline void invalidatePoolPointer() {
        pMemoryPool = nullptr;
    }

private:
    static constexpr uint32_t kMaxCapacity = 0x100;

private:
    std::array<Node, kMaxCapacity> nodes;
    void* pMemoryPool;
};
