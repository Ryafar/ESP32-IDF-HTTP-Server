/**
 * @file http_hello_client.h
 * @brief ESP32 HTTP Hello World Client Library
 * 
 * A modular HTTP client for sending "Hello World" messages with dynamic data
 * from ESP32 to any HTTP server on the local network.
 * 
 * Features:
 * - Configurable target host and port
 * - Dynamic message content with system information
 * - Custom HTTP headers with ESP32 statistics
 * - Error handling and logging
 * - Message counter and timing information
 */

#ifndef HTTP_HELLO_CLIENT_H
#define HTTP_HELLO_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP Hello Client configuration structure
 */
typedef struct {
    char host[64];              /**< Target server hostname or IP address */
    int port;                   /**< Target server port (default: 8000) */
    char path[128];             /**< HTTP path (default: "/hello") */
    int timeout_ms;             /**< Request timeout in milliseconds (default: 5000) */
    bool include_system_info;   /**< Include ESP32 system information in message */
    bool include_random_data;   /**< Include random data for verification */
    char user_agent[64];        /**< Custom User-Agent header */
} http_hello_config_t;

/**
 * @brief HTTP Hello Client statistics structure
 */
typedef struct {
    int messages_sent;          /**< Total number of messages sent */
    int messages_failed;        /**< Total number of failed messages */
    uint32_t last_response_code; /**< Last HTTP response code received */
    uint64_t last_send_time_ms; /**< Timestamp of last message sent */
    uint32_t total_uptime_ms;   /**< ESP32 uptime when last message was sent */
} http_hello_stats_t;

/**
 * @brief Default configuration initializer
 */
#define HTTP_HELLO_CONFIG_DEFAULT() {                \
    .host = "192.168.1.100",                        \
    .port = 8000,                                   \
    .path = "/hello",                               \
    .timeout_ms = 5000,                             \
    .include_system_info = true,                    \
    .include_random_data = true,                    \
    .user_agent = "ESP32-Hello-Client/1.0"          \
}

/**
 * @brief Initialize the HTTP Hello Client
 * 
 * This function must be called before using any other client functions.
 * It initializes internal state and validates configuration.
 * 
 * @param config Pointer to configuration structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_hello_client_init(const http_hello_config_t *config);

/**
 * @brief Send a Hello World message to the configured server
 * 
 * Sends an HTTP POST request with dynamic content including:
 * - Message counter
 * - ESP32 uptime and system information
 * - Random data for verification
 * - Custom HTTP headers
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_hello_client_send_message(void);

/**
 * @brief Send a custom Hello World message
 * 
 * @param custom_message Custom message content to send (will be appended to standard data)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_hello_client_send_custom_message(const char *custom_message);

/**
 * @brief Get current client statistics
 * 
 * @param stats Pointer to statistics structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if stats is NULL
 */
esp_err_t http_hello_client_get_stats(http_hello_stats_t *stats);

/**
 * @brief Update client configuration
 * 
 * @param config Pointer to new configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_hello_client_update_config(const http_hello_config_t *config);

/**
 * @brief Reset message counter and statistics
 * 
 * @return ESP_OK on success
 */
esp_err_t http_hello_client_reset_stats(void);

/**
 * @brief Get current configuration
 * 
 * @param config Pointer to configuration structure to fill
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if config is NULL
 */
esp_err_t http_hello_client_get_config(http_hello_config_t *config);

/**
 * @brief Deinitialize the HTTP Hello Client
 * 
 * Cleans up resources and resets internal state.
 * Call this when you're done using the client.
 * 
 * @return ESP_OK on success
 */
esp_err_t http_hello_client_deinit(void);

/**
 * @brief Send multiple messages with configurable delays
 * 
 * Convenience function for testing - sends multiple messages
 * with increasing delays between them.
 * 
 * @param message_count Number of messages to send
 * @param base_delay_ms Base delay between messages (will increase progressively)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_hello_client_send_test_sequence(int message_count, int base_delay_ms);

#ifdef __cplusplus
}
#endif

#endif // HTTP_HELLO_CLIENT_H