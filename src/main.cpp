#include <iostream>
#include <string>

#include "dBlockAllocator.h"
#include "forwardList.h"

// сложный тип для проверки
struct Type {
    int x;
    double val;
    std::string st;
    
    Type(int i = 0, double j = 0.0, std::string k = "")
        : x(i), val(j), st(std::move(k)) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Type& t) {
        return os << t.x << " " << t.val << " \"" << t.st << "\"";
    }
};

int main() {
    DynamicBlockAllocator allocator;
    
    std::cout << "   Test with (int):\n";
    ForwardList<int> intList(&allocator);
    
    intList.pushFront(55);
    intList.pushFront(20);
    intList.pushFront(1);
    
    std::cout << "   List elements: ";
    for (const auto& i : intList) {
        std::cout << i << " ";
    }
    std::cout << "\n   Size: " << intList.size() << "\n";
    
    std::cout << "\n   Test with complex Type):\n";
    ForwardList<Type> complexList(&allocator);
    
    complexList.pushFront(Type(3, 1.5, "ccc"));
    complexList.pushFront(Type(2, 2.22, "bbb"));
    complexList.pushFront(Type(1, 8.9, "aaa"));
    
    std::cout << "   List elements:\n";
    for (const auto& i : complexList) {
        std::cout << "   " << i << "\n";
    }
    
    std::cout << "\n   Forward iterator:\n";
    ForwardList<std::string> stringList(&allocator);
    
    stringList.pushFront("YRA");
    stringList.pushFront("LABA 5 IS COMPLETED");
    
    std::cout << "  Elements: ";
    for (const auto& str : stringList) {
        std::cout << str << " ";
    }
    
    std::cout << "\n\n   Memory reuse:\n";
    {
        ForwardList<int> tempList(&allocator);
        for (int i = 0; i < 5; ++i) {
            tempList.pushFront(i * 128);
        }
        std::cout << "   Temporary list with 5 el. created\n";
        // автоматом уничтожен
    }
    std::cout << "   Memory freed\n\n";
    std::cout << "   ALL!";
    return 0;
}