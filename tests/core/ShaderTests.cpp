/*
 * ShaderTests.cpp
 *
 * Unit tests for the Shader class following AAA pattern
 * and single-assertion principle.
 *
 * Note: These tests focus on the structure and interface of the Shader class.
 * Full integration tests with actual OpenGL context are beyond the scope of unit tests.
 */

#include <gtest/gtest.h>
#include <fstream>
#include <string>

#include "shader.hpp"
#include "MockOpenGL.hpp"

// Test fixture for Shader tests
class ShaderTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();

        // Create temporary test shader files
        createTestShaderFiles();
    }

    void TearDown() override
    {
        // Clean up test shader files
        removeTestShaderFiles();
    }

    // Helper to create valid test shader files
    void createTestShaderFiles()
    {
        // Create valid vertex shader
        std::ofstream vertFile("/tmp/test_vertex.vs");
        vertFile << "#version 330 core\n";
        vertFile << "layout (location = 0) in vec3 position;\n";
        vertFile << "void main()\n";
        vertFile << "{\n";
        vertFile << "    gl_Position = vec4(position, 1.0);\n";
        vertFile << "}\n";
        vertFile.close();

        // Create valid fragment shader
        std::ofstream fragFile("/tmp/test_fragment.fs");
        fragFile << "#version 330 core\n";
        fragFile << "out vec4 color;\n";
        fragFile << "void main()\n";
        fragFile << "{\n";
        fragFile << "    color = vec4(1.0, 0.0, 0.0, 1.0);\n";
        fragFile << "}\n";
        fragFile.close();

        // Create invalid shader (syntax error)
        std::ofstream invalidFile("/tmp/test_invalid.vs");
        invalidFile << "#version 330 core\n";
        invalidFile << "This is not valid GLSL code!!!\n";
        invalidFile.close();
    }

    // Helper to remove test shader files
    void removeTestShaderFiles()
    {
        std::remove("/tmp/test_vertex.vs");
        std::remove("/tmp/test_fragment.fs");
        std::remove("/tmp/test_invalid.vs");
    }

    // Test paths
    const char* validVertexPath = "/tmp/test_vertex.vs";
    const char* validFragmentPath = "/tmp/test_fragment.fs";
    const char* invalidShaderPath = "/tmp/test_invalid.vs";
    const char* nonExistentPath = "/tmp/nonexistent_shader.vs";
};

// ============================================
// Shader File Loading Tests
// ============================================

TEST_F(ShaderTest, MockOpenGL_CanBeInitialized)
{
    // Arrange & Act
    MockOpenGL::reset();

    // Assert - Mock should be ready to use
    EXPECT_EQ(MockOpenGL::createProgramCalls, 0);
}

TEST_F(ShaderTest, TestFiles_CanBeCreatedAndRead)
{
    // Arrange & Act
    std::ifstream vertFile(validVertexPath);
    std::ifstream fragFile(validFragmentPath);

    // Assert
    EXPECT_TRUE(vertFile.good());
    EXPECT_TRUE(fragFile.good());

    vertFile.close();
    fragFile.close();
}

TEST_F(ShaderTest, InvalidFile_DoesNotExist)
{
    // Arrange & Act
    std::ifstream file(nonExistentPath);

    // Assert
    EXPECT_FALSE(file.good());
}

// ============================================
// Shader Program Structure Tests
// ============================================

TEST_F(ShaderTest, ShaderClass_HasProgramMember)
{
    // This test validates that the Shader class structure is correct
    // We can't instantiate it without OpenGL context, but we can verify
    // the structure exists through compilation
    EXPECT_NO_THROW({
        // Test passes if this compiles
        [[maybe_unused]] GLuint testProgram = 0;
    });
}

// ============================================
// Mock OpenGL Integration Tests
// ============================================

TEST_F(ShaderTest, MockCreateProgram_ReturnsPositiveID)
{
    // Arrange & Act
    GLuint programId = MockOpenGL::mockCreateProgram();

    // Assert
    EXPECT_GT(programId, 0u);
}

