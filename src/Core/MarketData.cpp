// Same project headers.
#include "Core/MarketData.h"
// 3rd party headers.
// ....
// std headers.
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>

namespace QuantEngine {
    template<typename T>
    void MarketData<T>::addRiskFreeRate(T time, T rate) {
        // Validate input parameters before storage
        if (time < 0 || rate < 0) {
            throw std::invalid_argument("Invalid time/rate");
        }
        // Store time-to-rate mapping for yield curve
        yield_curve_[time] = rate;
    }

    template<typename T>
    void MarketData<T>::addVolatility(T strike, T maturity, T volatility) {
        // Verify valid financial parameters
        if (strike <= 0 || maturity < 0 || volatility < 0) {
            throw std::invalid_argument("Invalid strike/maturity/volatility");
        }

        // Store volatility point in surface map
        vol_surface_[{strike, maturity}] = volatility;

        // Maintain sorted strike prices for efficient lookup
        auto sk_it = std::lower_bound(strikes_.begin(), strikes_.end(), strike);
        if (sk_it == strikes_.end() || *sk_it != strike) {
            strikes_.insert(sk_it, strike);
        }

        // Maintain sorted maturities for efficient lookup
        auto mt_it = std::lower_bound(maturities_.begin(), maturities_.end(), maturity);
        if (mt_it == maturities_.end() || *mt_it != maturity) {
            maturities_.insert(mt_it, maturity);
        }
    }

    template<typename T>
    T MarketData<T>::getRiskFreeRate(T time) const {
        // Check for initialized yield curve data
        if (yield_curve_.empty()) {
            throw std::runtime_error("Yield curve is empty");
        }

        auto upper{ yield_curve_.upper_bound(time) };

        // Handle single data point scenario
        if (yield_curve_.size() == 1) {
            return yield_curve_.begin()->second;
        }

        // Handle requests before first or after last data point
        if (upper == yield_curve_.begin()) {
            return upper->second;
        }
        if (upper == yield_curve_.end()) {
            return std::prev(upper)->second;
        }

        // Perform linear interpolation between nearest time points
        auto lower{ std::prev(upper) };
        T t0{ lower->first }, r0{ lower->second };
        T t1{ upper->first }, r1{ upper->second };

        // Prevent division by zero for duplicate time entries
        if (t1 - t0 < std::numeric_limits<T>::epsilon()) {
            return r0;
        }

        // Calculate weighted average between neighboring rates
        T alpha{ (time - t0) / (t1 - t0) };
        return r0 + alpha * (r1 - r0);
    }

    template<typename T>
    T MarketData<T>::getVolatility(T strike, T maturity) const {
        // First check for exact match in volatility surface
        auto exact_it{ vol_surface_.find({ strike, maturity }) };
        if (exact_it != vol_surface_.end()) {
            return exact_it->second;
        }

        // Handle single-point volatility surface
        if (strikes_.size() == 1 && maturities_.size() == 1) {
            return vol_surface_.at({ strikes_[0], maturities_[0] });
        }

        // Validate surface state for interpolation
        if (strikes_.empty() || maturities_.empty()) {
            throw std::runtime_error("Volatility surface not initialized");
        }
        if (strikes_.size() < 2 || maturities_.size() < 2) {
            throw std::runtime_error("Insufficient data for interpolation");
        }

        // Verify requested strike is within known range
        if (strike < strikes_.front() || strike > strikes_.back()) {
            throw std::runtime_error("Strike out of bounds");
        }

        // Verify requested maturity is within known range
        if (maturity < maturities_.front() || maturity > maturities_.back()) {
            throw std::runtime_error("Maturity out of bounds");
        }

        // Find nearest strike prices for interpolation
        auto k_lower = std::lower_bound(strikes_.begin(), strikes_.end(), strike);
        T k0, k1;
        if (k_lower == strikes_.begin()) {
            k0 = k1 = *k_lower;
        }
        else if (k_lower == strikes_.end()) {
            k0 = k1 = *(k_lower - 1);
        }
        else {
            k0 = *(k_lower - 1);
            k1 = *k_lower;
        }

        // Find nearest maturities for interpolation
        auto t_lower = std::lower_bound(maturities_.begin(), maturities_.end(), maturity);
        T t0, t1;
        if (t_lower == maturities_.begin()) {
            t0 = t1 = *t_lower;
        }
        else if (t_lower == maturities_.end()) {
            t0 = t1 = *(t_lower - 1);
        }
        else {
            t0 = *(t_lower - 1);
            t1 = *t_lower;
        }

        // Handle edge case where point matches grid exactly
        if (k0 == k1 && t0 == t1) {
            return vol_surface_.at({ k0, t0 });
        }

        // Prepare for bilinear interpolation
        bool single_strike{ (k0 == k1) };
        bool single_maturity{ (t0 == t1) };

        // Helper to retrieve volatility with error handling
        auto get_vol = [this](T k, T t) {
            try {
                return vol_surface_.at({ k, t });
            }
            catch (const std::out_of_range&) {
                std::stringstream ss;
                ss << "Missing volatility point at (K=" << k << ", T=" << t << ")";
                throw std::runtime_error(ss.str());
            }
            };

        // Get four surrounding volatility points
        T v00{ get_vol(k0, t0) };
        T v01{ single_maturity ? v00 : get_vol(k0, t1) };
        T v10{ single_strike ? v00 : get_vol(k1, t0) };
        T v11{ (single_strike || single_maturity) ? v00 : get_vol(k1, t1) };

        // Calculate interpolation weights
        T x_ratio = (strike - k0) / (k1 - k0);
        T y_ratio = (maturity - t0) / (t1 - t0);

        // Perform bilinear interpolation
        return (1 - x_ratio) * (1 - y_ratio) * v00 +
            (1 - x_ratio) * y_ratio * v01 +
            x_ratio * (1 - y_ratio) * v10 +
            x_ratio * y_ratio * v11;
    }

    // Explicit template instantiation prevents linker errors
    // Generates concrete implementations for these types
    template class MarketData<double>;
    template class MarketData<float>;
}