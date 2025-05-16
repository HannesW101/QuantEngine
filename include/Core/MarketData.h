// Ensures the headers are only included once.
#pragma once

// Same project headers.
// ....
// 3rd party headers.
// ....
// std headers.
#include <map>
#include <vector>

namespace QuantEngine {
    // Stores current market conditions needed for pricing financial instruments  
    // Handles interest rates and volatility data  
    template<typename T>
    class MarketData {
    public:
        // Records interest rate for a specific time period (e.g., 0.5 years = 6 months)  
        void addRiskFreeRate(T time, T rate);

        // Saves volatility for specific price targets and expiration dates  
        void addVolatility(T strike, T maturity, T volatility);

        // Estimates interest rate for any time using stored data  
        T getRiskFreeRate(T time) const;

        // Finds volatility for specific price/expiration combination  
        T getVolatility(T strike, T maturity) const;

    private:
        // Time-based interest rate storage  
        // Format: {2.0 years -> 3.5% rate}  
        std::map<T, T> yield_curve_;

        // Volatility storage by price target and expiration  
        // Format: {110 strike, 1-year maturity -> 25% volatility}  
        std::map<std::pair<T, T>, T> vol_surface_;

        // All saved price targets (kept sorted for quick access)  
        std::vector<T> strikes_;

        // All saved expiration times (kept in order for quick access)  
        std::vector<T> maturities_;
    };
}