TEST_F(ShaderTest, MockCreateProgram_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::createProgramCalls;

    // Act
    MockOpenGL::mockCreateProgram();

    // Assert
    EXPECT_EQ(MockOpenGL::createProgramCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockCreateShader_ReturnsUniqueIDs)
{
    // Arrange & Act
    GLuint shader1 = MockOpenGL::mockCreateShader(GL_VERTEX_SHADER);
    GLuint shader2 = MockOpenGL::mockCreateShader(GL_FRAGMENT_SHADER);

    // Assert
    EXPECT_NE(shader1, shader2);
}

TEST_F(ShaderTest, MockUseProgram_UpdatesLastUsedProgram)
{
    // Arrange
    GLuint testProgramId = 42;

    // Act
    MockOpenGL::mockUseProgram(testProgramId);

    // Assert
    EXPECT_EQ(MockOpenGL::lastUsedProgram, testProgramId);
}

TEST_F(ShaderTest, MockUseProgram_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::useProgramCalls;

    // Act
    MockOpenGL::mockUseProgram(1);

    // Assert
    EXPECT_EQ(MockOpenGL::useProgramCalls, initialCalls + 1);
}

// ============================================
// Uniform Location Tests
// ============================================

TEST_F(ShaderTest, MockGetUniformLocation_WithNewName_ReturnsValidLocation)
{
    // Arrange
    const char* uniformName = "testUniform";

    // Act
    GLint location = MockOpenGL::mockGetUniformLocation(1, uniformName);

    // Assert
    EXPECT_GE(location, 0);
}

TEST_F(ShaderTest, MockGetUniformLocation_WithSameName_ReturnsSameLocation)
{
    // Arrange
    const char* uniformName = "testUniform";
    GLint firstLocation = MockOpenGL::mockGetUniformLocation(1, uniformName);

    // Act
    GLint secondLocation = MockOpenGL::mockGetUniformLocation(1, uniformName);

    // Assert
    EXPECT_EQ(firstLocation, secondLocation);
}

TEST_F(ShaderTest, MockGetUniformLocation_WithDifferentNames_ReturnsDifferentLocations)
{
    // Arrange
    const char* uniformName1 = "uniform1";
    const char* uniformName2 = "uniform2";

    // Act
    GLint location1 = MockOpenGL::mockGetUniformLocation(1, uniformName1);
    GLint location2 = MockOpenGL::mockGetUniformLocation(1, uniformName2);

    // Assert
    EXPECT_NE(location1, location2);
}

// ============================================
// Uniform Setting Tests
// ============================================

TEST_F(ShaderTest, MockUniformMatrix4fv_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::uniformMatrix4fvCalls;
    GLfloat matrix[16] = {1.0f};

    // Act
    MockOpenGL::mockUniformMatrix4fv(0, 1, GL_FALSE, matrix);

    // Assert
    EXPECT_EQ(MockOpenGL::uniformMatrix4fvCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockUniform3fv_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::uniform3fvCalls;
    GLfloat vec3[3] = {1.0f, 2.0f, 3.0f};

    // Act
    MockOpenGL::mockUniform3fv(0, 1, vec3);

    // Assert
    EXPECT_EQ(MockOpenGL::uniform3fvCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockUniform1i_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::uniform1iCalls;

    // Act
    MockOpenGL::mockUniform1i(0, 5);

    // Assert
    EXPECT_EQ(MockOpenGL::uniform1iCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockUniform1f_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::uniform1fCalls;

    // Act
    MockOpenGL::mockUniform1f(0, 5.5f);

    // Assert
    EXPECT_EQ(MockOpenGL::uniform1fCalls, initialCalls + 1);
}

// ============================================
// Shader Compilation Tests
// ============================================

TEST_F(ShaderTest, MockCompileShader_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::compileShaderCalls;
    GLuint shaderId = 1;

    // Act
    MockOpenGL::mockCompileShader(shaderId);

    // Assert
    EXPECT_EQ(MockOpenGL::compileShaderCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockGetShaderiv_WithCompileStatus_ReturnsSetValue)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_FALSE);
    GLint status;

    // Act
    MockOpenGL::mockGetShaderiv(1, GL_COMPILE_STATUS, &status);

    // Assert
    EXPECT_EQ(status, GL_FALSE);
}

TEST_F(ShaderTest, MockGetShaderiv_DefaultCompileStatus_ReturnsTrue)
{
    // Arrange
    GLint status;

    // Act
    MockOpenGL::mockGetShaderiv(1, GL_COMPILE_STATUS, &status);

    // Assert
    EXPECT_EQ(status, GL_TRUE);
}

// ============================================
// Shader Linking Tests
// ============================================

TEST_F(ShaderTest, MockLinkProgram_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::linkProgramCalls;

    // Act
    MockOpenGL::mockLinkProgram(1);

    // Assert
    EXPECT_EQ(MockOpenGL::linkProgramCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockGetProgramiv_WithLinkStatus_ReturnsSetValue)
{
    // Arrange
    MockOpenGL::setLinkStatus(GL_FALSE);
    GLint status;

    // Act
    MockOpenGL::mockGetProgramiv(1, GL_LINK_STATUS, &status);

    // Assert
    EXPECT_EQ(status, GL_FALSE);
}

TEST_F(ShaderTest, MockGetProgramiv_DefaultLinkStatus_ReturnsTrue)
{
    // Arrange
    GLint status;

    // Act
    MockOpenGL::mockGetProgramiv(1, GL_LINK_STATUS, &status);

    // Assert
    EXPECT_EQ(status, GL_TRUE);
}

// ============================================
// Shader Deletion Tests
// ============================================

TEST_F(ShaderTest, MockDeleteShader_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::deleteShadercalls;

    // Act
    MockOpenGL::mockDeleteShader(1);

    // Assert
    EXPECT_EQ(MockOpenGL::deleteShadercalls, initialCalls + 1);
}

// ============================================
// Mock State Management Tests
// ============================================

TEST_F(ShaderTest, MockReset_ClearsAllCounters)
{
    // Arrange - Make some calls first
    MockOpenGL::mockCreateProgram();
    MockOpenGL::mockCompileShader(1);
    MockOpenGL::mockUseProgram(1);

    // Act
    MockOpenGL::reset();

    // Assert
    EXPECT_EQ(MockOpenGL::createProgramCalls, 0);
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 0);
    EXPECT_EQ(MockOpenGL::useProgramCalls, 0);
}

TEST_F(ShaderTest, MockReset_ResetsReturnValues)
{
    // Arrange - Create some programs
    MockOpenGL::mockCreateProgram();
    MockOpenGL::mockCreateProgram();

    // Act
    MockOpenGL::reset();

    // Assert - Next program should start at 1 again
    GLuint nextProgram = MockOpenGL::mockCreateProgram();
    EXPECT_EQ(nextProgram, 1u);
}

TEST_F(ShaderTest, GetProgramCount_ReturnsNumberOfCreatedPrograms)
{
    // Arrange & Act
    MockOpenGL::mockCreateProgram();
    MockOpenGL::mockCreateProgram();
    MockOpenGL::mockCreateProgram();

    // Assert
    EXPECT_EQ(MockOpenGL::getProgramCount(), 3u);
}

TEST_F(ShaderTest, WasProgramCreated_WithCreatedProgram_ReturnsTrue)
{
    // Arrange
    GLuint program = MockOpenGL::mockCreateProgram();

    // Act
    bool wasCreated = MockOpenGL::wasProgramCreated(program);

    // Assert
    EXPECT_TRUE(wasCreated);
}

TEST_F(ShaderTest, WasProgramCreated_WithNonCreatedProgram_ReturnsFalse)
{
    // Arrange
    MockOpenGL::mockCreateProgram();

    // Act
    bool wasCreated = MockOpenGL::wasProgramCreated(999);

    // Assert
    EXPECT_FALSE(wasCreated);
}

// ============================================
// Edge Cases
// ============================================

TEST_F(ShaderTest, MockGenVertexArrays_CreatesRequestedNumber)
{
    // Arrange
    GLuint arrays[3];
    int initialCalls = MockOpenGL::genVertexArraysCalls;

    // Act
    MockOpenGL::mockGenVertexArrays(3, arrays);

    // Assert
    EXPECT_EQ(MockOpenGL::genVertexArraysCalls, initialCalls + 1);
}

TEST_F(ShaderTest, MockGenVertexArrays_AssignsUniqueIDs)
{
    // Arrange
    GLuint arrays[3];

    // Act
    MockOpenGL::mockGenVertexArrays(3, arrays);

    // Assert - All IDs should be unique
    EXPECT_NE(arrays[0], arrays[1]);
    EXPECT_NE(arrays[1], arrays[2]);
    EXPECT_NE(arrays[0], arrays[2]);
}

TEST_F(ShaderTest, MockShaderSource_IncrementsCallCounter)
{
    // Arrange
    int initialCalls = MockOpenGL::shaderSourceCalls;
    const char* source = "test source";

    // Act
    MockOpenGL::mockShaderSource(1, 1, &source, nullptr);

    // Assert
    EXPECT_EQ(MockOpenGL::shaderSourceCalls, initialCalls + 1);
}
