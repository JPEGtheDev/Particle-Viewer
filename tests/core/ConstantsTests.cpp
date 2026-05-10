#include <gtest/gtest.h>

#include "constants.hpp"

TEST(kSimToDisplayScale, HasExpectedValue)
{
    // Arrange
    constexpr float expected = 0.25f;

    // Act
    constexpr float actual = kSimToDisplayScale;

    // Assert
    EXPECT_EQ(actual, expected);
}
