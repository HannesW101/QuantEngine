// Same project headers.
#include "Core/DataFetcher.h"
#include "Core/ConfigManager.h"
// 3rd party headers.
#include <curl/curl.h>
#include <nlohmann/json.hpp>
// std headers.
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>

namespace QuantEngine {
    namespace {
        // Handles incoming data from CURL requests
        // Appends received data to the provided string buffer
        size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
            size_t totalSize = size * nmemb;
            output->append(static_cast<char*>(contents), totalSize);
            return totalSize;
        }

        // Executes HTTP GET request with 10-second timeout
        // Throws error if CURL initialization fails or request times out
        std::string httpGet(const std::string& url) {
            CURL* curl = curl_easy_init();
            std::string response;

            if (curl) {
                // Configure CURL options
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

                // Execute request and check for errors
                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
                }

                curl_easy_cleanup(curl);
                return response;
            }
            else {
                throw std::runtime_error("Failed to initialize CURL");
            }
        }

        // Computes annualized volatility from price history
        // Uses 30-day window by default (adjustable via days parameter)
        // Assumes 252 trading days/year for annualization
        double calculateHistoricalVolatility(const std::vector<double>& prices, int days = 30) {
            if (prices.size() < 2) {
                throw std::runtime_error("Not enough price data to calculate volatility");
            }

            // Convert prices to daily logarithmic returns
            std::vector<double> logReturns;
            for (size_t i = 1; i < prices.size(); ++i) {
                double returnValue = std::log(prices[i] / prices[i - 1]);
                logReturns.push_back(returnValue);
            }

            // Calculate statistical variance of returns
            double sum = 0.0;
            for (double ret : logReturns) {
                sum += ret;
            }
            double mean = sum / logReturns.size();

            double variance = 0.0;
            for (double ret : logReturns) {
                variance += std::pow(ret - mean, 2);
            }
            variance /= (logReturns.size() - 1);

            // Annualize the standard deviation
            return std::sqrt(variance * 252);
        }
    }

    // Initialize static API key storage (configured elsewhere)
    const std::string DataFetcher::API_KEY = "";

    // Retrieve Alpha Vantage API key from configuration
    std::string DataFetcher::getApiKey() {
        return ConfigManager::getInstance().getApiKey("alpha_vantage");
    }

    // Retrieve FRED API key from configuration
    std::string DataFetcher::getFredApiKey() {
        return ConfigManager::getInstance().getApiKey("fred");
    }

    // Get current risk-free rate from FRED's 3-month T-Bill data
    // Uses 5% fallback if data unavailable
    double DataFetcher::fetchRiskFreeRate() {
        std::string fred_api_key = getFredApiKey();
        std::string url = "https://api.stlouisfed.org/fred/series/observations?series_id=DTB3&api_key=" +
            fred_api_key +
            "&file_type=json&sort_order=desc&limit=1";

        std::string response = httpGet(url);
        nlohmann::json data = nlohmann::json::parse(response);

        // Parse latest rate from JSON response
        if (data.contains("observations") && !data["observations"].empty()) {
            std::string valueStr = data["observations"][0]["value"].get<std::string>();
            if (valueStr != ".") {
                return std::stod(valueStr) / 100.0;  // Convert percentage to decimal
            }
        }

        return 0.05; // Default if data missing
    }

    // Fetch 30 days of price data and compute volatility
    // Implements retry logic for API rate limits
    // Uses 30% fallback if data unavailable
    double DataFetcher::fetchHistoricalVolatility(const std::string& symbol, const std::string& apiKey) {
        std::string url = "https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=" +
            symbol + "&apikey=" + apiKey + "&outputsize=compact";

        std::string response = httpGet(url);
        nlohmann::json data = nlohmann::json::parse(response);

        // Handle API rate limiting
        if (data.contains("Note") || data.contains("Error Message")) {
            if (data.contains("Note") && data["Note"].get<std::string>().find("API call frequency") != std::string::npos) {
                std::this_thread::sleep_for(std::chrono::seconds(15));  // Wait before retry
                response = httpGet(url);
                data = nlohmann::json::parse(response);
            }

            if (data.contains("Note") || data.contains("Error Message")) {
                return 0.30; // Default volatility
            }
        }

        // Extract closing prices from JSON response
        if (!data.contains("Time Series (Daily)")) {
            throw std::runtime_error("Invalid response format from Alpha Vantage");
        }

        auto timeSeries = data["Time Series (Daily)"];
        std::vector<double> closingPrices;
        int count = 0;

        // Collect most recent 30 closing prices
        for (auto it = timeSeries.begin(); it != timeSeries.end() && count < 30; ++it, ++count) {
            double closePrice = std::stod(it.value()["4. close"].get<std::string>());
            closingPrices.push_back(closePrice);
        }

        return calculateHistoricalVolatility(closingPrices);
    }

    // Main data aggregation method
    // Combines real-time price, historical volatility, and risk-free rate
    // Implements fallback values for failed data components
    DataFetcher::StockData DataFetcher::fetchStockData(const std::string& symbol) {
        std::string apiKey = getApiKey();
        StockData result;

        // Get real-time price
        std::string url = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=" +
            symbol + "&apikey=" + apiKey;

        std::string response = httpGet(url);
        nlohmann::json data = nlohmann::json::parse(response);

        if (!data.contains("Global Quote") || data["Global Quote"].empty()) {
            throw std::runtime_error("Failed to fetch stock data for " + symbol);
        }

        result.spotPrice = std::stod(data["Global Quote"]["05. price"].get<std::string>());

        // Get volatility with fallback
        try {
            result.volatility = fetchHistoricalVolatility(symbol, apiKey);
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: Could not calculate volatility: " << e.what() << std::endl;
            result.volatility = 0.30; // Default
        }

        // Get risk-free rate with fallback
        try {
            result.riskFreeRate = fetchRiskFreeRate();
        }
        catch (const std::exception& e) {
            std::cerr << "Warning: Could not fetch risk-free rate: " << e.what() << std::endl;
            result.riskFreeRate = 0.05; // Default
        }

        return result;
    }
} // namespace QuantEngine