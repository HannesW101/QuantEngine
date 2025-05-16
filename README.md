# QuantEngine

QuantEngine is a modern C++ library for quantitative finance, offering a comprehensive framework for pricing and analyzing financial derivatives with a focus on performance, extensibility, and accuracy.

## Overview

This library provides tools for pricing financial instruments, particularly options, using various mathematical models. The current implementation focuses on European-style stock options with Black-Scholes pricing, but the architecture is designed to be extensible for other instrument types and pricing methodologies.

Key features include:
- Template-based design supporting different numeric precision types
- Modular architecture with clear separation of concerns
- Market data management for yield curves and volatility surfaces
- Built-in risk analytics (Greeks calculation)
- Automated data fetching from financial data providers
- Comprehensive test suite using Catch2 framework

## Architecture

QuantEngine follows a layered architecture:

```
┌───────────────────────────────────────────────────────────┐
│                        Applications                       │
│                  (main.cpp, CLI Interface)                │
└─────────────────────────────┬─────────────────────────────┘
                             │
┌─────────────────────────────▼─────────────────────────────┐
│                     Instrument Layer                      │
│           (EuropeanStockOption, SwapContract)            │
└─────────────────────────────┬─────────────────────────────┘
                             │
┌─────────────────────────────▼─────────────────────────────┐
│                    Pricing Engine Layer                   │
│            (BlackScholesEngine, Monte Carlo)             │
└─────────────────────────────┬─────────────────────────────┘
                             │
┌─────────────────────────────▼─────────────────────────────┐
│                      Core Data Layer                      │
│             (MarketData, DataFetcher, Config)            │
└───────────────────────────────────────────────────────────┘
```

### Core Components

1. **Instrument Classes**
   - `Instrument<T>`: Base template class for all financial instruments
   - `EuropeanStockOption<T>`: European option implementation

2. **Pricing Engines**
   - `PricingEngine<T>`: Abstract base class for all pricing algorithms
   - `BlackScholesEngine<T>`: Analytical Black-Scholes model implementation

3. **Market Data**
   - `MarketData<T>`: Stores interest rates and volatilities with interpolation
   - `DataFetcher`: Retrieves real-time financial data from external sources

4. **Configuration**
   - `ConfigManager`: Singleton class for API key and settings management

## Building the Project

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 7+, or MSVC 2019+)
- CMake 3.14 or newer
- Catch2 (for tests)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/your-username/quantengine.git
cd quantengine

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run tests
ctest
```

## Usage Examples

### Basic Option Pricing

```cpp
#include "QuantEngine/EuropeanStockOption.h"
#include "QuantEngine/BlackScholesEngine.h"
#include "QuantEngine/MarketData.h"

using namespace QuantEngine;

// Create option parameters
Instrument<double>::Parameters params{
    1.0,     // notional
    100.0,   // strike price
    1.0,     // maturity (years)
    105.0,   // spot price
    true     // call option
};

// Create market environment
MarketData<double> market;
market.addRiskFreeRate(1.0, 0.05);        // 5% rate
market.addVolatility(100.0, 1.0, 0.20);   // 20% volatility

// Create and configure option
auto option = std::make_shared<EuropeanStockOption<double>>(params);
auto engine = std::make_shared<BlackScholesEngine<double>>();
option->setPricingEngine(engine);
option->updateMarketData(market);

// Calculate price and risk metrics
double price = option->price();
auto greeks = option->greeks();
```

### Using the Command Line Interface

```bash
./quantengine_cli

=== European Stock Option Pricing ===
Enter option symbol (e.g., AAPL): AAPL

=== Fetched Market Data ===
Spot price: 178.42
Volatility: 0.25
Risk-free rate: 0.045

Override fetched values? (y/n): n

=== Option Parameters ===
Enter strike price: 180
Enter maturity (years): 0.5
Enter notional amount: 100
Is this a call option? (y/n): y

=== Pricing Results ===
Option Price: 13.45

=== Greeks ===
delta: 0.5812
gamma: 0.0164
vega: 0.2733
theta: -0.0251
rho: 0.3981
```

## Market Data Integration

QuantEngine can fetch real-time market data using the `DataFetcher` class. To enable this functionality:

1. Obtain API keys from supported financial data providers
2. Create a `config.json` file in the project root:

```json
{
  "api_keys": {
    "alpha_vantage": "YOUR_API_KEY",
    "fred": "YOUR_FRED_API_KEY"
  }
}
```

## Core Classes Documentation

### Instrument Interface

`Instrument<T>` provides the base interface for all financial instruments:

```cpp
template<typename T>
class Instrument {
public:
    virtual ~Instrument() = default;
    virtual T price() const = 0;
    virtual std::map<std::string, T> greeks() const = 0;
    virtual void updateMarketData(const MarketData<T>& market) = 0;
    virtual void setPricingEngine(std::shared_ptr<PricingEngine<T>> engine) = 0;
    virtual void validate() const = 0;
};
```

### MarketData Class

`MarketData<T>` manages financial environment data:

- Interest rate term structure using yield curve
- Volatility surface with strike/maturity dimensions
- Interpolation for missing data points

### PricingEngine Interface

`PricingEngine<T>` defines the calculation strategy:

```cpp
template<typename T>
class PricingEngine {
public:
    virtual ~PricingEngine() = default;
    virtual T calculatePrice(const Instrument<T>& instrument,
        const MarketData<T>& marketData) const = 0;
    virtual std::map<std::string, T> calculateGreeks(...) const;
    virtual std::unique_ptr<PricingEngine<T>> clone() const = 0;
};
```

## Extending the Library

### Adding New Instruments

1. Create a new class derived from `Instrument<T>`
2. Implement all required virtual methods
3. Add appropriate tests

Example for Asian option:

```cpp
template<typename T>
class AsianStockOption : public Instrument<T> {
public:
    AsianStockOption(const typename Instrument<T>::Parameters& params);
    
    // Implement required interface methods
    T price() const override;
    std::map<std::string, T> greeks() const override;
    // ...
};
```

### Adding New Pricing Engines

1. Create a new class derived from `PricingEngine<T>`
2. Implement the calculation algorithms
3. Add tests validating against known benchmarks

Example for Monte Carlo engine:

```cpp
template<typename T>
class MonteCarloEngine : public PricingEngine<T> {
public:
    T calculatePrice(const Instrument<T>& instrument,
        const MarketData<T>& marketData) const override;
    
    // Configure simulation parameters
    void setNumberOfPaths(size_t paths);
    
private:
    size_t numPaths = 10000;
};
```

## Performance Considerations

- Template parameters allow switching between `float` and `double` precision
- Thread-safe design through immutable objects and cloneable engines
- Pre-computation of common values in pricing algorithms

## Testing

The library includes comprehensive tests using the Catch2 framework:

- Unit tests for individual components
- Integration tests for pricing workflows
- Numerical tests against known analytical solutions
- Stress tests for edge cases and large datasets

Run the test suite with:

```bash
ctest -V
```

## Future Roadmap

- Additional instrument types (American options, swaps, futures)
- Alternative pricing models (Heston, SABR, Local Volatility)
- Multi-curve interest rate framework
- GPU acceleration support
- Python bindings
- Calibration utilities

## License

[MIT License](LICENSE)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
