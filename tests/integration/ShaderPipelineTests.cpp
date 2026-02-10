/*
 * ShaderPipelineTests.cpp
 *
 * Integration tests for the shader compilation pipeline:
 * Load shader source → compile vertex+fragment → link program
 *
 * These tests verify the shader loading and compilation workflow
 * using mocked OpenGL functions to avoid requiring a GPU.
 */

#include <cstdio>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "MockOpenGL.hpp"
#include "shader.hpp"

// Test fixture for shader pipeline integration tests
class ShaderPipelineTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD(); // Use centralized mock initialization for Windows compatibility
        createTestShaderFiles();
    }

    void TearDown() override
    {
        removeTestShaderFiles();
    }

    // Create test shader files with file open verification
    void createTestShaderFiles()
    {
        // Valid vertex shader
        std::ofstream vertFile("/tmp/integration_vertex.vs");
        if (!vertFile.is_open()) {
            FAIL() << "Failed to create test vertex shader file";
        }
        vertFile << "#version 330 core\n";
        vertFile << "layout (location = 0) in vec3 position;\n";
        vertFile << "uniform mat4 model;\n";
        vertFile << "uniform mat4 view;\n";
        vertFile << "uniform mat4 projection;\n";
        vertFile << "void main()\n";
        vertFile << "{\n";
        vertFile << "    gl_Position = projection * view * model * vec4(position, 1.0);\n";
        vertFile << "}\n";
        vertFile.close();

        // Valid fragment shader
        std::ofstream fragFile("/tmp/integration_fragment.frag");
        if (!fragFile.is_open()) {
            FAIL() << "Failed to create test fragment shader file";
        }
        fragFile << "#version 330 core\n";
        fragFile << "out vec4 FragColor;\n";
        fragFile << "uniform vec3 objectColor;\n";
        fragFile << "void main()\n";
        fragFile << "{\n";
        fragFile << "    FragColor = vec4(objectColor, 1.0);\n";
        fragFile << "}\n";
        fragFile.close();

        // Minimal vertex shader
        std::ofstream minVertFile("/tmp/integration_minimal.vs");
        if (!minVertFile.is_open()) {
            FAIL() << "Failed to create test minimal vertex shader file";
        }
        minVertFile << "#version 330 core\n";
        minVertFile << "void main() { gl_Position = vec4(0.0); }\n";
        minVertFile.close();

        // Minimal fragment shader
        std::ofstream minFragFile("/tmp/integration_minimal.frag");
        if (!minFragFile.is_open()) {
            FAIL() << "Failed to create test minimal fragment shader file";
        }
        minFragFile << "#version 330 core\n";
        minFragFile << "out vec4 color;\n";
        minFragFile << "void main() { color = vec4(1.0); }\n";
        minFragFile.close();
    }

    void removeTestShaderFiles()
    {
        std::remove("/tmp/integration_vertex.vs");
        std::remove("/tmp/integration_fragment.frag");
        std::remove("/tmp/integration_minimal.vs");
        std::remove("/tmp/integration_minimal.frag");
    }

    const char* validVertexPath = "/tmp/integration_vertex.vs";
    const char* validFragmentPath = "/tmp/integration_fragment.frag";
    const char* minimalVertexPath = "/tmp/integration_minimal.vs";
    const char* minimalFragmentPath = "/tmp/integration_minimal.frag";
};

// ============================================
// Shader Compilation Pipeline Tests
// ============================================

TEST_F(ShaderPipelineTest, ShaderConstruction_CreatesProgram)
{
    // Arrange: Set up successful compilation/linking
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act: Create a shader from files
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: A program was created
    EXPECT_EQ(MockOpenGL::createProgramCalls, 1);
    EXPECT_GT(shader.Program, 0u);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_CreatesBothShaders)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Both vertex and fragment shaders were created
    EXPECT_EQ(MockOpenGL::createShaderCalls, 2);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_LoadsSourceFromFiles)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Source was loaded for both shaders
    EXPECT_EQ(MockOpenGL::shaderSourceCalls, 2);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_CompilesBothShaders)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Both shaders were compiled
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 2);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_AttachesShadersToProgram)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Both shaders were attached to the program
    EXPECT_EQ(MockOpenGL::attachShaderCalls, 2);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_LinksProgram)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Program was linked
    EXPECT_EQ(MockOpenGL::linkProgramCalls, 1);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_DeletesIntermediateShaders)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    Shader shader(validVertexPath, validFragmentPath);

    // Assert: Intermediate shaders were cleaned up
    EXPECT_EQ(MockOpenGL::deleteShaderCalls, 2);
}

// ============================================
// Shader Use Pipeline Tests
// ============================================

