#include <gtest/gtest.h>

// temporal function for temporary
int calAdd(int a, int b) {
    return a + b;
}

// tests
TEST(calAddTest, calAddTestExample) {

    std::cout << "Test is executed " << std::endl;
    EXPECT_EQ(calAdd(1, 2), 3);
    EXPECT_EQ(calAdd(-1, 2), 1);
    EXPECT_EQ(calAdd(0, 0), 0);
}
