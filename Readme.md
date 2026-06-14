# VeloFX SDK

A lightweight, high-performance C++20 closed-source SDK wrapper for the **ExchangeRate-API (v6)**. Delivered as a pre-compiled static library exclusively for Windows (MSVC x64) environments, it features an optional binary file-caching layer to protect your API quota, eliminate unnecessary network latency, and secure proprietary integration logic.

---

## 🚀 Features

* **Closed-Source Delivery**: Distributed as a compiled static binary (`.lib`) to safeguard production architecture and deployment tokens.
* **Modern C++ Type Safety**: Robust error handling utilizing `std::variant` and `std::optional` instead of raw pointers or unchecked exceptions.
* **Smart Binary Caching**: Integrated TTL-based file cache that reads and writes raw binary payloads directly to disk for near-instantaneous responses.
* **Rich UI Metadata**: Native access to localized currency profiles, unicode currency symbols, and country flag CDN assets.
* **Zero Dependencies in Public Headers**: Clean public headers with Doxygen-ready IDE tooltips; no third-party JSON or network leakages into your source trees.

---

## 🔑 Obtaining an API Key

To use the VeloFX SDK, you must provision an API key from the data provider:
1. Navigate to the [ExchangeRate-API Portal](https://www.exchangerate-api.com/).
2. Sign up for an account (The Free Tier grants 1,500 requests/month, no credit card required).
3. Copy your unique cryptographically generated key and pass it into the `VeloFX` engine instance.

---

## 💾 Cache Management Modes

The `VeloFX` orchestrator can run in two distinct states depending on how you initialize its constructor:

### Mode 1: Live-Only Mode (No Cache)
```cpp
velofx::VeloFX fx("YOUR_API_KEY"); // cache_provider defaults to nullptr
```
* **Mechanism**: Every method invocation triggers a blocking synchronous HTTP network transaction via LibCurl.
* **Use Case**: Critical financial modules requiring ticking data updates down to the millisecond.
* **Trade-Off**: Higher execution latency (network-bound) and immediate consumption of your monthly API limits.

### Mode 2: File Cache Mode (Recommended)
```cpp
velofx::FileCache cache(2); // Instantiates a binary cache with a 2-hour TTL
velofx::VeloFX fx("YOUR_API_KEY", &cache);
```
* **Mechanism**: The SDK checks a localized binary file on disk first. If a record exists and its internal timestamp is younger than the specified TTL, it is hydrated immediately with **zero network overhead**. If expired or missing, it triggers an HTTP fallback and updates the cache automatically.

---

## 📚 Detailed API Reference

### Data Types & Variants

```cpp
struct ConvertSuccess { 
    double rate;             // Calculated unit exchange rate
    double converted_amount; // Computed final total value
};

struct ConvertError { 
    std::string message;     // Direct API error reason or Curl failure logs
};

using ConvertResult = std::variant<ConvertSuccess, ConvertError>;
```

---

### Core Methods

#### 1. `convert`
```cpp
ConvertResult convert(const Currency& from, const Currency& to, double amount, bool force_refresh = false);
```
Performs fiat conversion calculations between an asset pair.
* **Arguments**:
  * `from`: Originating `Currency` anchor (e.g., `velofx::Currency::EUR`).
  * `to`: Counterparty `Currency` target (e.g., `velofx::Currency::USD`).
  * `amount`: Floating point monetary volume to scale.
  * `force_refresh`: (Optional, defaults to `false`). Set to `true` to deliberately bypass the disk cache for an on-demand network ping.
* **Returns**: `ConvertResult` variant. Evaluate using `std::holds_alternative<velofx::ConvertSuccess>(result)`.

#### 2. `get_rates`
```cpp
std::map<std::string, double> get_rates(const Currency& base);
```
Downloads the complete global exchange grid indexed to the chosen base currency.
* **Arguments**:
  * `base`: Anchor currency filter.
* **Returns**: A standard `std::map` matching 3-letter ISO strings to currency values. Returns an empty map if an unrecoverable network failure occurs. *Note: Caching is bypassed for this call.*

#### 3. `convert_historical`
```cpp
ConvertResult convert_historical(const Currency& from, const Currency& to, double amount, int year, int month, int day);
```
Queries systemic data records to evaluate pairs on an exact date in the past.
* **Arguments**:
  * `from`, `to`, `amount`: Standard exchange specifications.
  * `year`, `month`, `day`: Specific calendar parameters (Full data mapping covers 1990 to the current year).
* **Returns**: `ConvertResult` containing verified historical math configurations or failure structural diagnostics.

#### 4. `get_currency_info`
```cpp
std::optional<CurrencyInfo> get_currency_info(const Currency& currency);
```
Resolves structural assets and naming matrices mapped to localized interface rendering.
* **Arguments**:
  * `currency`: Target token reference.
* **Returns**: An `std::optional<velofx::CurrencyInfo>` containing:
  * `.name`: Extended English literal label (e.g., `"Euro"`).
  * `.symbol`: Typographical Unicode glyph representation (e.g., `"€"`).
  * `.country_name`: Issuing state/union designation (e.g., `"European Union"`).
  * `.flag_url`: Verified CDN pathway serving high-definition flag visuals.

---

## 📦 Repository Layout

```
velofx-sdk/
├── include/
│   └── velofx/
│       └── velofx.h       <-- Public API interface (Pre-documented Doxygen tooltips)
├── lib/
│   └── windows/
│       └── velofx.lib     <-- MSVC Pre-compiled binary (Windows x64 Build)
├── CMakeLists.txt        <-- SDK downstream build descriptor
└── README.md
```

## 🛠️ Integration Guide

This SDK uses an `IMPORTED` target mapping layer. Add it directly into your modern Windows project architecture via CMake `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
    velofx
    GIT_REPOSITORY https://github.com/YOUR_USERNAME/velofx-sdk.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(velofx)

# Bind the imported static binary to your local workspace executable
target_link_libraries(your_target_name PRIVATE velofx)
```

### Build Environment Prerequisites
* **OS**: Windows 10 / 11 (x64 Native Architecture)
* **CMake Engine**: Version 3.24 or higher
* **Toolchain**: Microsoft Visual C++ (MSVC) 2019 or later
* **C++ Standard**: Explicit `/std:c++20` or higher enabled on target
* **System Libraries**: `LibCurl` must be configured globally or available on host development path (e.g., via `vcpkg` or manual system integration).

---

## 🚀 Complete Production-Ready Example

```cpp
#include &lt;velofx/velofx.h&gt;
#include &lt;iostream&gt;

int main() {
    // 1. Initialize custom binary file-system cache layer (1 hour persistence windows)
    velofx::FileCache local_cache(1); 
    
    // 2. Instantiate VeloFX core execution framework
    velofx::VeloFX fx("6baf00000000000000000000", &local_cache);

    // 3. Process Live Real-Time Conversion Call
    auto exchange_data = fx.convert(velofx::Currency::EUR, velofx::Currency::USD, 1250.50);
    
    if (std::holds_alternative&lt;velofx::ConvertSuccess&gt;(exchange_data)) {
        const auto& success = std::get&lt;velofx::ConvertSuccess&gt;(exchange_data);
        std::cout &lt;&lt; "[VeloFX] Conversion Executed Successfully.\n";
        std::cout &lt;&lt; " &gt; Exchange Rate: " &lt;&lt; success.rate &lt;&lt; "\n";
        std::cout &lt;&lt; " &gt; Total Value  : " &lt;&lt; success.converted_amount &lt;&lt; " USD\n";
    } else {
        const auto& error = std::get&lt;velofx::ConvertError&gt;(exchange_data);
        std::cerr << "[VeloFX-Error] Critical transaction error: " &lt;&lt; error.message &lt;&lt; "\n";
    }

    // 4. Extract Visual Assets and Localization Details
    auto currency_profile = fx.get_currency_info(velofx::Currency::GBP);
    if (currency_profile) {
        std::cout &lt;&lt; "\n[UI Resolution Summary]\n";
        std::cout &lt;&lt; " &gt; Name:   " &lt;&lt; currency_profile-&gt;name &lt;&lt; "\n";
        std::cout &lt;&lt; " &gt; Symbol: " &lt;&lt; currency_profile-&gt;symbol &lt;&lt; "\n";
        std::cout &lt;&lt; " &gt; Flag:   " &lt;&lt; currency_profile-&gt;flag_url &lt;&lt; "\n";
    }

    return 0;
}
```

---

## ⚖️ License

Distributed under the MIT License. See `LICENSE` for details.
'@

# 2. DEFINIZIONE DEL CONTENUTO DI CMAKELISTS.TXT
```cmake
cmake_minimum_required(VERSION 3.24)
project(VeloFX-SDK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fallisce immediatamente se l'utente non è su Windows
if(NOT WIN32)
    message(FATAL_ERROR "VeloFX SDK currently only supports Windows (MSVC x64). Support for Linux/macOS is planned for future releases.")
endif()

# La libreria pre-compilata usa LibCurl, serve che sia presente su Windows
find_package(CURL REQUIRED)

# Definisce VeloFX come libreria statica pre-compilata (importata)
add_library(velofx STATIC IMPORTED GLOBAL)

# Imposta la cartella degli header pubblici
set_target_properties(velofx PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Punta direttamente al file binario .lib su Windows
set_target_properties(velofx PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/velofx.lib"
)

# Propaga la dipendenza di LibCurl all'eseguibile finale che userà l'SDK
if(TARGET CURL::libcurl)
    target_link_libraries(velofx INTERFACE CURL::libcurl)
else()
    target_link_libraries(velofx INTERFACE libcurl)
endif()
```