---
title: "Qualitative Visual Analysis"
description: "The agent must look at what it is rendering. Render-and-inspect is the only way to debug visual output -- the agent uses its vision capability to analyze screenshots directly, the same way a developer would."
domain: testing
subdomain: visual-regression
tags: [testing, visual-regression, debugging, opengl, rendering, agent]
related:
  - "visual-regression.md"
  - "visual-regression-authoring.md"
  - "../rendering/ssm-reference.md"  # SSM attempt/failure post-mortem; real example source
  - "../TESTING_STANDARDS.md"
---

# Qualitative Visual Analysis

## The Agent Must Look

This project renders to a screen. The agent is multimodal -- it can read image files and see what is in them, the same way a developer would look at a screenshot. **This capability must be used.**

When a visual bug is reported, the only valid investigation is to produce a render and look at it. The agent must:

1. Write a test that renders the scene under the reported conditions
2. Save the framebuffer as a PNG artifact
3. **Read the image file and describe what is on screen**
4. Diagnose from what it sees -- not from what the code says

Without step 3, the agent is debugging blind. It may have read every line of shader code and confirmed the math is correct. It still does not know what is on screen. The render is the ground truth. Source code is a hypothesis about what the render will look like.

**If the agent has not looked at a render, it has not investigated the bug.**

---

## Why Source Code Is Not Enough

The agent can read shader code, verify uniform values, trace execution paths, and confirm that every formula is mathematically correct. None of that tells it what pixels the GPU actually wrote. Shader compilation succeeds, draw calls execute, tests pass -- and the screen is blank. Or the blobs are squares. Or iron particles are rendering blue.

The math was correct. The output was wrong. Code reading cannot close that gap.

A developer debugging a rendering issue opens the app and looks at it. The agent must do the same thing: render, capture, look.

---

## The Distinction

| Mode | The agent can answer | The agent cannot answer |
|------|---------------------|------------------------|
| **Reading source code** | Is this formula correct? Does this uniform get set? | What does the render actually look like? |
| **Quantitative (pixel comparison)** | Did this pixel change from baseline? | Is this correct? Is the screen blank? Are the colors wrong? |
| **Qualitative (render + look)** | What is actually on screen? Does this look right? What changed between these two renders? | Whether a specific pixel differs by N/255 from a prior baseline |

Quantitative tests answer "did it change." Qualitative analysis answers "does it look right." Both are necessary. Neither substitutes for the other.

---

## When to Use Qualitative Analysis

- A visual bug is reported -- **this is the first action, not a last resort**
- Automated tests pass but the rendered output looks wrong
- Tuning render parameters (blur radius, depth cull range, threshold) and comparing the effect of different values
- A shader produces no visible output and the agent needs to confirm the pipeline is producing fragments at all
- Any time the agent would otherwise say "the code looks correct" without having seen the render

**The first response to any visual bug report is a qualitative render.** Not code inspection. Not reasoning about what might be wrong. Not confidence that the formula is correct. Render the scene. Read the image. Describe what is visible. Then diagnose.

---

## Pattern: Render-Capture-Look

A qualitative test renders the scene and saves the output. The agent then reads the saved image using its vision capability and states explicitly what it sees. There is no baseline and no pass/fail pixel comparison -- the test always exits successfully in the GTest sense. Its value is the image it produces and the agent's description of that image.

```cpp
TEST_F(SSMRenderingTest, SSMQualitative_RealData_DepthCullComparison)
{
    // Skip if simulation data is not present on this machine
    const std::string data_path = "/path/to/PosAndVel";
    if (!std::filesystem::exists(data_path)) {
        GTEST_SKIP() << "Simulation data not present -- qualitative test requires real data";
    }

    // Load real simulation data (not synthetic test data)
    Particle particles;
    particles.loadFromBinary(data_path, /*frame=*/360, /*n=*/65536);

    // Render at multiple parameter values and save each for inspection
    for (auto [suffix, cull_range] : std::initializer_list<std::pair<const char*, float>>{
             {"cull60", 6.0f}, {"cull20", 2.0f}, {"cull10", 1.0f}, {"cull05", 0.5f}})
    {
        glUniform1f(cull_loc, cull_range);
        renderSSMScene(particles);

        Image img = glContext_.captureFramebuffer();
        img.save("artifacts/ssm_qualitative_" + std::string(suffix) + ".png", ImageFormat::PNG);

        // Print inline statistics -- agent reads these alongside the image
        auto stats = analyzeImage(img);
        std::cout << "[" << suffix << "] coverage=" << stats.coverage_pct << "% "
                  << "rgb=(" << stats.avg_r << "," << stats.avg_g << "," << stats.avg_b << ") "
                  << "fe_dominant=" << stats.fe_px << " si_dominant=" << stats.si_px << "\n";
    }
}
```

After the test runs, the agent reads each saved artifact image and describes what it sees:

> "cull60: The blob surface is partially transparent -- iron core particles are visible through the silicate shell. cull10: The surface is opaque. No interior bleed-through visible. The boundary between the two bodies is sharp."

