// Ensures the headers are only included once.
#pragma once

// Same project headers.
#include "Core/Instrument.h"
#include "Core/MarketData.h"
#include "PricingEngines/PricingEngine.h"
// 3rd party headers.
// ....
// std headers.

namespace QuantEngine {

    // Concrete implementation for European-style equity options
    // Uses the Instrument interface for pricing functionality
    template<typename T>
    class EuropeanStockOption : public Instrument<T> {
    public:
        // Creates option with specific contract terms
        // Inherits parameters structure from base Instrument class
        explicit EuropeanStockOption(const typename Instrument<T>::Parameters& params);

        // ------ Mandatory Instrument implementations ------

        // Calculate current option value using pricing engine
        T price() const override;

        // Compute risk sensitivities (delta, gamma, vega, etc.)
        std::map<std::string, T> greeks() const override;

        // Refresh market data (rates, volatilities)
        void updateMarketData(const MarketData<T>& market) override;

        // Set calculation method (Monte Carlo/Analytic/etc.)
        void setPricingEngine(std::shared_ptr<PricingEngine<T>> engine) override;

        // Verify contract parameters are logical/valid
        void validate() const override;

        // Get reference to stored contract terms
        const typename Instrument<T>::Parameters& getParameters() const override;

    private:
        typename Instrument<T>::Parameters params_;  // Contract details (strike, maturity, etc.)
        std::shared_ptr<PricingEngine<T>> pricingEngine_;  // Calculation strategy
        MarketData<T> marketData_;  // Current market environment snapshot
    };

}