// Same project headers.
#include "PricingEngines/BlackScholesEngine.h"
// 3rd party headers.
// ....
// std headers.
#include <cmath>

namespace QuantEngine {
    template<typename T>
    T BlackScholesEngine<T>::calculatePrice(const Instrument<T>& instrument, const MarketData<T>& marketData) const {
        // Extract contract parameters from the instrument
        const auto& params = instrument.getParameters();
        T S = params.spotPrice_;   // Current stock price
        T K = params.strike_;      // Option strike price
        T maturity = params.maturity_; // Time until expiration (years)

        // Retrieve market conditions for pricing
        T r = marketData.getRiskFreeRate(maturity);    // Risk-free rate
        T sigma = marketData.getVolatility(K, maturity); // Volatility

        // Calculate Black-Scholes intermediate terms
        T d1 = this->d1(S, K, r, sigma, maturity);
        T d2 = this->d2(S, K, r, sigma, maturity);

        // Compute call/put price using Black-Scholes formula
        if (params.isCall_) {
            // Call formula: S*N(d1) - K*e^(-rT)*N(d2)
            return S * N(d1) - K * std::exp(-r * maturity) * N(d2);
        }
        else {
            // Put formula: K*e^(-rT)*N(-d2) - S*N(-d1)
            return K * std::exp(-r * maturity) * N(-d2) - S * N(-d1);
        }
    }

    template<typename T>
    std::unique_ptr<PricingEngine<T>> BlackScholesEngine<T>::clone() const {
        // Create independent copy of the pricing engine
        return std::make_unique<BlackScholesEngine<T>>(*this);
    }

    // Black-Scholes d1 calculation
    // Represents (ln(S/K) + (r + sigma^2/2)T) / (sigma*sqrt(T))
    template<typename T>
    T BlackScholesEngine<T>::d1(T S, T K, T r, T sigma, T T) const {
        return (std::log(S / K) + (r + 0.5 * sigma * sigma) * T)
            / (sigma * std::sqrt(T));
    }

    // Black-Scholes d2 calculation
    // Simplified as d1 - sigma*sqrt(T)
    template<typename T>
    T BlackScholesEngine<T>::d2(T S, T K, T r, T sigma, T T) const {
        return d1(S, K, r, sigma, T) - sigma * std::sqrt(T);
    }

    // Normal cumulative distribution function (CDF)
    // Approximated using error function
    template<typename T>
    T BlackScholesEngine<T>::N(T x) const {
        return 0.5 * (1 + std::erf(x / std::sqrt(2)));
    }

    // Normal probability density function (PDF)
    // theta(x) = (1/sqrt((2*pi))e^(-x^2/2))
    template<typename T>
    T BlackScholesEngine<T>::N_prime(T x) const {
        return (1.0 / std::sqrt(2.0 * 3.14159265358979323846))
            * std::exp(-0.5 * x * x);
    }

    // Risk sensitivity calculations (Greeks)
    template<typename T>
    std::map<std::string, T> BlackScholesEngine<T>::calculateGreeks(
        const Instrument<T>& instrument, const MarketData<T>& marketData) const {
        const auto& params = instrument.getParameters();
        const T S = params.spotPrice_;
        const T K = params.strike_;
        const T maturity = params.maturity_;
        const bool isCall = params.isCall_;

        // Get market data parameters
        const T r = marketData.getRiskFreeRate(maturity);
        const T sigma = marketData.getVolatility(K, maturity);

        // Pre-calculate common values
        const T d1_val = d1(S, K, r, sigma, maturity);
        const T d2_val = d2(S, K, r, sigma, maturity);
        const T discountFactor = std::exp(-r * maturity);
        const T n_prime = N_prime(d1_val);

        std::map<std::string, T> greeks;

        // Delta: Price sensitivity to underlying price
        greeks["delta"] = isCall ? N(d1_val) : N(d1_val) - T(1);

        // Gamma: Delta's sensitivity to underlying price
        greeks["gamma"] = n_prime / (S * sigma * std::sqrt(maturity));

        // Vega: Price sensitivity to volatility (per 1% change)
        greeks["vega"] = S * std::sqrt(maturity) * n_prime * T(0.01);

        // Theta: Price sensitivity to time (daily decay)
        T theta;
        if (isCall) {
            theta = (-(S * sigma * n_prime) / (2 * std::sqrt(maturity))
                - r * K * discountFactor * N(d2_val)) / T(365);
        }
        else {
            theta = (-(S * sigma * n_prime) / (2 * std::sqrt(maturity))
                + r * K * discountFactor * N(-d2_val)) / T(365);
        }
        greeks["theta"] = theta;

        // Rho: Price sensitivity to interest rates (per 1% change)
        if (isCall) {
            greeks["rho"] = K * maturity * discountFactor * N(d2_val) * T(0.01);
        }
        else {
            greeks["rho"] = -K * maturity * discountFactor * N(-d2_val) * T(0.01);
        }

        return greeks;
    }

    // Generate template implementations for common numeric types
    template class BlackScholesEngine<double>;
    template class BlackScholesEngine<float>;
}