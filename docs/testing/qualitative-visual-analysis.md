---
title: "Qualitative Visual Analysis"
description: "Render-and-inspect pattern for debugging rendering bugs -- produces observable evidence the agent can analyze directly instead of relying on pixel math."
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debugging, opengl, rendering, agent]
related:
  - "visual-regression.md"
  - "visual-regression-authoring.md"
  - "../rendering/ssm-reference.md"
  - "../TESTING_STANDARDS.md"
---

# Qualitative Visual Analysis

## The Distinction

| Type | What it answers | What it cannot answer |
|------|----------------|----------------------|
| **Quantitative (algorithmic)** | Did pixels change? Does similarity exceed N%? | Does this look correct? Is the screen blank? Are colors wrong? |
| **Qualitative (visual)** | Does the render look right? What is actually on screen? | Whether a specific pixel differs from baseline by N/255 |

Quantitative tests prove a render has not changed. They do not prove it was correct to begin with, and they cannot tell you why a blank screen is blank.

**The agent knows the math is correct. The screen is still blank.** This is the failure mode quantitative testing cannot catch, and the reason qualitative analysis exists.

## When to Use Qualitative Analysis

Use a qualitative render test when:

- A visual bug is reported and the cause is not obvious from a compiler error or test failure
- Automated tests pass but the rendered output looks wrong
- You are tuning render parameters (blur, depth cull, threshold) and need to compare the effect of different values
- A shader produces no visible output and you need to confirm the pipeline is reaching the fragment stage at all

**The first response to any visual bug report is a qualitative render.** Not code inspection. Not reasoning about what might be wrong. Render the scene. Look at it. Then diagnose.

## Pattern: Render-and-Inspect Test

A qualitative test renders the scene and saves the output as an artifact for visual inspection. Unlike a regression test, there is no baseline and no pass/fail pixel comparison. The test always "passes" in the GTest sense; the value is the image artifact it produces.

```cpp
TEST_F(SSMRenderingTest, SSMQualitative_RealData_DepthCullComparison)
{
    // Skip if data not available
    const std::string data_path = "/path/to/PosAndVel";
    if (!std::filesystem::exists(data_path)) {
        GTEST_SKIP() << "Simulation data not present";
    }

    // Load real data
    Particle particles;
    particles.loadFromBinary(data_path, /*frame=*/360, /*n=*/65536);

    // Render at multiple parameter values
    for (auto [suffix, cull_range] : std::initializer_list<std::pair<const char*, float>>{
             {"cull60", 6.0f}, {"cull20", 2.0f}, {"cull10", 1.0f}, {"cull05", 0.5f}})
    {
        glUniform1f(cull_loc, cull_range);
        renderSSMScene(particles);

        Image img = glContext_.captureFramebuffer();
        img.save("artifacts/ssm_qualitative_" + std::string(suffix) + ".png", ImageFormat::PNG);

        // Report statistics inline so they appear in test output
        auto [coverage, avg_rgb] = analyzeImage(img);
        std::cout << suffix << ": coverage=" << coverage << "% rgb=("
                  << avg_rgb.r << "," << avg_rgb.g << "," << avg_rgb.b << ")\n";
    }
}
```

The agent then reads the saved artifacts using its vision capability and evaluates the renders directly.

## What Qualitative Analysis Catches

Situations where quantitative tests pass but the render is wrong:

- **Blank screen** -- shader compiled, pipeline runs, zero fragments written. Coverage = 0% is immediately visible. No baseline comparison needed.
- **Wrong colors** -- material categories mapping to wrong RGB. Visible from the artifact. Math in the color formula may be correct but the category encoding is wrong.
- **Squircle blobs** -- polynomial kernel produces square artifacts at low weights. Visible in the render; not detectable from pixel statistics alone.
- **Depth bleed-through** -- inner particles visible through outer shell. Side-by-side renders at different depth cull values show the effect of parameter tuning directly.
- **FBO wrap bleed** -- Gaussian blur sampling from opposite screen edge. Visible as a color stripe at the viewport boundary.

## Statistics to Capture Inline

Inline statistics make the qualitative test machine-readable without requiring a baseline:

```cpp
int total = img.width() * img.height();
int covered = 0;
float r_sum = 0, g_sum = 0, b_sum = 0;

for (int i = 0; i < total; ++i) {
    auto px = img.pixel(i);
    if (px.r > 10 || px.g > 10 || px.b > 10) {  // non-black pixel
        ++covered;
        r_sum += px.r; g_sum += px.g; b_sum += px.b;
    }
}

float coverage = 100.0f * covered / total;
float avg_r = covered > 0 ? r_sum / covered : 0.0f;
// ...
```

Coverage below 1% means the pipeline is not producing output. RGB averages reveal material color bias.

## Real-World Example

SSM `u_depth_cull_range` tuning (session e5c79d53):

The agent ran one qualitative test with four depth cull values and printed coverage and RGB averages for each:

```
cull60: coverage=41.2%  rgb=(0.61, 0.41, 0.39)   -- Fe/Si heavily blended
cull20: coverage=39.8%  rgb=(0.58, 0.43, 0.37)   -- improved separation
cull10: coverage=38.1%  rgb=(0.57, 0.45, 0.36)   -- near-optimal
cull05: coverage=37.9%  rgb=(0.57, 0.45, 0.36)   -- statistically equivalent to cull10
```

This data determined that `blobRadius * 0.5` was the production value in a single test run. No human review of intermediate renders was needed for the parameter decision. Human review happened once, at the end, to confirm the chosen parameter looked correct.

## Relationship to Quantitative Tests

Qualitative and quantitative tests are complementary, not competing:

- **Qualitative**: diagnose new bugs, tune parameters, confirm the render pipeline is alive, investigate "blank screen" failures. Run during development; artifacts are not committed.
- **Quantitative (regression)**: protect against regressions after a render is confirmed correct. Baselines require human approval before commit.

Qualitative analysis leads. Quantitative regression testing follows.

## Agent Workflow

1. User reports visual bug.
2. **First action**: write and run a qualitative render test that reproduces the reported condition. Save the artifact.
3. Use vision to read the artifact. State what is visible.
4. Form a hypothesis about root cause based on what is seen, not what the code says.
5. Make one change. Re-run the qualitative test. Compare artifacts.
6. Once the render looks correct: write the quantitative regression baseline (with human approval).

Step 2 is mandatory before step 4. Code reading is not a substitute for producing a render.
