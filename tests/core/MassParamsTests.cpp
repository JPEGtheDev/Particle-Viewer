// Include glad first to avoid OpenGL header conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include "MassParams.hpp"
#include "settingsIO.hpp"

class MassParamsTest : public ::testing::Test
{
};

TEST_F(MassParamsTest, AggregateInit_SetsAllFields)
{
    // Arrange / Act
    MassParams mp{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};

    // Assert
    EXPECT_EQ(mp.fraction_earth_mass_of_body1, 1.0);
    EXPECT_EQ(mp.fraction_earth_mass_of_body2, 2.0);
    EXPECT_EQ(mp.fraction_fe_body1, 3.0);
    EXPECT_EQ(mp.fraction_si_body1, 4.0);
    EXPECT_EQ(mp.fraction_fe_body2, 5.0);
    EXPECT_EQ(mp.fraction_si_body2, 6.0);
    EXPECT_EQ(mp.mass_of_earth, 7.0);
}

TEST_F(MassParamsTest, FromSettingsIO_DefaultSettingsIO_ReturnsZeroParams)
{
    // Arrange — default SettingsIO has no stats file loaded; mass-fraction fields
    // must be zero-initialised by the default constructor (not left uninitialised).
    SettingsIO sio; // zero-arg constructor

    // Act
    MassParams mp = MassParams::fromSettingsIO(sio);

    // Assert — well-defined zero values, not garbage from uninitialised memory
    EXPECT_EQ(mp.fraction_earth_mass_of_body1, 0.0);
    EXPECT_EQ(mp.fraction_earth_mass_of_body2, 0.0);
    EXPECT_EQ(mp.fraction_fe_body1, 0.0);
    EXPECT_EQ(mp.fraction_si_body1, 0.0);
    EXPECT_EQ(mp.fraction_fe_body2, 0.0);
    EXPECT_EQ(mp.fraction_si_body2, 0.0);
    EXPECT_EQ(mp.mass_of_earth, 0.0);
}

TEST_F(MassParamsTest, FromSettingsIO_PopulatesAllFields)
{
    constexpr double kSettingsIOSentinel = 100.0; // SettingsIO fallback when file not found

    // Arrange — 3-arg constructor with non-existent path sets all mass fields to 100
    // (the else-branch fallback in SettingsIO sets sentinel value 100 for all fields)
    SettingsIO sio("/nonexistent/pos", "/nonexistent/stats", "/nonexistent/com");

    // Act
    MassParams mp = MassParams::fromSettingsIO(sio);

    // Assert
    EXPECT_EQ(mp.fraction_earth_mass_of_body1, kSettingsIOSentinel);
    EXPECT_EQ(mp.fraction_earth_mass_of_body2, kSettingsIOSentinel);
    EXPECT_EQ(mp.fraction_fe_body1, kSettingsIOSentinel);
    EXPECT_EQ(mp.fraction_si_body1, kSettingsIOSentinel);
    EXPECT_EQ(mp.fraction_fe_body2, kSettingsIOSentinel);
    EXPECT_EQ(mp.fraction_si_body2, kSettingsIOSentinel);
    EXPECT_EQ(mp.mass_of_earth, kSettingsIOSentinel);
}
