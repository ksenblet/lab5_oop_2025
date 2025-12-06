#pragma once

#include <memory_resource>
#include <map>
#include <cstddef>

// каждый объект в отдельном блоке на куче
// отслеживание через std::map
// переиспользование освобожденной памяти
class DynamicBlockAllocator : public std::pmr::memory_resource {
public:
    DynamicBlockAllocator();
    ~DynamicBlockAllocator() noexcept override;

    DynamicBlockAllocator(const DynamicBlockAllocator&) = delete;
    DynamicBlockAllocator& operator=(const DynamicBlockAllocator&) = delete;
    DynamicBlockAllocator(DynamicBlockAllocator&&) = delete;
    DynamicBlockAllocator& operator=(DynamicBlockAllocator&&) = delete;

private:
    void* do_allocate(size_t bytes, size_t alignment) override;
    void do_deallocate(void* p, size_t bytes, size_t alignment) override;
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

    // Структура для хранения информации о блоке
    struct BlockInfo {
        size_t size;
        size_t alignment;
    };

    // std::map для отслеживания активных блоков (ключ - указатель)
    std::map<void*, BlockInfo> activeBlocks;

    // std::map для свободных блоков, сгруппированных по размеру
    std::map<size_t, void*> freeBlocks;
};