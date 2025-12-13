#pragma once

#include <memory_resource>
#include <iterator>

#include "dBlockAllocator.h"

template<typename T>
class ForwardList;

template<typename T>
class ForwardListIterator {
private:
    using Node = typename ForwardList<T>::Node;
    Node* currentNode;
    
    explicit ForwardListIterator(Node* node) : currentNode(node) {}
    
public:
    friend class ForwardList<T>;
    // требуемый + доп типы для std::forward_iterator_tag
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    ForwardListIterator() : currentNode(nullptr) {}
    ForwardListIterator(const ForwardListIterator&) = default;
    ForwardListIterator& operator=(const ForwardListIterator&) = default;
    
    reference operator*() const;
    pointer operator->() const;
    
    ForwardListIterator& operator++();
    ForwardListIterator operator++(int); //пост
    
    bool operator==(const ForwardListIterator& other) const;
    bool operator!=(const ForwardListIterator& other) const;
};

template<typename T>
class ForwardList {
private:
    // узел
    struct Node {
        T data;
        Node* next;
        // констр узла
        template<typename... Args>
        Node(Node* nextNode = nullptr, Args&&... args);
    };
    
    using allocator_type = std::pmr::polymorphic_allocator<Node>;
    
    Node* head;
    size_t elementCount;
    allocator_type allocator;
    
public:
    using iterator = ForwardListIterator<T>;
    // конс с алл-м
    explicit ForwardList(std::pmr::memory_resource* mr = std::pmr::get_default_resource());
    ~ForwardList();
    
    void pushFront(const T& value);
    void popFront();
    void clear();
    
    iterator begin();
    iterator end();
    
    T& front();
    
    size_t size() const;
    bool empty() const;

    friend class ForwardListIterator<T>;
};

#include "forwardList.ipp"