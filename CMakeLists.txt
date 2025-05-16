# Ensures we using CMake 3.8+ features
cmake_minimum_required(VERSION 3.8)
# Set the vcpkg toolchain
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# Names your project and initializes CMake variables
# Sets CMAKE_PROJECT_NAME to "QuantEngine"
# Initializes PROJECT_SOURCE_DIR to the directory containing this file
project(QuantEngine)
# Enforce C++20 for compatibility with Catch2 from vcpkg
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Auto-export symbols
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Dependency management
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# --------------------------------------------
# Core Library
# --------------------------------------------
# Creates a static/shared library from source files
add_library(QuantEngine
  src/Core/MarketData.cpp
  src/Core/ConfigManager.cpp
  src/Core/DataFetcher.cpp 
  src/PricingEngines/BlackScholesEngine.cpp
  src/Instruments/EuropeanStockOption.cpp
)
target_link_libraries(QuantEngine PUBLIC
  CURL::libcurl
  nlohmann_json::nlohmann_json
)
# Tells compiler where to find headers
target_include_directories(QuantEngine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/extern
)
# Create executable
add_executable(QuantEngineApp
  src/CLI/main.cpp
)
target_link_libraries(QuantEngineApp 
  QuantEngine
)
# --------------------------------------------
# Unit Tests
# --------------------------------------------
# Enable testing framework
enable_testing()
# Create test executable with Catch2 main
add_executable(QuantEngineTests
	tests/MarketDataTests.cpp
	tests/EuropeanStockOptionTests
	tests/BlackScholesTests.cpp
	tests/tests_main.cpp
)
# Link test executable to library and Catch2 (in the correct order)
target_link_libraries(QuantEngineTests
  QuantEngine
)