That description is the diagnosis. It does not come from reading the shader. It comes from looking at the image.

---

## Inline Statistics Supplement Vision, Not Replace It

Print coverage and color statistics to stdout so they appear in the test output alongside the visual. Statistics catch things that are hard to describe verbally (e.g., whether Fe-dominant pixels outnumber Si-dominant pixels by the right ratio). But statistics do not replace looking at the image.

```cpp
int total = img.width() * img.height();
int covered = 0, fe_dominant = 0, si_dominant = 0;
float r_sum = 0, g_sum = 0, b_sum = 0;

for (int y = 0; y < img.height(); ++y) {
    for (int x = 0; x < img.width(); ++x) {
        auto px = img.pixel(x, y);
        if (px.r > 10 || px.g > 10 || px.b > 10) {
            ++covered;
            r_sum += px.r; g_sum += px.g; b_sum += px.b;
            if (px.r > px.b + 30) ++fe_dominant;   // red > blue = iron
            if (px.b > px.r + 30) ++si_dominant;   // blue > red = silicate
        }
    }
}
```

- **Coverage near 0%**: The pipeline is not producing output. Stop and diagnose the render path before examining shader code.
- **Coverage looks right but image is wrong**: The pipeline is alive but the content is incorrect. Read the image to determine what is actually wrong.
- **RGB averages**: Reveal material color bias. Useful for parameter comparisons but insufficient for spatial diagnosis.

The image is always the primary artifact. Statistics are annotations on the image.

---

## What Qualitative Analysis Catches

Situations where quantitative tests pass but the render is wrong:

- **Blank screen** -- shader compiled, pipeline runs, zero fragments written. Coverage = 0%. Cause cannot be determined from source code alone.
- **Wrong colors** -- material categories mapping to wrong RGB. Math in the color formula may be correct but the w-component encoding is wrong. Visible immediately on inspection.
- **Squircle blobs** -- polynomial kernel produces square artifacts at low falloff weights. Not detectable from pixel statistics. Obvious in the render image.
- **Depth bleed-through** -- inner particles visible through outer shell. Side-by-side renders at different depth cull values show the progression directly.
- **FBO wrap bleed** -- Gaussian blur sampling from the opposite screen edge. Visible as a color stripe along a viewport boundary. Cannot be inferred from code.
- **Black star holes** -- blur radius too small for production resolution. Gaps between particles appear as dark voids. Only apparent at the render stage.

---

## Real Example: SSM Depth Cull Tuning

Session e5c79d53 demonstrates both the cost of not using this pattern and the gain from using it.

**Without qualitative analysis (first 15 hours):** The agent read shader code, confirmed formulas, and committed 7 fixes. The user had to report 14 corrections. Every commit was followed immediately by a user saying the bug was still there. The agent was reasoning about what the render should look like. It was not looking at what the render did look like.

**With qualitative analysis (after user demanded it):** One test rendered the scene at four depth cull values. The agent read the output images and the inline statistics:

```
cull60: coverage=41.2%  rgb=(0.61,0.41,0.39)  fe=8821  si=5102  -- iron core visible through shell
cull20: coverage=39.8%  rgb=(0.58,0.43,0.37)  fe=7634  si=5891  -- improved
cull10: coverage=38.1%  rgb=(0.57,0.45,0.36)  fe=7201  si=6034  -- surface opaque, materials separated
cull05: coverage=37.9%  rgb=(0.57,0.45,0.36)  fe=7198  si=6031  -- statistically equivalent to cull10
```

The agent described what it saw in the images: at cull60, the iron core was visible through the silicate surface. At cull10, the surface was fully opaque. The production value `blobRadius * 0.5` was selected from this one test run. Human review happened once, at the end, to confirm the selected parameter looked correct.

The qualitative test produced in one run what 15 hours of code inspection and user corrections could not.

---

## Relationship to Quantitative Tests

Qualitative and quantitative tests are complementary, not competing:

- **Qualitative**: diagnose new bugs, tune parameters, confirm the render pipeline is alive. The agent looks at the output and describes what it sees. Artifacts are not committed.
- **Quantitative (regression)**: protect against regressions after a render has been confirmed correct by a human. Baselines require human approval before commit.

Qualitative analysis leads. Quantitative regression testing follows. Never use a regression test to diagnose a new bug -- it will tell you pixels changed, not what changed or why.

---

## Agent Workflow

1. User reports a visual bug or asks for parameter tuning.
2. **First action -- mandatory:** write and run a qualitative render test that reproduces the reported conditions. Save the framebuffer.
3. **Read the saved image.** Describe explicitly what is on screen: colors, shapes, where the artifact appears, what looks wrong.
4. Form a hypothesis based on what was seen. Not based on what the code says.
5. Make one change. Re-run the qualitative test. Read the new image. Compare what changed.
6. Repeat until the render looks correct.
7. Once correct: write the quantitative regression baseline (with human approval per the visual-regression-testing skill).

Steps 2 and 3 are not optional. An agent that skips them is reasoning about a render it has not seen. That is not debugging -- it is guessing.
