/**
 * @file esp32_http_client.h
 * @brief Generic ESP32 HTTP Client Library
 * 
 * A general-purpose, modular HTTP client for ESP32 that can be used
 * for any HTTP communication needs. Not tied to specific applications.
 * 
 * Features:
 * - Configurable HTTP methods (GET, POST, PUT, DELETE, etc.)
 * - Custom headers support
 * - Request/response handling
 * - Error handling and logging
 * - Statistics tracking
 * - Timeout management
 */

#ifndef ESP32_HTTP_CLIENT_H
#define ESP32_HTTP_CLIENT_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_http_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP method enumeration
 */
typedef enum {
    HTTP_CLIENT_METHOD_GET = 0,
    HTTP_CLIENT_METHOD_POST,
    HTTP_CLIENT_METHOD_PUT,
    HTTP_CLIENT_METHOD_DELETE,
    HTTP_CLIENT_METHOD_HEAD,
    HTTP_CLIENT_METHOD_PATCH
} http_client_method_t;

/**
 * @brief HTTP client configuration structure
 */
typedef struct {
    char host[64];              /**< Target server hostname or IP address */
    int port;                   /**< Target server port */
    char path[128];             /**< HTTP path */
    int timeout_ms;             /**< Request timeout in milliseconds */
    char user_agent[64];        /**< User-Agent header */
    bool enable_logging;        /**< Enable detailed logging */
} http_client_config_t;

/**
 * @brief HTTP request structure
 */
typedef struct {
    http_client_method_t method;    /**< HTTP method */
    const char *content_type;       /**< Content-Type header */
    const char *body;               /**< Request body (for POST/PUT) */
    size_t body_length;             /**< Body length (0 = auto-calculate) */
    void *custom_headers;           /**< Custom headers (implementation specific) */
} http_client_request_t;

/**
 * @brief HTTP response structure
 */
typedef struct {
    int status_code;            /**< HTTP status code */
    char *body;                 /**< Response body (allocated, caller must free) */
    size_t body_length;         /**< Response body length */
    size_t content_length;      /**< Content-Length header value */
    char content_type[128];     /**< Content-Type header value */
} http_client_response_t;

/**
 * @brief HTTP client statistics
 */
typedef struct {
    int requests_sent;          /**< Total requests sent */
    int requests_failed;        /**< Total failed requests */
    int last_status_code;       /**< Last HTTP status code */
    uint64_t last_request_time_ms; /**< Timestamp of last request */
    uint64_t total_bytes_sent;  /**< Total bytes sent */
    uint64_t total_bytes_received; /**< Total bytes received */
} http_client_stats_t;

/**
 * @brief Default configuration initializer
 */
#define HTTP_CLIENT_CONFIG_DEFAULT() {              \
    .host = "192.168.1.100",                       \
    .port = 80,                                     \
    .path = "/",                                    \
    .timeout_ms = 5000,                             \
    .user_agent = "ESP32-HTTP-Client/1.0",          \
    .enable_logging = true                          \
}

/**
 * @brief Initialize the HTTP client
 * 
 * @param config Pointer to configuration structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_init(const http_client_config_t *config);

/**
 * @brief Send an HTTP request
 * 
 * @param request Pointer to request structure
 * @param response Pointer to response structure (will be filled)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_send_request(const http_client_request_t *request, http_client_response_t *response);

/**
 * @brief Send a simple GET request
 * 
 * @param path HTTP path (can override config path)
 * @param response Pointer to response structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_get(const char *path, http_client_response_t *response);

/**
 * @brief Send a simple POST request
 * 
 * @param path HTTP path
 * @param content_type Content-Type header
 * @param body Request body
 * @param response Pointer to response structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_post(const char *path, const char *content_type, const char *body, http_client_response_t *response);

/**
 * @brief Add a custom header for next request
 * 
 * @param key Header name
 * @param value Header value
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_add_header(const char *key, const char *value);

/**
 * @brief Clear all custom headers
 * 
 * @return ESP_OK on success
 */
esp_err_t esp32_http_client_clear_headers(void);

/**
 * @brief Get client statistics
 * 
 * @param stats Pointer to statistics structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_get_stats(http_client_stats_t *stats);

/**
 * @brief Reset statistics
 * 
 * @return ESP_OK on success
 */
esp_err_t esp32_http_client_reset_stats(void);

/**
 * @brief Update client configuration
 * 
 * @param config Pointer to new configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_update_config(const http_client_config_t *config);

/**
 * @brief Get current configuration
 * 
 * @param config Pointer to configuration structure to fill
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t esp32_http_client_get_config(http_client_config_t *config);

/**
 * @brief Free response resources
 * 
 * Call this to free memory allocated in response->body
 * 
 * @param response Pointer to response structure
 */
void esp32_http_client_free_response(http_client_response_t *response);

/**
 * @brief Deinitialize the HTTP client
 * 
 * @return ESP_OK on success
 */
esp_err_t esp32_http_client_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // ESP32_HTTP_CLIENT_H