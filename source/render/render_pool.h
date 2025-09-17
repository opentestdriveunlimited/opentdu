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

public:
    RenderPool() {
        : pMemoryPool(nullptr)

    }

    ~RenderPool() {

    }

    Node<T>* addToPool(T* param_2) {
        // FUN_00512040
        if (pMemoryPool == nullptr) {
            return nullptr;
        }
        
        OTDU_UNIMPLEMENTED;

        if (param_2 != nullptr) {
            param_1_00->pAllocationsMemoryPool = &peVar1->pNext->pPrevious;
            uVar1 = (uint)param_2 >> 4 & 0xff;
            peVar1->pPrevious = (CachedPendingAlloc *)0x0;
            peVar1->pNext = param_1_00->pAllocations[uVar1];
            peVar1->pContent = param_2;
            if (param_1_00->pAllocations[uVar1] != (CachedPendingAlloc *)0x0) {
                param_1_00->pAllocations[uVar1]->pPrevious = peVar1;
            }
            param_1_00->pAllocations[uVar1] = peVar1;
            return peVar1;
        }
        return nullptr;
    }

    void removeFromPool(T* param_2) {
        // FUN_005120a0
        OTDU_UNIMPLEMENTED;
    }

private:
    static constexpr uint32_t kMaxCapacity = 0x100;

private:
    std::array<Node<T>, kMaxCapacity> nodes;
    void* pMemoryPool;
};
