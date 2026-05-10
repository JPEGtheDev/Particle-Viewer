/*
 * MockCOMProviderTests.cpp
 *
 * Contract tests for MockCOMProvider — verifies the mock correctly implements
 * the ICOMProvider interface: preset values are returned on hit, out parameter
 * is left unchanged on miss, and frame isolation is enforced.
 */

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "MockCOMProvider.hpp"

TEST(MockCOMProviderTest, UnsetFrame_ReturnsFalse)
{
    MockCOMProvider provider;
    glm::vec3 out{99.0f, 99.0f, 99.0f};

    bool result = provider.getCOM(0, out);

    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(out.x, 99.0f);
    EXPECT_FLOAT_EQ(out.y, 99.0f);
    EXPECT_FLOAT_EQ(out.z, 99.0f);
}

TEST(MockCOMProviderTest, SetFrame_ReturnsTrue)
{
    MockCOMProvider provider;
    provider.setCOM(5, glm::vec3{1.0f, 2.0f, 3.0f});
    glm::vec3 out{0.0f, 0.0f, 0.0f};

    bool result = provider.getCOM(5, out);

    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(out.x, 1.0f);
    EXPECT_FLOAT_EQ(out.y, 2.0f);
    EXPECT_FLOAT_EQ(out.z, 3.0f);
}

TEST(MockCOMProviderTest, UnsetFrame_AfterOtherFrameSet_ReturnsFalse)
{
    MockCOMProvider provider;
    provider.setCOM(5, glm::vec3{1.0f, 2.0f, 3.0f});
    glm::vec3 out{7.0f, 7.0f, 7.0f};

    bool result = provider.getCOM(99, out);

    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(out.x, 7.0f);
    EXPECT_FLOAT_EQ(out.y, 7.0f);
    EXPECT_FLOAT_EQ(out.z, 7.0f);
}
