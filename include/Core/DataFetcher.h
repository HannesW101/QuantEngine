// Ensures the headers are only included once.
#pragma once

// Same project headers.
// ....
// 3rd party headers.
// ....
// std headers.
#include <string>
#include <map>
#include <vector>

namespace QuantEngine {
    // Central class for retrieving financial data from external sources
    // All methods are static - no instantiation required
    class DataFetcher {
    public:
        // Container for essential stock market metrics
        // Used to pass data between components
        struct StockData {
            double spotPrice;       // Current market price
            double volatility;      // Measured volatility (IV or HV)
            double riskFreeRate;    // Risk-free rate reference
        };

        // Main interface to get current market data for a stock symbol
        // Symbol format depends on data provider (e.g., "AAPL" or "AAPL.OQ")
        static StockData fetchStockData(const std::string& symbol);

        // ----- Market data utilities -----

        // Returns current risk-free rate (typically 10yr Treasury yield)
        static double fetchRiskFreeRate();

        // Calculates historical volatility for given symbol
        // Requires valid API key for data provider
        static double fetchHistoricalVolatility(const std::string& symbol, const std::string& apiKey);

    private:
        // Internal API key management
        static const std::string API_KEY;      // Primary service API key
        static std::string getApiKey();        // Retrieves encrypted API key
        static std::string getFredApiKey();     // FRED economic data API key
    };
}