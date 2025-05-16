// Same project headers.
#include "Core/Instrument.h"
#include "Instruments/EuropeanStockOption.h"
#include "PricingEngines/BlackScholesEngine.h"
#include "Core/MarketData.h"
#include "Core/DataFetcher.h"
#include "Core/ConfigManager.h"
// 3rd party headers.
// ....
// std headers.
#include <iostream>
#include <memory>
#include <string>

using namespace QuantEngine;

// Generic input handler for different data types
template<typename T>
T getInput(const std::string& prompt) {
    T value;
    std::cout << prompt;
    std::cin >> value;
    return value;
}

// Simple yes/no prompt handler
bool getYesNo(const std::string& prompt) {
    char response;
    std::cout << prompt << " (y/n): ";
    std::cin >> response;
    return (response == 'y' || response == 'Y');
}

int main() {
    try {
        std::cout << "=== European Stock Option Pricing ===" << std::endl;

        // Load API keys from configuration
        ConfigManager::getInstance().loadConfig("../../../config.json");

        // Get stock symbol and fetch current market data
        std::string symbol = getInput<std::string>("Enter option symbol (e.g., AAPL): ");
        DataFetcher fetcher;
        auto stockData = fetcher.fetchStockData(symbol);

        // Display automatically fetched values
        std::cout << "\n=== Fetched Market Data ===" << std::endl;
        std::cout << "Spot price: " << stockData.spotPrice << std::endl;
        std::cout << "Volatility: " << stockData.volatility << std::endl;
        std::cout << "Risk-free rate: " << stockData.riskFreeRate << std::endl;

        // Allow manual override of market data
        if (getYesNo("\nOverride fetched values?")) {
            stockData.spotPrice = getInput<double>("Enter new spot price: ");
            stockData.volatility = getInput<double>("Enter new volatility: ");
            stockData.riskFreeRate = getInput<double>("Enter new risk-free rate: ");
        }

        // Collect remaining contract parameters
        std::cout << "\n=== Option Parameters ===" << std::endl;
        double strike = getInput<double>("Enter strike price: ");
        double maturity = getInput<double>("Enter maturity (years): ");
        double notional = getInput<double>("Enter notional amount: ");
        bool isCall = getYesNo("Is this a call option?");

        // Configure market environment
        MarketData<double> market;
        market.addRiskFreeRate(maturity, stockData.riskFreeRate);
        market.addVolatility(strike, maturity, stockData.volatility);

        // Create option contract with user parameters
        Instrument<double>::Parameters params{
            notional, strike, maturity, stockData.spotPrice, isCall
        };
        auto option = std::make_shared<EuropeanStockOption<double>>(params);

        // Set up pricing calculation engine
        auto engine = std::make_shared<BlackScholesEngine<double>>();
        option->setPricingEngine(engine);
        option->updateMarketData(market);
        option->validate();  // Verify parameters are valid

        // Display pricing results
        std::cout << "\n=== Pricing Results ===" << std::endl;
        std::cout << "Option Price: " << option->price() << std::endl;

        // Display risk sensitivities
        std::cout << "\n=== Greeks ===" << std::endl;
        for (const auto& [greek, value] : option->greeks()) {
            std::cout << greek << ": " << value << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}