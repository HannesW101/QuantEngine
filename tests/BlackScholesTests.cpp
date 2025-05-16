// Same project headers.
#include "PricingEngines/BlackScholesEngine.h"
#include "Core/MarketData.h"
#include "Core/Instrument.h"
// 3rd party headers.
#include "catch2.h"
// std headers.

// Mock class for testing pricing engines without real instruments
// Provides minimal Instrument implementation with configurable parameters
namespace QuantEngine {
    template<typename T>
    class MockEuropeanOption : public Instrument<T> {
    public:
        MockEuropeanOption(T spotPrice, T strike, T maturity, bool isCall)
            : spotPrice_(spotPrice), strike_(strike), maturity_(maturity), isCall_(isCall) {
        }

        // Returns test parameters without validation
        const typename Instrument<T>::Parameters& getParameters() const override {
            static typename Instrument<T>::Parameters params;
            params.spotPrice_ = spotPrice_;
            params.strike_ = strike_;
            params.maturity_ = maturity_;
            params.isCall_ = isCall_;
            return params;
        }

        // Dummy implementations for unused interface methods
        T price() const override { return 0; }
        std::map<std::string, T> greeks() const override { return {}; }
        void updateMarketData(const MarketData<T>&) override {}
        void setPricingEngine(std::shared_ptr<PricingEngine<T>>) override {}
        void validate() const override {}

    private:
        T spotPrice_;
        T strike_;
        T maturity_;
        bool isCall_;
    };
}

// =================================================================
// Pricing TESTS - Verify core Black-Scholes formula implementation
// =================================================================
TEST_CASE("BlackScholesEngine Pricing", "[PricingEngine][BlackScholes]") {
    // Standard test market environment
    QuantEngine::MarketData<double> md;
    md.addRiskFreeRate(1.0, 0.05);      // 5% rate for 1-year maturity
    md.addVolatility(100, 1.0, 0.20);   // 20% volatility at-the-money

    // Test instruments - ATM options
    QuantEngine::MockEuropeanOption<double> callOption(100.0, 100.0, 1.0, true);
    QuantEngine::BlackScholesEngine<double> engine;

    SECTION("Valid Call Price Calculation") {
        // Expected value from Black-Scholes formula for:
        // S=100, K=100, T=1, r=5%, sigma=20%
        CHECK(engine.calculatePrice(callOption, md) == Approx(10.45).margin(0.01));
    }

    SECTION("Put Option Price") {
        // Put-Call parity validation
        QuantEngine::MockEuropeanOption<double> putOption(100.0, 100.0, 1.0, false);
        CHECK(engine.calculatePrice(putOption, md) == Approx(5.57).margin(0.01));
    }

    SECTION("Zero Volatility Handling") {
        // With 0% volatility, put value should equal intrinsic value (0 for ATM)
        QuantEngine::MockEuropeanOption<double> putOption(100.0, 100.0, 1.0, false);
        md.addVolatility(100, 1.0, 0.0);
        CHECK(engine.calculatePrice(putOption, md) == Approx(0.0));
    }

    SECTION("Invalid Market Data") {
        // Verify error handling for missing market data
        QuantEngine::MarketData<double> emptyMd;
        REQUIRE_THROWS_AS(engine.calculatePrice(callOption, emptyMd), std::runtime_error);
    }
}

// =================================================================
// Greeks TESTS - Validate risk sensitivity calculations
// =================================================================
TEST_CASE("BlackScholesEngine Greeks Calculation", "[PricingEngine][Greeks][BlackScholes]") {
    // Standard test environment
    QuantEngine::MarketData<double> md;
    md.addRiskFreeRate(1.0, 0.05);       // 5% annual rate
    md.addVolatility(100, 1.0, 0.20);    // 20% volatility

    QuantEngine::BlackScholesEngine<double> engine;
    const double tol = 0.001;  // 0.1% tolerance for approximations

    SECTION("Call Option Greeks") {
        QuantEngine::MockEuropeanOption<double> call(100.0, 100.0, 1.0, true);
        const auto greeks = engine.calculateGreeks(call, md);

        // Reference values from financial calculator
        CHECK(greeks.at("delta") == Approx(0.6368).epsilon(tol));  // N(d1)
        CHECK(greeks.at("gamma") == Approx(0.01876).epsilon(0.001)); // Gamma formula
        CHECK(greeks.at("vega") == Approx(0.3752).epsilon(tol));   // Sensitivity to 1% vol change
        CHECK(greeks.at("theta") == Approx(-0.0176).epsilon(0.01)); // Daily time decay
        CHECK(greeks.at("rho") == Approx(0.5327).epsilon(tol));    // Sensitivity to 1% rate change
    }

    SECTION("Put Option Greeks") {
        QuantEngine::MockEuropeanOption<double> put(100.0, 100.0, 1.0, false);
        const auto greeks = engine.calculateGreeks(put, md);

        // Verify put-specific Greek calculations
        CHECK(greeks.at("delta") == Approx(-0.3632).epsilon(tol));  // Call delta - 1
        CHECK(greeks.at("gamma") == Approx(0.01876).epsilon(tol));  // Same gamma as call
        CHECK(greeks.at("vega") == Approx(0.3752).epsilon(tol));    // Same vega as call
        CHECK(greeks.at("theta") == Approx(-0.00454).margin(0.00001)); // Different theta formula
        CHECK(greeks.at("rho") == Approx(-0.4189).epsilon(tol));    // Negative rate sensitivity
    }

    SECTION("Extreme Volatility Handling") {
        // Test behavior with 100% volatility
        QuantEngine::MarketData<double> highVolMd;
        highVolMd.addRiskFreeRate(1.0, 0.05);
        highVolMd.addVolatility(100, 1.0, 1.0);

        QuantEngine::MockEuropeanOption<double> call(100.0, 100.0, 1.0, true);
        const auto greeks = engine.calculateGreeks(call, highVolMd);

        // Verify vega decreases with higher volatility (convexity)
        CHECK(greeks.at("vega") == Approx(0.3429).epsilon(0.001));
    }
}

// =================================================================
// TEMPLATE TYPE TESTS - Verify numeric type support
// =================================================================
TEMPLATE_TEST_CASE("BlackScholesEngine Template Support", "[PricingEngine][Templates]", float, double) {
    // Test with both float and double precision
    QuantEngine::MarketData<TestType> md;
    md.addRiskFreeRate(1.0, static_cast<TestType>(0.05));
    md.addVolatility(100, 1.0, static_cast<TestType>(0.20));

    QuantEngine::MockEuropeanOption<TestType> option(100, 100, 1.0, true);
    QuantEngine::BlackScholesEngine<TestType> engine;

    // Verify consistent results across numeric types
    CHECK(engine.calculatePrice(option, md) ==
        Approx(static_cast<TestType>(10.45)).margin(0.01));
}