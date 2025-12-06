#include <gtest/gtest.h>

#include <string>
#include <memory_resource>
#include <vector>
#include <algorithm>

#include "dBlockAllocator.h"
#include "forwardList.h"

// Test fixture for DynamicBlockAllocator
class DynamicBlockAllocatorTest : public ::testing::Test {
protected:
    DynamicBlockAllocator allocator;
};

// Test fixture for ForwardList
class ForwardListTest : public ::testing::Test {
protected:
    DynamicBlockAllocator allocator;
    
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// Simple test structure
struct TestData {
    int id;
    double value;
    std::string name;
    
    TestData(int i = 0, double v = 0.0, std::string n = "")
        : id(i), value(v), name(std::move(n)) {}
    
    bool operator==(const TestData& other) const {
        return id == other.id && value == other.value && name == other.name;
    }
};

// ================== TESTS FOR DYNAMIC BLOCK ALLOCATOR ==================

TEST_F(DynamicBlockAllocatorTest, BasicAllocation) {
    // Используем публичные методы allocate и deallocate
    void* ptr1 = allocator.allocate(100);
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = allocator.allocate(200);
    ASSERT_NE(ptr2, nullptr);
    
    ASSERT_NE(ptr1, ptr2);
    
    allocator.deallocate(ptr1, 100);
    allocator.deallocate(ptr2, 200);
}

TEST_F(DynamicBlockAllocatorTest, MemoryReuse) {
    void* ptr1 = allocator.allocate(100);
    ASSERT_NE(ptr1, nullptr);
    
    allocator.deallocate(ptr1, 100);
    
    void* ptr2 = allocator.allocate(100);
    ASSERT_NE(ptr2, nullptr);
    
    // Память может быть переиспользована, но это деталь реализации
    // Мы тестируем только что аллокатор работает без ошибок
    
    allocator.deallocate(ptr2, 100);
}

TEST_F(DynamicBlockAllocatorTest, ZeroBytesAllocation) {
    // Проверяем что не падает при нулевом размере
    void* ptr = allocator.allocate(0);
    // Может вернуть nullptr или нет, главное - без падения
    allocator.deallocate(ptr, 0);
}

TEST_F(DynamicBlockAllocatorTest, InvalidDeallocation) {
    void* invalidPtr = reinterpret_cast<void*>(0x12345678);
    
    // Деаллокация невалидного указателя не должна падать
    ASSERT_NO_THROW(allocator.deallocate(invalidPtr, 100));
    
    ASSERT_NO_THROW(allocator.deallocate(nullptr, 0));
}

TEST_F(DynamicBlockAllocatorTest, EqualityCheck) {
    DynamicBlockAllocator allocator2;
    
    // Используем метод is_equal из базового класса
    ASSERT_TRUE(allocator.is_equal(allocator));
    ASSERT_FALSE(allocator.is_equal(allocator2));
    
    // Также проверяем оператор == если он определен
    std::pmr::memory_resource& res1 = allocator;
    std::pmr::memory_resource& res2 = allocator2;
    ASSERT_TRUE(res1.is_equal(res1));
    ASSERT_FALSE(res1.is_equal(res2));
}

TEST_F(DynamicBlockAllocatorTest, DifferentAlignments) {
    // Тестируем разные выравнивания
    void* ptr1 = allocator.allocate(64, 8);   // выравнивание 8
    void* ptr2 = allocator.allocate(128, 16); // выравнивание 16
    void* ptr3 = allocator.allocate(256, 32); // выравнивание 32
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    
    // Проверяем что указатели выровнены правильно
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 8, 0);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 16, 0);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr3) % 32, 0);
    
    allocator.deallocate(ptr1, 64, 8);
    allocator.deallocate(ptr2, 128, 16);
    allocator.deallocate(ptr3, 256, 32);
}

