#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <filesystem>
#include <chrono>
#include <memory>
#include <optional>
#include <map>

/**
 * @namespace velofx
 * @brief Main namespace for the VeloFX currency exchange framework.
 */
namespace velofx {

    /**
     * @class Currency
     * @brief Represents a currency compliant with the ISO 4217 standard (e.g., EUR, USD).
     * * This class ensures that currency codes are automatically validated and formatted
     * in uppercase.
     */
    class Currency {
        public:
            /**
             * @brief Constructs a Currency object from a string code.
             * @param code The ISO currency code (e.g., "eur", "USD"). Automatically converted to uppercase.
             */
            explicit Currency(std::string_view code);

            static const Currency EUR; ///< Euro currency (EUR)
            static const Currency USD; ///< United States Dollar (USD)
            static const Currency GBP; ///< British Pound Sterling (GBP)
            static const Currency JPY; ///< Japanese Yen (JPY)
            static const Currency CHF; ///< Swiss Franc (CHF)
            static const Currency CAD; ///< Canadian Dollar (CAD)
            static const Currency AUD; ///< Australian Dollar (AUD)

            /**
             * @brief Returns the string representation of the currency code.
             * @return A string view (std::string_view) of the 3-letter code.
             */
            [[nodiscard]] std::string_view str() const;

            /** @brief Equality operator based on the ISO currency code. */
            bool operator==(const Currency& other) const { return code_ == other.code_; }
            
            /** @brief Inequality operator based on the ISO currency code. */
            bool operator!=(const Currency& other) const { return !(*this == other); }
    
        private:
            std::string code_;
    };

    /**
     * @struct CurrencyInfo
     * @brief Contains enriched details for a specific currency.
     * * Encompasses localized information and useful assets for UI/UX development.
     */
    struct CurrencyInfo {
        std::string name;          ///< Full name of the currency (e.g., "Euro")
        std::string symbol;        ///< Typographical or Unicode symbol (e.g., "€")
        std::string country_name;  ///< Associated country name (e.g., "European Union")
        std::string flag_url;      ///< URL of the country flag image (PNG/SVG)
    };
    
    /**
     * @struct ConvertSuccess
     * @brief Structure returned upon successful currency conversion.
     */
    struct ConvertSuccess { 
        double rate; 
        double converted_amount; 
    };

    /**
     * @struct ConvertError
     * @brief Structure returned upon operation failure.
     */
    struct ConvertError { 
        std::string message;     ///< Detailed message describing the error encountered 
    };

    /**
     * @typedef ConvertResult
     * @brief Variant type representing the outcome of an exchange operation.
     * * Can contain either a ConvertSuccess or a ConvertError object.
     */
    using ConvertResult   = std::variant<ConvertSuccess, ConvertError>;

    /**
     * @class Cache
     * @brief Pure abstract interface for managing exchange rate caching.
     * * Derived classes must implement methods to save and retrieve rates from RAM, file, or database.
     */
    class Cache {
        public:
            virtual ~Cache() = default;

            /**
             * @brief Tries to retrieve a stored exchange rate from the cache.
             * @param from Base currency code.
             * @param to Target currency code.
             * @param out_rate Reference where the rate will be written if found and valid.
             * @return true if the rate is present in the cache and valid, false otherwise.
             */
            virtual bool get(std::string_view from, std::string_view to, double& out_rate) = 0;
            
            /**
             * @brief Stores a new exchange rate inside the cache.
             * @param from Base currency code.
             * @param to Target currency code.
             * @param rate The exchange rate to save.
             * @return true if successfully saved, false in case of IO or storage errors.
             */
            virtual bool set(std::string_view from, std::string_view to, double rate) = 0;
    };

    /**
     * @class FileCache
     * @brief Implementation of the Cache interface based on local binary files.
     * * Saves rate records on the user's disk inside a dedicated folder,
     * verifying time validity via a Time-To-Live (TTL) mechanism.
     */
    class FileCache : public Cache {
        public:
            /**
             * @brief Initializes the file cache, setting data expiration limits.
             * @param ttl_hours Hours of data validity before it is considered stale (Default: 1 hour).
             */
            explicit FileCache(int ttl_hours = 1);
            bool get(std::string_view from, std::string_view to, double& out_rate) override;
            bool set(std::string_view from, std::string_view to, double rate) override;

        private:
            int ttl_hours_;
            struct RateCache {
                std::time_t timestamp;
                char from_currency[4];
                char to_currency[4];
                double rate;
            };
            [[nodiscard]] std::filesystem::path get_cache_path(std::string_view from, std::string_view to) const;
    };

    /**
     * @class VeloFX
     * @brief Core engine responsible for interacting with the remote ExchangeRate endpoint.
     * * Manages HTTP requests via LibCurl, parses JSON responses, and performs currency exchange
     * operations while integrating seamlessly with the provided caching layer.
     */
    class VeloFX {
        public:
            /**
             * @brief Initializes the VeloFX engine with API credentials and an optional cache provider.
             * @param api_key Valid authentication key for the ExchangeRate-API (v6) service.
             * @param cache_provider Pointer to a cache instance (e.g., FileCache). Pass nullptr to disable caching.
             */
            explicit VeloFX(std::string_view api_key, Cache* cache_provider = nullptr);

            /**
             * @brief Performs real-time conversion of an amount between two currencies.
             * * Checks the cache first (if enabled). If data is missing or expired, it queries the
             * network and automatically updates the local cache storage.
             * * @param from The base currency.
             * @param to The target currency.
             * @param amount The amount of money to convert.
             * @param force_refresh If set to true, ignores the cache and forces a direct network call.
             * @return A ConvertResult containing computation details or the error description.
             */
            ConvertResult convert(const Currency& from, const Currency& to, double amount, bool force_refresh = false);

            /**
             * @brief Retrieves the full list of world exchange rates relative to a base currency.
             * @param base The reference currency to calculate the full rate list.
             * @return A std::map containing currency codes as keys and their corresponding exchange rates as values.
             */
            std::map<std::string, double> get_rates(const Currency& base);

            /**
             * @brief Performs a currency conversion based on historical exchange rates (dating back to 1990).
             * @param from The base currency.
             * @param to The target currency.
             * @param amount The amount to convert.
             * @param year The historical year of the request (e.g., 2015).
             * @param month The historical month of the request (1-12).
             * @param day The historical day of the request (1-31).
             * @return A ConvertResult with historical calculations or an error message if the date is not found.
             */
            ConvertResult convert_historical(const Currency& from, const Currency& to, double amount, int year, int month, int day);

            /**
             * @brief Retrieves extended and enriched information (symbols, full names, flags) for a currency.
             * @param currency The currency to fetch structured details for.
             * @return A std::optional containing the populated CurrencyInfo structure, or std::nullopt in case of network error.
             */
            std::optional<CurrencyInfo> get_currency_info(const Currency& currency);

        private:
            std::string api_key_;
            Cache* cache_;

            double fetch_from_network(std::string_view from, std::string_view to);
    };
}