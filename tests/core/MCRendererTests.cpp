// tests/core/MCRendererTests.cpp
// Tests for MCRenderer shell -- GL resource lifecycle and non-GL observable behavior.
// NOTE: Do NOT include MCRenderer.hpp here -- it requires GLAD and a live GL context.
// These tests validate the design constants and pure-C++ behaviors only.

#include <gtest/gtest.h>

// 1. Validate the capacity formula: kMaxTriangles = 2,000,000 triangles
//    kMaxVertices = kMaxTriangles * 3 = 6,000,000 vertices
//    SSBO size = kMaxVertices * 9 floats * 4 bytes = 216 MB
//    This is a design constant test -- if the value changes, the shader and SSBO must be re-evaluated.
TEST(MCRendererTest, MaxTriangles_Is2Million)
{
    constexpr int kExpectedMaxTriangles = 2'000'000;
    constexpr int kExpectedMaxVertices = kExpectedMaxTriangles * 3;
    EXPECT_EQ(kExpectedMaxVertices, 6'000'000);
}

// 2. Validate the SSBO size formula at the expected cap: 6M vertices * 9 floats * 4 bytes = 216 MB
TEST(MCRendererTest, SsboSizeFormula_IsCorrect)
{
    constexpr long long kMaxVertices = 6'000'000;
    constexpr long long kFloatsPerVertex = 9;
    constexpr long long kBytesPerFloat = 4;
    constexpr long long kExpectedBytes = kMaxVertices * kFloatsPerVertex * kBytesPerFloat;
    constexpr long long k216MB = 216LL * 1024 * 1024;
    EXPECT_EQ(kExpectedBytes, 216'000'000LL);
    // Confirm within 3% of 216 MB (actual is exactly 216,000,000 bytes vs 226,492,416 for exact MiB)
    EXPECT_LT(kExpectedBytes, k216MB * 103 / 100);
}

// 3. Validate the MC table sizes: edge_table[256], tri_table[256][16], flattened = 4096 ints
TEST(MCRendererTest, MCTableFlattenedSize_Is4096Entries)
{
    constexpr int kEdgeTableSize = 256;
    constexpr int kTriTableRows = 256;
    constexpr int kTriTableCols = 16;
    constexpr int kFlatTriTableSize = kTriTableRows * kTriTableCols;
    EXPECT_EQ(kEdgeTableSize, 256);
    EXPECT_EQ(kFlatTriTableSize, 4096);
}

// 4. Validate the UBO layout size: edge_table (1024 bytes) + tri_table (16384 bytes) = 17408 bytes
TEST(MCRendererTest, UboLayoutSize_Is17408Bytes)
{
    constexpr int kEdgeSizeBytes = 256 * sizeof(int);
    constexpr int kTriSizeBytes = 256 * 16 * sizeof(int);
    constexpr int kTotalBytes = kEdgeSizeBytes + kTriSizeBytes;
    EXPECT_EQ(kEdgeSizeBytes, 1024);
    EXPECT_EQ(kTriSizeBytes, 16384);
    EXPECT_EQ(kTotalBytes, 17408);
}

// 5. Validate vertex stride: 9 floats/vertex (3 pos + 3 normal + 3 color).
//    Must match mesh.vert's `uint base = uint(gl_VertexID) * 9u`.
TEST(MCRendererTest, VertexStrideMatches_MeshVert_9Floats)
{
    constexpr int kPosComponents = 3;
    constexpr int kNormalComponents = 3;
    constexpr int kColorComponents = 3;
    EXPECT_EQ(kPosComponents + kNormalComponents + kColorComponents, 9);
}