TEST_F(ShaderPipelineTest, ShaderUse_ActivatesProgram)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);
    Shader shader(validVertexPath, validFragmentPath);
    GLuint programId = shader.Program;

    // Act
    shader.Use();

    // Assert: Program was activated
    EXPECT_EQ(MockOpenGL::useProgramCalls, 1);
    EXPECT_EQ(MockOpenGL::lastUsedProgram, programId);
}

TEST_F(ShaderPipelineTest, ShaderUse_MultipleUses_ActivatesCorrectProgram)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);
    Shader shader1(validVertexPath, validFragmentPath);
    Shader shader2(minimalVertexPath, minimalFragmentPath);

    // Act & Assert: Use shader1
    shader1.Use();
    EXPECT_EQ(MockOpenGL::lastUsedProgram, shader1.Program);

    // Act & Assert: Use shader2
    shader2.Use();
    EXPECT_EQ(MockOpenGL::lastUsedProgram, shader2.Program);

    // Act & Assert: Use shader1 again
    shader1.Use();
    EXPECT_EQ(MockOpenGL::lastUsedProgram, shader1.Program);
}

// ============================================
// Full Pipeline Flow Tests
// ============================================

TEST_F(ShaderPipelineTest, FullPipeline_FilesToUse_CompletesSuccessfully)
{
    // Arrange: Set up successful compilation
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act: Full pipeline from file loading to shader use
    Shader shader(validVertexPath, validFragmentPath);
    shader.Use();

    // Assert: All pipeline stages completed
    EXPECT_EQ(MockOpenGL::createProgramCalls, 1);
    EXPECT_EQ(MockOpenGL::createShaderCalls, 2);
    EXPECT_EQ(MockOpenGL::shaderSourceCalls, 2);
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 2);
    EXPECT_EQ(MockOpenGL::attachShaderCalls, 2);
    EXPECT_EQ(MockOpenGL::linkProgramCalls, 1);
    EXPECT_EQ(MockOpenGL::deleteShaderCalls, 2);
    EXPECT_EQ(MockOpenGL::useProgramCalls, 1);
    EXPECT_GT(shader.Program, 0u);
}

TEST_F(ShaderPipelineTest, FullPipeline_MultipleShaders_IndependentPrograms)
{
    // Arrange
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act: Create multiple shaders
    Shader shader1(validVertexPath, validFragmentPath);
    Shader shader2(minimalVertexPath, minimalFragmentPath);

    // Assert: Each shader has its own program
    EXPECT_NE(shader1.Program, shader2.Program);
    EXPECT_EQ(MockOpenGL::createProgramCalls, 2);
}

// ============================================
// Error Handling Tests
// ============================================

TEST_F(ShaderPipelineTest, ShaderConstruction_WithCompileFailure_ContinuesExecution)
{
    // Arrange: Set up compilation failure
    MockOpenGL::setCompileStatus(GL_FALSE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act: Create shader (should print error but not crash)
    EXPECT_NO_THROW({ Shader shader(validVertexPath, validFragmentPath); });

    // Assert: Pipeline attempted all stages
    EXPECT_EQ(MockOpenGL::compileShaderCalls, 2);
}

TEST_F(ShaderPipelineTest, ShaderConstruction_WithLinkFailure_ContinuesExecution)
{
    // Arrange: Set up link failure
    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_FALSE);

    // Act: Create shader (should print error but not crash)
    EXPECT_NO_THROW({ Shader shader(validVertexPath, validFragmentPath); });

    // Assert: Link was attempted
    EXPECT_EQ(MockOpenGL::linkProgramCalls, 1);
}

// ============================================
// Shader File Reading Tests
// ============================================

TEST_F(ShaderPipelineTest, ShaderConstruction_ReadsFileContents)
{
    // Arrange: Create a shader file with specific content
    std::ofstream testFile("/tmp/integration_test_content.vs");
    testFile << "#version 330\n";
    testFile << "// Test shader content\n";
    testFile << "void main() {}\n";
    testFile.close();

    MockOpenGL::setCompileStatus(GL_TRUE);
    MockOpenGL::setLinkStatus(GL_TRUE);

    // Act
    EXPECT_NO_THROW({ Shader shader("/tmp/integration_test_content.vs", minimalFragmentPath); });

    // Cleanup
    std::remove("/tmp/integration_test_content.vs");

    // Assert: Source was loaded (shader source call was made)
    EXPECT_GE(MockOpenGL::shaderSourceCalls, 1);
}

// ============================================
// Default Constructor Tests
// ============================================

TEST_F(ShaderPipelineTest, DefaultConstructor_DoesNotCreateProgram)
{
    // Act
    Shader shader;

    // Assert: No OpenGL calls made
    EXPECT_EQ(MockOpenGL::createProgramCalls, 0);
}
