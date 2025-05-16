// Same project headers.
#include "Instruments/EuropeanStockOption.h"
#include "PricingEngines/BlackScholesEngine.h"
#include "Core/MarketData.h"
// 3rd party headers.
#include "catch2.h"
// std headers.
#include <memory>


// =================================================================
// Construction TESTS - Validate parameter validation during object creation
// =================================================================

TEST_CASE("EuropeanStockOption Construction and Validation", "[EuropeanStockOption][Validation]") {
    // Standard valid parameters for testing
    const QuantEngine::Instrument<double>::Parameters validParams{
        1.0,    // Notional
        100.0,  // Strike
        1.0,    // Maturity (years)
        100.0,  // Spot price
        true    // Call option
    };

    SECTION("Valid parameters construction") {
        // Verify valid parameters don't throw exceptions
        REQUIRE_NOTHROW(QuantEngine::EuropeanStockOption<double>(validParams));
    }

    SECTION("Invalid parameter validation") {
        // Test negative strike price rejection
        QuantEngine::Instrument<double>::Parameters invalidParams = validParams;
        invalidParams.strike_ = -100.0;
        REQUIRE_THROWS_AS(QuantEngine::EuropeanStockOption<double>(invalidParams),
            std::invalid_argument);
    }
}

// =================================================================
// Pricing from engine TESTS - Verify full pricing workflow integration
// =================================================================

TEST_CASE("EuropeanStockOption Pricing Workflow", "[EuropeanStockOption][Pricing]") {
    // Standard test contract parameters
    const QuantEngine::Instrument<double>::Parameters params{
        1.0,    // Notional multiplier
        100.0,  // At-the-money strike
        1.0,    // 1-year maturity
        100.0,  // Current stock price
        true    // Call option
    };

    QuantEngine::EuropeanStockOption<double> option(params);
    auto engine = std::make_shared<QuantEngine::BlackScholesEngine<double>>();
    QuantEngine::MarketData<double> md;

    // Configure test market environment
    md.addRiskFreeRate(1.0, 0.05);     // 5% annual rate
    md.addVolatility(100.0, 1.0, 0.2); // 20% volatility

    SECTION("Price calculation") {
        // Full workflow test: engine setup -> market data -> pricing
        option.setPricingEngine(engine);
        option.updateMarketData(md);

        REQUIRE_NOTHROW(option.validate());  // Pre-pricing validation
        const double price = option.price();
        CHECK(price == Approx(10.45).margin(0.1));  // Expected Black-Scholes value
    }

    SECTION("Unconfigured engine handling") {
        // Verify error when pricing without engine
        REQUIRE_THROWS_AS(option.price(), std::runtime_error);
    }
}

// =================================================================
// Parameters TESTS - Verify parameter storage and retrieval
// =================================================================

TEST_CASE("EuropeanStockOption Parameter Integrity", "[EuropeanStockOption][Parameters]") {
    // Non-standard parameters for testing
    const QuantEngine::Instrument<double>::Parameters params{
        500000.0,   // Large notional
        150.0,      // Out-of-the-money strike
        0.5,        // 6-month maturity
        145.0,      // Below-strike spot price
        false       // Put option
    };

    const QuantEngine::EuropeanStockOption<double> option(params);
    const auto& retrievedParams = option.getParameters();

    SECTION("Parameter storage accuracy") {
        // Verify all parameters are stored and retrieved correctly
        CHECK(retrievedParams.strike_ == 150.0);        // Strike check
        CHECK(retrievedParams.maturity_ == 0.5);        // Maturity check
        CHECK(retrievedParams.spotPrice_ == 145.0);     // Spot price check
        CHECK(retrievedParams.isCall_ == false);         // Option type check
    }
}

// =================================================================
// Template TESTS - Verify numeric type support (float/double)
// =================================================================

TEMPLATE_TEST_CASE("EuropeanStockOption Template Support", "[Instrument][Templates]", float, double) {
    using namespace QuantEngine;  // Local namespace for test clarity

    // Type-generic test parameters
    const typename Instrument<TestType>::Parameters params{
        static_cast<TestType>(1.0),   // Notional
        static_cast<TestType>(100.0), // Strike
        static_cast<TestType>(1.0),   // Maturity
        static_cast<TestType>(100.0), // Spot
        true                         // Call
    };

    MarketData<TestType> md;
    md.addRiskFreeRate(static_cast<TestType>(1.0),
        static_cast<TestType>(0.05));
    md.addVolatility(static_cast<TestType>(100.0),
        static_cast<TestType>(1.0),
        static_cast<TestType>(0.2));

    EuropeanStockOption<TestType> option(params);
    auto engine = std::make_shared<BlackScholesEngine<TestType>>();

    SECTION("Basic Pricing") {
        // Verify consistent pricing across numeric types
        option.setPricingEngine(engine);
        option.updateMarketData(md);

        const auto price = option.price();
        const TestType expected = static_cast<TestType>(10.45);

        // Different precision tolerances for float vs double
        if constexpr (std::is_same_v<TestType, float>) {
            CHECK(price == Approx(expected).margin(0.1f));  // 10% tolerance for float
        }
        else {
            CHECK(price == Approx(expected).margin(0.01)); // 1% tolerance for double
        }
    }
}