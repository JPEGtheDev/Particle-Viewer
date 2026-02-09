# Testing Examples and Patterns

This reference provides concrete examples of correct and incorrect test patterns for Particle-Viewer. Load this when writing or reviewing tests.

---

## AAA Pattern — Correct Examples

### Unit Test: Camera Movement

```cpp
TEST(CameraTest, MoveForward_DefaultSpeed_IncreasesPosition)
{
    // Arrange
    Camera camera(800, 600);
    camera.cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.setSpeed(1.0f);

    // Act
    camera.moveForward();

    // Assert
    EXPECT_LT(camera.cameraPos.z, 0.0f);
}
```

### Unit Test: Constructor with Named Expected Values

```cpp
TEST(ImageTest, Constructor_WithDimensions_SetsWidthAndHeight)
{
    // Arrange
    uint32_t expected_width = 16;
    uint32_t expected_height = 32;

    // Act
    Image image(expected_width, expected_height);

    // Assert
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
}
```

### Unit Test: No Arrange Needed (Omit It)

```cpp
TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Act
    Image image;

    // Assert
    EXPECT_TRUE(image.empty());
}
```

### Visual Regression: Exact Match

```cpp
TEST_F(VisualRegressionTest, ExactMatch_IdenticalSolidImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 255, 0, 0);
    Image current = createTestImage(16, 16, 255, 0, 0);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);

    // Assert
    EXPECT_TRUE(result.matches);
    EXPECT_TRUE(result.error.empty());
}
```

### Visual Regression: Tolerance Match

```cpp
TEST_F(VisualRegressionTest, TolerantMatch_SlightlyDifferentImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 128, 128, 128);
    Image current = createTestImage(16, 16, 129, 127, 128);
    float tolerance = VisualTestConfig::TOLERANT_THRESHOLD;

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, tolerance, false);

    // Assert
    EXPECT_TRUE(result.matches);
    EXPECT_TRUE(result.error.empty());
}
```

### Visual Regression: Detecting Differences

```cpp
TEST_F(VisualRegressionTest, ExactMatch_DifferentImages_GeneratesDiffArtifacts)
{
    // Arrange
    Image baseline = createTestImage(8, 8, 255, 0, 0); // Red
    Image current = createTestImage(8, 8, 0, 255, 0);  // Green

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);

    // Assert
    EXPECT_FALSE(result.matches);
    EXPECT_EQ(result.diff_pixels, 64u);
    EXPECT_GT(result.total_pixels, 0u);
    EXPECT_TRUE(result.diff_image.valid());
}
```

### Integration Test: Data Pipeline

```cpp
TEST(DataLoadingPipelineTest, LoadFrame_ValidData_PopulatesTranslations)
{
    // Arrange
    SettingsIO settings;
    std::string test_file = createTestDataFile(10); // 10 particles
    settings.loadSettings(test_file);
    Particle particle;

    // Act
    bool loaded = particle.loadTranslations(settings.getTranslationData());

    // Assert
    EXPECT_TRUE(loaded);
    EXPECT_EQ(particle.getCount(), 10u);
}
```

### Helper Test: PPM Round-Trip (RGB only, alpha discarded)

```cpp
TEST(VisualHelperTest, LoadImageFromPPM_RoundTrip_PreservesPixels)
{
    // Arrange
    Image original = createTestImage(4, 4, 200, 100, 50);
    std::string path = "/tmp/visual_test_roundtrip.ppm";
    writeImageToPPM(path, original);

    // Act
    Image loaded = loadImageFromPPM(path);

    // Assert
    EXPECT_TRUE(loaded.valid());
    EXPECT_EQ(loaded.width, original.width);
    EXPECT_EQ(loaded.height, original.height);
    EXPECT_EQ(loaded.pixels[0], 200u); // R preserved
    EXPECT_EQ(loaded.pixels[1], 100u); // G preserved
    EXPECT_EQ(loaded.pixels[2], 50u);  // B preserved
    EXPECT_EQ(loaded.pixels[3], 255u); // A set to 255 (alpha discarded in PPM)

    std::remove(path.c_str());
}
```

---

## AAA Pattern — INCORRECT Examples (Do NOT Follow)

### ❌ Combined Arrange & Act

```cpp
// BAD: "Arrange & Act" combined
TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Arrange & Act  <-- WRONG: Don't combine
    Image image;

    // Assert
    EXPECT_TRUE(image.empty());
}
```

**Fix:** Omit `// Arrange` entirely, start with `// Act`.

### ❌ Combined Act & Assert

