// Same project headers.
#include "Instruments/EuropeanStockOption.h"
// 3rd party headers.
// ....
// std headers.
#include <stdexcept>

namespace QuantEngine {

    template<typename T>
    EuropeanStockOption<T>::EuropeanStockOption(const typename Instrument<T>::Parameters& params)
        : params_(params) {
        // Verify contract parameters meet basic validity checks
        validate();
    }

    template<typename T>
    T EuropeanStockOption<T>::price() const {
        // Ensure pricing method is configured before calculation
        if (!pricingEngine_) {
            throw std::runtime_error("Pricing engine not set");
        }
        // Calculate base price and apply contract multiplier
        return pricingEngine_->calculatePrice(*this, marketData_) * params_.notional_;
    }

    template<typename T>
    std::map<std::string, T> EuropeanStockOption<T>::greeks() const {
        // Require pricing engine with Greek calculation support
        if (!pricingEngine_) {
            throw std::runtime_error("Pricing engine not set for European stock option");
        }
        // Delegate risk calculation to pricing engine
        return pricingEngine_->calculateGreeks(*this, marketData_);
    }

    template<typename T>
    void EuropeanStockOption<T>::updateMarketData(const MarketData<T>& market) {
        // Refresh current market conditions (rates, volatilities)
        marketData_ = market;
    }

    template<typename T>
    void EuropeanStockOption<T>::setPricingEngine(std::shared_ptr<PricingEngine<T>> engine) {
        // Set calculation strategy (Monte Carlo, Analytic, etc.)
        pricingEngine_ = engine;
    }

    template<typename T>
    void EuropeanStockOption<T>::validate() const {
        // Verify contract parameters make financial sense
        if (params_.strike_ <= 0) throw std::invalid_argument("Strike price must be positive");
        if (params_.maturity_ <= 0) throw std::invalid_argument("Time to maturity must be positive");
        if (params_.spotPrice_ <= 0) throw std::invalid_argument("Stock spot price must be positive");
        if (params_.notional_ <= 0) throw std::invalid_argument("Contract notional must be positive");
    }

    template<typename T>
    const typename Instrument<T>::Parameters& EuropeanStockOption<T>::getParameters() const {
        // Provide read-only access to contract terms
        return params_;
    }

    // Generate concrete template implementations to prevent linker errors
    template class EuropeanStockOption<double>;
    template class EuropeanStockOption<float>;
}