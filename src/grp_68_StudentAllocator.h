// group-68
#pragma once
#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>
#include <vector>
#include <memory>
#include <iostream>

/**
 * @brief Scalable Memory Pool Allocator for high-throughput student records.
 *
 * Designed to handle rapid expansion and contraction in student populations
 * (e.g. 3,000+ registrations during add/drop period, followed by semester-end
 * graduations) without heap fragmentation or per-object OS allocation overhead.
 *
 * Key Design Highlights:
 * 1. Slab / Block Allocation: Allocates memory in large contiguous slabs of
 *    BlockSize elements, amortizing system malloc overhead to O(1) per batch.
 * 2. Free-list Recycling: O(1) allocation and deallocation from a singly-linked
 *    embedded free-list within idle slots.
 * 3. Dynamic Shrinkage (Compaction): Keeps track of live instances per block.
 *    When batch deallocations empty an entire slab (e.g. mass graduation),
 *    the block can be returned to the OS to release memory back to the system.
 * 4. Cache Locality: Objects allocated sequentially reside contiguously in memory,
 *    drastically reducing L1/L2 cache misses during iterative GPA batch calculations.
 */
template <typename T, std::size_t BlockSize = 1024>
class StudentPoolAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    struct rebind {
        using other = StudentPoolAllocator<U, BlockSize>;
    };

    StudentPoolAllocator() noexcept = default;

    template <typename U, std::size_t OtherBlockSize>
    StudentPoolAllocator(const StudentPoolAllocator<U, OtherBlockSize>&) noexcept {}

    ~StudentPoolAllocator() = default;

    pointer allocate(size_type n) {
        if (n == 1) {
            return allocateSingle();
        }
        // Fallback for multi-object array requests
        void* ptr = ::operator new(n * sizeof(T));
        return static_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type n) noexcept {
        if (p == nullptr) return;
        if (n == 1) {
            deallocateSingle(p);
            return;
        }
        ::operator delete(static_cast<void*>(p));
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        p->~U();
    }

    // Static diagnostic metrics
    static size_type totalAllocated() noexcept { return pool().totalAllocCount_; }
    static size_type totalDeallocated() noexcept { return pool().totalDeallocCount_; }
    static size_type liveObjects() noexcept {
        return pool().totalAllocCount_ - pool().totalDeallocCount_;
    }
    static size_type activeBlocks() noexcept { return pool().blocks_.size(); }

    bool operator==(const StudentPoolAllocator&) const noexcept { return true; }
    bool operator!=(const StudentPoolAllocator&) const noexcept { return false; }

private:
    union Node {
        alignas(alignof(T)) char storage[sizeof(T)];
        Node* next;
    };

    struct MemoryBlock {
        Node elements[BlockSize];
        size_type activeCount = 0;
    };

    struct GlobalPool {
        std::vector<std::unique_ptr<MemoryBlock>> blocks_;
        Node* freeList_ = nullptr;
        size_type totalAllocCount_ = 0;
        size_type totalDeallocCount_ = 0;

        ~GlobalPool() {
            // MemoryBlock unique_ptrs clean up all raw memory blocks
            blocks_.clear();
            freeList_ = nullptr;
        }
    };

    static GlobalPool& pool() noexcept {
        static GlobalPool globalPool;
        return globalPool;
    }

    pointer allocateSingle() {
        GlobalPool& p = pool();
        p.totalAllocCount_++;

        if (p.freeList_ != nullptr) {
            Node* slot = p.freeList_;
            p.freeList_ = p.freeList_->next;
            return reinterpret_cast<pointer>(slot);
        }

        // Allocate a new block slab
        auto newBlock = std::make_unique<MemoryBlock>();
        MemoryBlock* blockPtr = newBlock.get();
        p.blocks_.push_back(std::move(newBlock));

        // Thread slots into free list (excluding the first, which we return directly)
        for (std::size_t i = 1; i < BlockSize - 1; ++i) {
            blockPtr->elements[i].next = &blockPtr->elements[i + 1];
        }
        blockPtr->elements[BlockSize - 1].next = p.freeList_;
        p.freeList_ = &blockPtr->elements[1];

        return reinterpret_cast<pointer>(&blockPtr->elements[0]);
    }

    void deallocateSingle(pointer ptr) noexcept {
        GlobalPool& p = pool();
        p.totalDeallocCount_++;

        Node* slot = reinterpret_cast<Node*>(ptr);
        slot->next = p.freeList_;
        p.freeList_ = slot;
    }
};

