#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <unordered_map>
#include <vector>

// === MemoryArena ===
class MemoryArena
{
   public:
    MemoryArena(size_t sizeBytes = 64 * 1024 * 1024) : m_size(sizeBytes), m_offset(0)
    {
        m_buffer = static_cast<uint8_t*>(std::malloc(sizeBytes));
        if (!m_buffer)
        {
            throw std::bad_alloc();
        }
    }

    ~MemoryArena()
    {
        std::free(m_buffer);
    }

    void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t))
    {
        size_t current = reinterpret_cast<size_t>(m_buffer) + m_offset;
        size_t aligned = (current + alignment - 1) & ~(alignment - 1);
        size_t offset = aligned - reinterpret_cast<size_t>(m_buffer);

        if (offset + size > m_size)
        {
            throw std::bad_alloc();
        }

        void* ptr = m_buffer + offset;
        m_offset = offset + size;
        return ptr;
    }

    void Reset()
    {
        m_offset = 0;
    }

   private:
    uint8_t* m_buffer = nullptr;
    size_t m_offset = 0;
    size_t m_size = 0;
};

// === PoolAllocator ===
class PoolAllocator
{
   public:
    PoolAllocator(size_t elementSize = 64, size_t elementCount = 1024)
        : m_elementSize(std::max(elementSize, sizeof(void*))), m_elementCount(elementCount)
    {
        m_pool = static_cast<uint8_t*>(std::malloc(m_elementSize * m_elementCount));
        if (!m_pool)
        {
            throw std::bad_alloc();
        }

        m_freeList = reinterpret_cast<void**>(m_pool);
        for (size_t i = 0; i < m_elementCount - 1; ++i)
        {
            void* current = m_pool + i * m_elementSize;
            void* next = m_pool + (i + 1) * m_elementSize;
            *reinterpret_cast<void**>(current) = next;
        }
        *reinterpret_cast<void**>(m_pool + (m_elementCount - 1) * m_elementSize) = nullptr;
    }

    ~PoolAllocator()
    {
        std::free(m_pool);
    }

    void* Allocate()
    {
        if (!m_freeList)
        {
            throw std::bad_alloc();
        }

        void* head = m_freeList;
        m_freeList = reinterpret_cast<void**>(*m_freeList);
        return head;
    }

    void Free(void* ptr)
    {
        *reinterpret_cast<void**>(ptr) = m_freeList;
        m_freeList = reinterpret_cast<void**>(ptr);
    }

   private:
    uint8_t* m_pool = nullptr;
    void** m_freeList = nullptr;
    size_t m_elementSize = 0;
    size_t m_elementCount = 0;
};

// === TrackingAllocator ===
class TrackingAllocator
{
   public:
    void* Allocate(size_t size, const std::string& tag)
    {
        void* ptr = std::malloc(size);
        if (!ptr)
        {
            throw std::bad_alloc();
        }

        m_allocations[ptr] = {size, tag};
        return ptr;
    }

    void Free(void* ptr)
    {
        auto it = m_allocations.find(ptr);
        if (it != m_allocations.end())
        {
            m_allocations.erase(it);
        }
        std::free(ptr);
    }

    void ReportLeaks() const
    {
        if (m_allocations.empty())
        {
            std::cout << "[MEMORY] No leaks detected.\n";
            return;
        }

        std::cout << "[MEMORY] Leak Report:\n";
        for (const auto& [ptr, info] : m_allocations)
        {
            std::cout << " - Leak at " << ptr << " | Size: " << info.size << " | Tag: " << info.tag
                      << "\n";
        }
    }

   private:
    struct AllocationInfo
    {
        size_t size;
        std::string tag;
    };

    std::unordered_map<void*, AllocationInfo> m_allocations;
};

// === MemoryManager ===
class MemoryManager
{
   public:
    static void Initialize(size_t arenaSizeBytes = 64 * 1024 * 1024)
    {
        memoryArena = std::make_unique<MemoryArena>(arenaSizeBytes);
        poolAllocator = std::make_unique<PoolAllocator>();
        trackingAllocator = std::make_unique<TrackingAllocator>();
    }

    static void Shutdown()
    {
        trackingAllocator->ReportLeaks();
        memoryArena.reset();
        poolAllocator.reset();
        trackingAllocator.reset();
    }

    static MemoryArena& Arena()
    {
        return *memoryArena;
    }
    static PoolAllocator& Pool()
    {
        return *poolAllocator;
    }
    static TrackingAllocator& Tracker()
    {
        return *trackingAllocator;
    }

   private:
    inline static std::unique_ptr<MemoryArena> memoryArena;
    inline static std::unique_ptr<PoolAllocator> poolAllocator;
    inline static std::unique_ptr<TrackingAllocator> trackingAllocator;
};
