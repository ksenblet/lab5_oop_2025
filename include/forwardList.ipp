#include "forwardList.h"

template<typename T>
typename ForwardListIterator<T>::reference 
ForwardListIterator<T>::operator*() const {
    return currentNode->data;
}

template<typename T>
typename ForwardListIterator<T>::pointer 
ForwardListIterator<T>::operator->() const {
    return &currentNode->data;
}

template<typename T>
ForwardListIterator<T>& ForwardListIterator<T>::operator++() {
    if (currentNode) {
        currentNode = currentNode->next;
    }
    return *this;
}

// пост
template<typename T>
ForwardListIterator<T> ForwardListIterator<T>::operator++(int) {
    ForwardListIterator temp = *this;
    ++(*this);
    return temp;
}

template<typename T>
bool ForwardListIterator<T>::operator==(const ForwardListIterator& other) const {
    return currentNode == other.currentNode;
}

template<typename T>
bool ForwardListIterator<T>::operator!=(const ForwardListIterator& other) const {
    return !(*this == other);
}

// конструктор узла
template<typename T>
template<typename... Args>
ForwardList<T>::Node::Node(Node* nextNode, Args&&... args)
    : data(std::forward<Args>(args)...), next(nextNode) {}


// для списка
template<typename T>
ForwardList<T>::ForwardList(std::pmr::memory_resource* mr)
    : head(nullptr), elementCount(0), allocator(mr) {}

template<typename T>
ForwardList<T>::~ForwardList() {
    clear();
}

template<typename T>
void ForwardList<T>::pushFront(const T& value) {
    Node* newNode = allocator.allocate(1);
    allocator.construct(newNode, head, value);
    head = newNode;
    elementCount++;
}

template<typename T>
void ForwardList<T>::popFront() {
    if (!head) {
        return;
    }
    
    Node* nodeToDelete = head;
    head = head->next;
    std::allocator_traits<allocator_type>::destroy(allocator, nodeToDelete);
    allocator.deallocate(nodeToDelete, 1);
    elementCount--;
}

template<typename T>
void ForwardList<T>::clear() {
    while (!empty()) {
        popFront();  
    }
}

template<typename T>
typename ForwardList<T>::iterator ForwardList<T>::begin() {
    return iterator(head);
}

template<typename T>
typename ForwardList<T>::iterator ForwardList<T>::end() {
    return iterator(nullptr);
}

// знач 1
template<typename T>
T& ForwardList<T>::front() {
    return head->data;
}

template<typename T>
size_t ForwardList<T>::size() const {
    return elementCount;
}

template<typename T>
bool ForwardList<T>::empty() const {
    return elementCount == 0;
}