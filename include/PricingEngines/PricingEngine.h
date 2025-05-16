// Ensures the headers are only included once.
#pragma once

// Same project headers.
#include "Core/Instrument.h"
#include "Core/MarketData.h"
// 3rd party headers.
// ....
// std headers.
#include <stdexcept>

namespace QuantEngine {
    // Base class for all pricing calculation methods  
    // Defines interface for derivative valuation engines  
    template<typename T>
    class PricingEngine {
    public:
        // Allows safe deletion of derived engine objects  
        virtual ~PricingEngine() = default;

        // Main pricing interface - must be implemented by all engines  
        // Combines instrument details and market data for valuation  
        virtual T calculatePrice(const Instrument<T>& instrument,
            const MarketData<T>& marketData) const = 0;

        // Optional risk calculation interface  
        // Not all engines support Greek calculations  
        // Throws error by default if not implemented  
        virtual std::map<std::string, T> calculateGreeks(const Instrument<T>& instrument,
            const MarketData<T>& marketData) const {
            throw std::runtime_error("Greeks calculation not implemented for this engine");
        }

        // Creates independent copy of the pricing engine  
        // Essential for thread-safe operations and engine presets  
        virtual std::unique_ptr<PricingEngine<T>> clone() const = 0;
    };
}