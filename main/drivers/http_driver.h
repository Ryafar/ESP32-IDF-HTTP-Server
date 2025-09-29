/**
 * @file http_driver.h
 * @brief HTTP Client Driver Interface
 * 
 * This driver provides a clean interface for HTTP operations
 * without any FreeRTOS tasks. It handles HTTP requests and
 * responses in a synchronous manner.
 */

#ifndef HTTP_DRIVER_H
#define HTTP_DRIVER_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HTTP method enumeration
 */
typedef enum {
    HTTP_METHOD_GET = 0,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_PATCH
} http_method_t;

/**
 * @brief HTTP request structure
 */
typedef struct {
    http_method_t method;
    char *url;                  /**< Full URL (http://host:port/path) */
    char *headers;              /**< Additional headers (optional) */
    char *body;                 /**< Request body (for POST/PUT) */
    size_t body_length;         /**< Body length (0 = auto-calculate) */
    int timeout_ms;             /**< Request timeout */
} http_request_t;

/**
 * @brief HTTP response structure
 */
typedef struct {
    int status_code;            /**< HTTP status code */
    char *body;                 /**< Response body (caller must free) */
    size_t body_length;         /**< Response body length */
    size_t content_length;      /**< Content-Length header value */
    char content_type[128];     /**< Content-Type header */
} http_response_t;

/**
 * @brief HTTP statistics
 */
typedef struct {
    uint32_t requests_sent;
    uint32_t requests_failed;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t last_request_time_ms;
    int last_status_code;
} http_stats_t;

/**
 * @brief Initialize HTTP driver
 * 
 * Sets up HTTP client subsystem.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_driver_init(void);

/**
 * @brief Send HTTP request
 * 
 * Sends an HTTP request and returns the response.
 * This is a blocking operation.
 * 
 * @param request Pointer to request structure
 * @param response Pointer to response structure (will be filled)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_driver_send_request(const http_request_t *request, http_response_t *response);

/**
 * @brief Send simple GET request
 * 
 * @param url Full URL
 * @param response Pointer to response structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_driver_get(const char *url, http_response_t *response);

/**
 * @brief Send simple POST request
 * 
 * @param url Full URL
 * @param content_type Content-Type header
 * @param body Request body
 * @param response Pointer to response structure
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t http_driver_post(const char *url, const char *content_type, const char *body, http_response_t *response);

/**
 * @brief Free response resources
 * 
 * Call this to free memory allocated in response->body
 * 
 * @param response Pointer to response structure
 */
void http_driver_free_response(http_response_t *response);

/**
 * @brief Get HTTP statistics
 * 
 * @param stats Pointer to statistics structure
 * @return ESP_OK on success
 */
esp_err_t http_driver_get_stats(http_stats_t *stats);

/**
 * @brief Reset HTTP statistics
 * 
 * @return ESP_OK on success
 */
esp_err_t http_driver_reset_stats(void);

/**
 * @brief Deinitialize HTTP driver
 * 
 * @return ESP_OK on success
 */
esp_err_t http_driver_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // HTTP_DRIVER_H