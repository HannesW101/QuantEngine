// Same project headers.
#include "Core/MarketData.h"
// 3rd party headers.
#include "catch2.h"
// std headers.

// =================================================================
// RISK-FREE RATE TESTS - Verify yield curve management and interpolation
// =================================================================
TEST_CASE("Risk-Free Rate Management", "[MarketData][YieldCurve]") {
    QuantEngine::MarketData<double> md;

    SECTION("Valid Rate Additions") {
        // Verify basic storage and retrieval of rates
        md.addRiskFreeRate(0.5, 0.02);  // 2% for 6 months
        md.addRiskFreeRate(1.0, 0.03);  // 3% for 1 year

        // Check exact matches
        REQUIRE(md.getRiskFreeRate(0.5) == 0.02);
        REQUIRE(md.getRiskFreeRate(1.0) == 0.03);
    }

    SECTION("Rate Interpolation") {
        // Test linear interpolation between points
        md.addRiskFreeRate(0.5, 0.02);
        md.addRiskFreeRate(1.0, 0.03);

        // Verify midpoint calculation
        CHECK(md.getRiskFreeRate(0.75) == Approx(0.025));
        // Check before first point returns first rate
        CHECK(md.getRiskFreeRate(0.25) == Approx(0.02));
        // Check after last point returns last rate
        CHECK(md.getRiskFreeRate(2.0) == Approx(0.03));
    }

    SECTION("Single Rate Edge Case") {
        // Test behavior with only one rate point
        md.addRiskFreeRate(1.0, 0.03);

        // All time queries return the single rate
        CHECK(md.getRiskFreeRate(0.5) == 0.03);
        CHECK(md.getRiskFreeRate(2.0) == 0.03);
    }

    SECTION("Invalid Input Handling") {
        // Verify validation of negative values
        REQUIRE_THROWS_AS(md.addRiskFreeRate(-0.5, 0.02), std::invalid_argument);
        REQUIRE_THROWS_AS(md.addRiskFreeRate(1.0, -0.01), std::invalid_argument);
    }

    SECTION("Empty Yield Curve") {
        // Ensure error when no rates are available
        REQUIRE_THROWS_AS(md.getRiskFreeRate(0.5), std::runtime_error);
    }
}

// =================================================================
// VOLATILITY SURFACE TESTS - Verify volatility grid management
// =================================================================
TEST_CASE("Volatility Surface Management", "[MarketData][VolSurface]") {
    QuantEngine::MarketData<double> md;

    SECTION("Valid Volatility Additions") {
        // Basic storage verification
        md.addVolatility(100, 1.0, 0.20);  // ATM 1-year vol
        md.addVolatility(150, 2.0, 0.25);  // OTM 2-year vol

        REQUIRE(md.getVolatility(100, 1.0) == 0.20);
        REQUIRE(md.getVolatility(150, 2.0) == 0.25);
    }

    SECTION("Exact Match Retrieval") {
        // Test multiple maturities for same strike
        md.addVolatility(100, 1.0, 0.20);  // 1-year
        md.addVolatility(100, 2.0, 0.25);  // 2-year

        CHECK(md.getVolatility(100, 1.0) == 0.20);
        CHECK(md.getVolatility(100, 2.0) == 0.25);
    }

    SECTION("Bilinear Interpolation") {
        // Create 2x2 grid for interpolation testing
        md.addVolatility(100, 1.0, 0.20);  // K=100, T=1
        md.addVolatility(100, 2.0, 0.25);  // K=100, T=2
        md.addVolatility(150, 1.0, 0.22);  // K=150, T=1
        md.addVolatility(150, 2.0, 0.28);  // K=150, T=2

        // Test center point interpolation
        CHECK(md.getVolatility(125, 1.5) == Approx(0.2375));

        // Verify boundary checks
        CHECK_THROWS_AS(md.getVolatility(90, 1.5), std::runtime_error);  // Low strike
        CHECK_THROWS_AS(md.getVolatility(125, 0.5), std::runtime_error);  // Low maturity
    }

    SECTION("Single Point Surface") {
        // Test flat volatility surface behavior
        md.addVolatility(100, 1.0, 0.20);

        CHECK(md.getVolatility(100, 1.0) == 0.20);
        CHECK(md.getVolatility(120, 1.5) == 0.20);  // Extrapolate single point
    }

    SECTION("Invalid Input Handling") {
        // Verify parameter validation
        REQUIRE_THROWS_AS(md.addVolatility(-100, 1.0, 0.2), std::invalid_argument);
        REQUIRE_THROWS_AS(md.addVolatility(100, -1.0, 0.2), std::invalid_argument);
        REQUIRE_THROWS_AS(md.addVolatility(100, 1.0, -0.2), std::invalid_argument);
    }

    SECTION("Missing Data Handling") {
        // Test error handling for out-of-bounds queries
        md.addVolatility(100, 1.0, 0.20);
        md.addVolatility(150, 1.0, 0.22);

        REQUIRE_THROWS_AS(md.getVolatility(200, 1.0), std::runtime_error);  // High strike
        REQUIRE_THROWS_AS(md.getVolatility(100, 3.0), std::runtime_error);  // High maturity
    }
}

// =================================================================
// TEMPLATE TYPE TESTS - Verify numeric precision support
// =================================================================
TEMPLATE_TEST_CASE("MarketData Template Type Support", "[MarketData][Templates]", float, double) {
    QuantEngine::MarketData<TestType> md;
    const TestType strike = 100;
    const TestType maturity = 1.0;
    const TestType vol = 0.2;

    SECTION("Type-Specific Operations") {
        // Test basic operations with template types
        md.addRiskFreeRate(maturity, 0.03);
        md.addVolatility(strike, maturity, vol);

        REQUIRE(md.getRiskFreeRate(maturity) == Approx(0.03));
        REQUIRE(md.getVolatility(strike, maturity) == Approx(vol));
    }
}

// =================================================================
// STRESS TESTS - Validate performance and edge cases
// =================================================================
TEST_CASE("MarketData Stress Tests", "[MarketData][Stress]") {
    QuantEngine::MarketData<double> md;

    SECTION("Large Dataset Handling") {
        // Test with 1000 rate points
        for (int i = 0; i <= 1000; ++i) {
            md.addRiskFreeRate(i, 0.01 + i * 0.0001);  // Linear rate curve
        }

        // Verify interpolation in dense data
        CHECK(md.getRiskFreeRate(500.5) == Approx(0.01 + 500.5 * 0.0001));

        // Create 100x100 volatility grid
        for (int s = 50; s <= 150; ++s) {
            for (int t = 1; t <= 100; ++t) {
                md.addVolatility(s, t, 0.2 + s * 0.001 + t * 0.002);
            }
        }

        // Verify grid point calculation
        CHECK(md.getVolatility(125, 50) == Approx(0.2 + 125 * 0.001 + 50 * 0.002));
    }

    SECTION("Update Overwrites") {
        // Test data overwrite functionality
        md.addRiskFreeRate(1.0, 0.03);
        md.addRiskFreeRate(1.0, 0.04);  // Overwrite rate
        CHECK(md.getRiskFreeRate(1.0) == 0.04);

        md.addVolatility(100, 1.0, 0.20);
        md.addVolatility(100, 1.0, 0.22);  // Overwrite volatility
        CHECK(md.getVolatility(100, 1.0) == 0.22);
    }
}