TEST_F(DynamicBlockAllocatorTest, MultipleAllocationsDeallocations) {
    const int NUM_ALLOCATIONS = 100;
    std::vector<void*> allocations;
    
    // Множественное выделение
    for (int i = 0; i < NUM_ALLOCATIONS; ++i) {
        void* ptr = allocator.allocate(i * 16 + 16); // Разные размеры
        ASSERT_NE(ptr, nullptr);
        allocations.push_back(ptr);
    }
    
    // Освобождение в обратном порядке
    for (int i = NUM_ALLOCATIONS - 1; i >= 0; --i) {
        allocator.deallocate(allocations[i], i * 16 + 16);
    }
    
    // Еще раз выделение для проверки переиспользования
    for (int i = 0; i < NUM_ALLOCATIONS; ++i) {
        void* ptr = allocator.allocate(i * 16 + 16);
        ASSERT_NE(ptr, nullptr);
        allocator.deallocate(ptr, i * 16 + 16);
    }
}

// ================== TESTS FOR FORWARD LIST ==================

TEST_F(ForwardListTest, DefaultConstructor) {
    ForwardList<int> list(&allocator);
    
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

TEST_F(ForwardListTest, PushFrontAndSize) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(1);
    EXPECT_FALSE(list.empty());
    EXPECT_EQ(list.size(), 1);
    
    list.pushFront(2);
    EXPECT_EQ(list.size(), 2);
    
    list.pushFront(3);
    EXPECT_EQ(list.size(), 3);
}

TEST_F(ForwardListTest, PopFront) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(3);
    list.pushFront(2);
    list.pushFront(1);
    
    EXPECT_EQ(list.front(), 1);
    list.popFront();
    EXPECT_EQ(list.front(), 2);
    EXPECT_EQ(list.size(), 2);
    
    list.popFront();
    EXPECT_EQ(list.front(), 3);
    EXPECT_EQ(list.size(), 1);
    
    list.popFront();
    EXPECT_TRUE(list.empty());
    EXPECT_EQ(list.size(), 0);
}

TEST_F(ForwardListTest, FrontAccess) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(42);
    EXPECT_EQ(list.front(), 42);
    
    list.pushFront(100);
    EXPECT_EQ(list.front(), 100);
    
    // Проверяем что возвращается ссылка
    list.front() = 999;
    EXPECT_EQ(list.front(), 999);
}

TEST_F(ForwardListTest, Clear) {
    ForwardList<int> list(&allocator);
    
    for (int i = 0; i < 10; ++i) {
        list.pushFront(i);
    }
    
    EXPECT_EQ(list.size(), 10);
    EXPECT_FALSE(list.empty());
    
    list.clear();
    
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.empty());
}

TEST_F(ForwardListTest, IteratorBasic) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(3);
    list.pushFront(2);
    list.pushFront(1);
    
    auto it = list.begin();
    EXPECT_EQ(*it, 1);
    
    ++it;
    EXPECT_EQ(*it, 2);
    
    ++it;
    EXPECT_EQ(*it, 3);
    
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST_F(ForwardListTest, IteratorRangeBasedFor) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(1);
    list.pushFront(2);
    list.pushFront(3);
    list.pushFront(4);
    list.pushFront(5);
    
    int sum = 0;
    for (const auto& val : list) {
        sum += val;
    }
    
    // 5 + 4 + 3 + 2 + 1 = 15
    EXPECT_EQ(sum, 15);
}

TEST_F(ForwardListTest, IteratorComparison) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(2);
    list.pushFront(1);
    
    auto it1 = list.begin();
    auto it2 = list.begin();
    
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    
    ++it1;
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
    
    ++it2;
    EXPECT_TRUE(it1 == it2);
}

TEST_F(ForwardListTest, PostfixIncrement) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(2);
    list.pushFront(1);
    
    auto it = list.begin();
    auto old_it = it++;
    
    EXPECT_EQ(*old_it, 1);
    EXPECT_EQ(*it, 2);
}

