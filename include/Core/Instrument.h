// Ensures the headers are only included once.
#pragma once

// Same project headers.
// ....
// 3rd party headers.
// ....
// std headers.
#include <map>
#include <memory>
#include <string>

// Forward declare classes to avoid header dependencies
// Allows using MarketData/PricingEngine types without full definitions here
namespace QuantEngine {
    template<typename T> class MarketData;
    template<typename T> class PricingEngine;
}

namespace QuantEngine {
    // Base class for all financial instruments
    // Derived classes must implement pricing and risk calculations
    template<typename T>
    class Instrument {
    public:
        // Allows proper cleanup of derived class objects
        virtual ~Instrument() = default;

        // Calculates current instrument value
        virtual T price() const = 0;

        // Computes risk metrics (delta, gamma, etc.)
        virtual std::map<std::string, T> greeks() const = 0;

        // Updates with latest market conditions
        virtual void updateMarketData(const MarketData<T>& market) = 0;

        // Sets calculation method (e.g., Monte Carlo vs analytic formulas)
        virtual void setPricingEngine(std::shared_ptr<PricingEngine<T>> engine) = 0;

        // Verifies instrument parameters are valid
        virtual void validate() const = 0;

        // Common parameters for financial contracts
        struct Parameters {
            T notional_;    // Total contract value
            T strike_;      // Option exercise price
            T maturity_;    // Time until expiration (years)
            T spotPrice_;    // Current underlying asset price
            bool isCall_;    // Call option = true, put option = false
        };

        // Gets contract terms and conditions
        virtual const Parameters& getParameters() const = 0;

    protected:
        // Can only be created through derived classes
        Instrument() = default;
    };
}