```cpp
// BAD: "Act & Assert" combined
TEST_F(VisualRegressionTest, ExactMatch_IdenticalImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 255, 0, 0);
    Image current = createTestImage(16, 16, 255, 0, 0);

    // Act & Assert  <-- WRONG: Don't combine
    assertVisualMatch(baseline, current, "test_name");
}
```

**Fix:** Separate into `// Act` (run comparison) and `// Assert` (check result).

### ❌ Inline Expected Values in Assert

```cpp
// BAD: Magic numbers in assert
TEST(ImageTest, Constructor_SetsSize)
{
    // Act
    Image image(16, 32);

    // Assert
    EXPECT_EQ(image.width, 16);   // <-- WRONG: 16 is a magic number
    EXPECT_EQ(image.height, 32);  // <-- WRONG: 32 is a magic number
}
```

**Fix:** Put expected values as named variables in Arrange.

### ❌ Testing External Library Behavior

```cpp
// BAD: Testing std::vector, not our code
TEST(ImageTest, PixelVector_Resizes)
{
    // Arrange
    std::vector<uint8_t> pixels(100);

    // Act
    pixels.resize(200);

    // Assert
    EXPECT_EQ(pixels.size(), 200u);  // <-- WRONG: Testing std::vector
}
```

**Fix:** Only test YOUR code — wrapper logic, integration, or behavior you own.

### ❌ Vague Test Name

```cpp
// BAD: No clear behavior being tested
TEST(CameraTest, WorksProperly)     // <-- WRONG
TEST(ShaderTest, TestShader)        // <-- WRONG
TEST(CameraTest, Update)            // <-- WRONG
```

**Fix:** Use `UnitName_StateUnderTest_ExpectedResult` format.

---

## Key Types Reference

### Image (src/testing/PixelComparator.hpp)

```cpp
struct Image
{
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> pixels; // RGBA, 4 bytes per pixel

    Image();                           // Default: empty
    Image(uint32_t w, uint32_t h);    // Allocates w*h*4 bytes, zeroed
    bool empty() const;
    bool valid() const;
};
```

### ComparisonResult (src/testing/PixelComparator.hpp)

```cpp
struct ComparisonResult
{
    bool matches;
    float similarity;        // 0.0 to 1.0 (1.0 = identical)
    uint32_t diff_pixels;
    uint32_t total_pixels;
    BoundingBox diff_bounds;
    Image diff_image;        // Red overlay on differing pixels
    std::string error;       // Non-empty on error (dimension mismatch, etc.)
};
```

### PixelComparator (src/testing/PixelComparator.hpp)

```cpp
class PixelComparator
{
  public:
    PixelComparator();
    ComparisonResult compare(const Image& baseline, const Image& current,
                            float tolerance, bool generate_diff = false) const;
    void setMode(ComparisonMode mode);  // EXACT or TOLERANT
    ComparisonMode getMode() const;
};
```

### ImageConverter (src/ImageConverter.hpp)

```cpp
class ImageConverter
{
  public:
    ImageConverter(int compression_level = 6);
    ConversionResult convert(const std::string& input, const std::string& output,
                            ImageFormat from, ImageFormat to) const;
    static PpmData parsePPM(const std::string& path);
    static ConversionResult writePNG(const std::string& path, const uint8_t* pixels,
                                     uint32_t width, uint32_t height);
};
```

### Visual Test Helpers (tests/visual-regression/VisualTestHelpers.hpp)

```cpp
// Create solid-color RGBA image
Image createTestImage(uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

// Create horizontal gradient (clamped to [0,255])
Image createGradientImage(uint32_t w, uint32_t h,
                          uint8_t r1, uint8_t g1, uint8_t b1,
                          uint8_t r2, uint8_t g2, uint8_t b2);

// Save RGBA Image as PPM (drops alpha)
bool writeImageToPPM(const std::string& path, const Image& image);

// Save RGBA Image as PNG (drops alpha)
bool writeImageToPNG(const std::string& path, const Image& image);

// Load PPM as RGBA Image (alpha set to 255)
Image loadImageFromPPM(const std::string& path);
```

---

## Test Directory Structure

```
tests/
├── core/               # Unit tests (CameraTests.cpp, ShaderTests.cpp, etc.)
├── integration/        # Multi-component tests
├── testing/            # Tests for PixelComparator, ImageConverter
├── visual-regression/  # Visual comparison tests
│   ├── VisualTestHelpers.hpp
│   └── VisualRegressionTests.cpp
├── mocks/              # MockOpenGL.hpp/.cpp
├── stb_image_write_impl.cpp
└── CMakeLists.txt
```