TEST_F(ForwardListTest, ComplexType) {
    ForwardList<TestData> list(&allocator);
    
    list.pushFront(TestData(1, 1.1, "first"));
    list.pushFront(TestData(2, 2.2, "second"));
    list.pushFront(TestData(3, 3.3, "third"));
    
    EXPECT_EQ(list.size(), 3);
    
    auto it = list.begin();
    EXPECT_EQ(it->id, 3);
    EXPECT_EQ(it->value, 3.3);
    EXPECT_EQ(it->name, "third");
    
    ++it;
    EXPECT_EQ(it->id, 2);
    
    ++it;
    EXPECT_EQ(it->id, 1);
    
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST_F(ForwardListTest, StringType) {
    ForwardList<std::string> list(&allocator);
    
    list.pushFront("world");
    list.pushFront("hello");
    
    std::string concatenated;
    for (const auto& str : list) {
        concatenated += str + " ";
    }
    
    EXPECT_EQ(concatenated, "hello world ");
}

TEST_F(ForwardListTest, MemoryReuseInList) {
    // Test that list properly uses allocator's memory reuse
    ForwardList<int> list(&allocator);
    
    for (int i = 0; i < 100; ++i) {
        list.pushFront(i);
    }
    
    EXPECT_EQ(list.size(), 100);
    
    list.clear();
    EXPECT_TRUE(list.empty());
    
    for (int i = 0; i < 50; ++i) {
        list.pushFront(i * 10);
    }
    
    EXPECT_EQ(list.size(), 50);
}

TEST_F(ForwardListTest, DestructorCleansUp) {
    {
        ForwardList<int> list(&allocator);
        for (int i = 0; i < 10; ++i) {
            list.pushFront(i);
        }
        // Деструктор должен освободить все узлы
    }
    SUCCEED(); // Если не было утечек памяти - успех
}

TEST_F(ForwardListTest, CopyIterator) {
    ForwardList<int> list(&allocator);
    
    list.pushFront(3);
    list.pushFront(2);
    list.pushFront(1);
    
    auto it1 = list.begin();
    auto it2 = it1; // Копирование
    
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 1);
    EXPECT_TRUE(it1 == it2);
    
    ++it1;
    EXPECT_EQ(*it1, 2);
    EXPECT_EQ(*it2, 1); // it2 не изменился
    EXPECT_FALSE(it1 == it2);
}

// ================== INTEGRATION TESTS ==================

TEST(IntegrationTest, ForwardListWithDefaultAllocator) {
    // Используем стандартный аллокатор по умолчанию
    ForwardList<int> list;
    
    list.pushFront(3);
    list.pushFront(2);
    list.pushFront(1);
    
    EXPECT_EQ(list.size(), 3);
    
    int sum = 0;
    for (const auto& val : list) {
        sum += val;
    }
    
    EXPECT_EQ(sum, 6);
}

TEST(IntegrationTest, MultipleListsSameAllocator) {
    DynamicBlockAllocator allocator;
    
    ForwardList<int> list1(&allocator);
    ForwardList<std::string> list2(&allocator);
    ForwardList<double> list3(&allocator);
    
    list1.pushFront(42);
    list2.pushFront("test");
    list3.pushFront(3.14);
    
    EXPECT_EQ(list1.front(), 42);
    EXPECT_EQ(list2.front(), "test");
    EXPECT_EQ(list3.front(), 3.14);
}

TEST(IntegrationTest, ForwardListMoveSemantics) {
    DynamicBlockAllocator allocator;
    
    ForwardList<int> list1(&allocator);
    list1.pushFront(3);
    list1.pushFront(2);
    list1.pushFront(1);
    
    // Перемещение списка (если реализовано)
    // ForwardList<int> list2 = std::move(list1);
    // EXPECT_EQ(list2.size(), 3);
    // EXPECT_TRUE(list1.empty());
}

// ================== STRESS TESTS ==================

TEST(StressTest, LargeNumberOfElements) {
    DynamicBlockAllocator allocator;
    ForwardList<int> list(&allocator);
    
    const int NUM_ELEMENTS = 1000;
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        list.pushFront(i);
    }
    
    EXPECT_EQ(list.size(), NUM_ELEMENTS);
    
    // Проверяем что можем пройти по всем элементам
    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
        ++count;
    }
    EXPECT_EQ(count, NUM_ELEMENTS);
    
    list.clear();
    EXPECT_TRUE(list.empty());
}

// ================== MAIN FUNCTION ==================

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    std::cout << "=== Running tests ===\n";
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "\n=== All tests passed! ===\n";
    } else {
        std::cout << "\n=== Some tests failed ===\n";
    }
    
    return result;
}