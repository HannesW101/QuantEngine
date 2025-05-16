// Ensures the headers are only included once.
#pragma once

// Same project headers.
#include "PricingEngines/PricingEngine.h"
#include "Core/MarketData.h"
// 3rd party headers.
// ....
// std headers.

namespace QuantEngine {
    // Black-Scholes pricing model implementation for options
    // Concrete engine for analytical option valuation
    template<typename T>
    class BlackScholesEngine : public PricingEngine<T> {
    public:
        // Main pricing calculation using Black-Scholes formula
        T calculatePrice(const Instrument<T>& instrument,
            const MarketData<T>& marketData) const override;

        // Creates copy of engine for safe parallel pricing
        std::unique_ptr<PricingEngine<T>> clone() const override;

        // Computes risk sensitivities (delta, gamma, theta, vega, rho)
        std::map<std::string, T> calculateGreeks(const Instrument<T>& instrument,
            const MarketData<T>& marketData) const override;

    private:
        // Black-Scholes intermediate calculation (d1 term)
        T d1(T S /*spot*/, T K /*strike*/, T r /*rate*/,
            T sigma /*vol*/, T T /*time*/) const;

        // Black-Scholes intermediate calculation (d2 term)
        T d2(T S, T K, T r, T sigma, T T) const;

        // Normal distribution probability (calculates theta(x))
        T N(T x) const;

        // Normal distribution density (calculates theta(x))
        T N_prime(T x) const;
    };
}