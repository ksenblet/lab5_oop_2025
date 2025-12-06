#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "dBlockAllocator.h"

DynamicBlockAllocator::DynamicBlockAllocator() {}

DynamicBlockAllocator::~DynamicBlockAllocator() noexcept {
    for (const auto& [ptr, info] : activeBlocks) {
        std::free(ptr);
    }
    activeBlocks.clear();
    
    for (const auto& [size, ptr] : freeBlocks) {
        std::free(ptr);
    }
    freeBlocks.clear();
}

void* DynamicBlockAllocator::do_allocate(size_t bytes, size_t alignment) {
    if (bytes == 0) {
        return nullptr;
    }
    
    void* ptr = nullptr;
    
    // 1. Пытаемся переиспользовать свободный блок
    auto it = freeBlocks.lower_bound(bytes);
    if (it != freeBlocks.end()) {
        // Нашли блок подходящего или большего размера
        ptr = it->second;
        freeBlocks.erase(it);
    } else {
        // 2. Выделяем новый блок на куче
        ptr = std::malloc(bytes);
        if (!ptr) {
            throw std::bad_alloc();
        }
    }
    
    // 3. Сохраняем информацию о блоке в std::map
    activeBlocks[ptr] = {bytes, alignment};
    
    return ptr;
}

void DynamicBlockAllocator::do_deallocate(void* ptr, size_t bytes, size_t alignment) {
    if (!ptr) {
        return;
    }
    
    auto it = activeBlocks.find(ptr);
    if (it != activeBlocks.end()) {
        // Сохраняем блок для переиспользования (не освобождаем!)
        freeBlocks[it->second.size] = ptr;
        activeBlocks.erase(it);
    } else {
        std::cout << "An attempt to free an unallocated pointer: " 
                  << ptr << "\n";
    }
}

bool DynamicBlockAllocator::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}