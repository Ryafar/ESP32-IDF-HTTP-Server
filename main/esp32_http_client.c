/**
 * @file esp32_http_client.c
 * @brief Generic ESP32 HTTP Client Implementation
 */

#include "esp32_http_client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ESP32_HTTP_CLIENT";

#define MAX_HEADERS 10
#define MAX_HEADER_SIZE 256

/**
 * @brief Custom header structure
 */
typedef struct {
    char key[64];
    char value[192];
} custom_header_t;

/**
 * @brief Global client state
 */
static struct {
    http_client_config_t config;
    http_client_stats_t stats;
    bool initialized;
    custom_header_t custom_headers[MAX_HEADERS];
    int header_count;
} g_client_state = {0};

/**
 * @brief Convert method enum to ESP-IDF method
 */
static esp_http_client_method_t convert_method(http_client_method_t method)
{
    switch (method) {
        case HTTP_CLIENT_METHOD_GET:    return HTTP_METHOD_GET;
        case HTTP_CLIENT_METHOD_POST:   return HTTP_METHOD_POST;
        case HTTP_CLIENT_METHOD_PUT:    return HTTP_METHOD_PUT;
        case HTTP_CLIENT_METHOD_DELETE: return HTTP_METHOD_DELETE;
        case HTTP_CLIENT_METHOD_HEAD:   return HTTP_METHOD_HEAD;
        case HTTP_CLIENT_METHOD_PATCH:  return HTTP_METHOD_PATCH;
        default:                        return HTTP_METHOD_GET;
    }
}

/**
 * @brief HTTP event handler
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            }
            break;
        case HTTP_EVENT_ON_CONNECTED:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            }
            break;
        case HTTP_EVENT_HEADER_SENT:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            }
            break;
        case HTTP_EVENT_ON_HEADER:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            }
            break;
        case HTTP_EVENT_ON_DATA:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            }
            break;
        case HTTP_EVENT_DISCONNECTED:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            }
            break;
        case HTTP_EVENT_REDIRECT:
            if (g_client_state.config.enable_logging) {
                ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            }
            break;
    }
    return ESP_OK;
}

esp_err_t esp32_http_client_init(const http_client_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Configuration cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(config->host) == 0) {
        ESP_LOGE(TAG, "Host cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    if (config->port <= 0 || config->port > 65535) {
        ESP_LOGE(TAG, "Invalid port number: %d", config->port);
        return ESP_ERR_INVALID_ARG;
    }

    // Copy configuration
    memcpy(&g_client_state.config, config, sizeof(http_client_config_t));
    
    // Initialize statistics
    memset(&g_client_state.stats, 0, sizeof(http_client_stats_t));
    
    // Initialize headers
    memset(g_client_state.custom_headers, 0, sizeof(g_client_state.custom_headers));
    g_client_state.header_count = 0;
    
    g_client_state.initialized = true;

    ESP_LOGI(TAG, "HTTP Client initialized");
    ESP_LOGI(TAG, "Target: %s:%d", config->host, config->port);
    ESP_LOGI(TAG, "Default path: %s", config->path);
    ESP_LOGI(TAG, "Timeout: %d ms", config->timeout_ms);

    return ESP_OK;
}

esp_err_t esp32_http_client_send_request(const http_client_request_t *request, http_client_response_t *response)
{
    if (!g_client_state.initialized) {
        ESP_LOGE(TAG, "Client not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!request || !response) {
        ESP_LOGE(TAG, "Request and response cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Initialize response
    memset(response, 0, sizeof(http_client_response_t));

    // Configure HTTP client
    esp_http_client_config_t client_config = {
        .host = g_client_state.config.host,
        .port = g_client_state.config.port,
        .path = g_client_state.config.path,
        .method = convert_method(request->method),
        .timeout_ms = g_client_state.config.timeout_ms,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        g_client_state.stats.requests_failed++;
        return ESP_FAIL;
    }

    // Set headers
    esp_http_client_set_header(client, "User-Agent", g_client_state.config.user_agent);
    
    if (request->content_type) {
        esp_http_client_set_header(client, "Content-Type", request->content_type);
    }

    // Add custom headers
    for (int i = 0; i < g_client_state.header_count; i++) {
        esp_http_client_set_header(client, 
                                   g_client_state.custom_headers[i].key,
                                   g_client_state.custom_headers[i].value);
    }

    // Set body for POST/PUT methods
    if (request->body && (request->method == HTTP_CLIENT_METHOD_POST || 
                         request->method == HTTP_CLIENT_METHOD_PUT ||
                         request->method == HTTP_CLIENT_METHOD_PATCH)) {
        size_t body_len = request->body_length > 0 ? request->body_length : strlen(request->body);
        esp_http_client_set_post_field(client, request->body, body_len);
        g_client_state.stats.total_bytes_sent += body_len;
    }

    // Perform HTTP request
    uint64_t start_time = esp_timer_get_time() / 1000;
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        response->status_code = esp_http_client_get_status_code(client);
        response->content_length = esp_http_client_get_content_length(client);
        
        // Get content type
        char *content_type = NULL;
        esp_err_t header_err = esp_http_client_get_header(client, "Content-Type", &content_type);
        if (header_err == ESP_OK && content_type) {
            strncpy(response->content_type, content_type, sizeof(response->content_type) - 1);
            response->content_type[sizeof(response->content_type) - 1] = '\0';
        }

        // Read response body if available
        if (response->content_length > 0 && response->content_length < 64*1024) { // Limit to 64KB
            response->body = malloc(response->content_length + 1);
            if (response->body) {
                int data_read = esp_http_client_read_response(client, response->body, response->content_length);
                if (data_read >= 0) {
                    response->body[data_read] = '\0';
                    response->body_length = data_read;
                    g_client_state.stats.total_bytes_received += data_read;
                } else {
                    free(response->body);
                    response->body = NULL;
                    response->body_length = 0;
                }
            }
        }
        
        // Update statistics
        g_client_state.stats.requests_sent++;
        g_client_state.stats.last_status_code = response->status_code;
        g_client_state.stats.last_request_time_ms = start_time;
        
        if (g_client_state.config.enable_logging) {
            ESP_LOGI(TAG, "✅ HTTP %s successful - Status: %d, Content-Length: %zu", 
                    (request->method == HTTP_CLIENT_METHOD_GET) ? "GET" :
                    (request->method == HTTP_CLIENT_METHOD_POST) ? "POST" :
                    (request->method == HTTP_CLIENT_METHOD_PUT) ? "PUT" :
                    (request->method == HTTP_CLIENT_METHOD_DELETE) ? "DELETE" : "REQUEST",
                    response->status_code, response->content_length);
        }
    } else {
        ESP_LOGE(TAG, "❌ HTTP request failed: %s", esp_err_to_name(err));
        g_client_state.stats.requests_failed++;
    }
    
    esp_http_client_cleanup(client);
    return err;
}

esp_err_t esp32_http_client_get(const char *path, http_client_response_t *response)
{
    if (!path) {
        path = g_client_state.config.path;
    }

    // Temporarily update path if different
    char original_path[128];
    bool path_changed = false;
    if (strcmp(path, g_client_state.config.path) != 0) {
        strcpy(original_path, g_client_state.config.path);
        strncpy(g_client_state.config.path, path, sizeof(g_client_state.config.path) - 1);
        g_client_state.config.path[sizeof(g_client_state.config.path) - 1] = '\0';
        path_changed = true;
    }

    http_client_request_t request = {
        .method = HTTP_CLIENT_METHOD_GET,
        .content_type = NULL,
        .body = NULL,
        .body_length = 0,
        .custom_headers = NULL
    };

    esp_err_t ret = esp32_http_client_send_request(&request, response);

    // Restore original path if changed
    if (path_changed) {
        strcpy(g_client_state.config.path, original_path);
    }

    return ret;
}

esp_err_t esp32_http_client_post(const char *path, const char *content_type, const char *body, http_client_response_t *response)
{
    if (!path || !body) {
        return ESP_ERR_INVALID_ARG;
    }

    // Temporarily update path if different
    char original_path[128];
    bool path_changed = false;
    if (strcmp(path, g_client_state.config.path) != 0) {
        strcpy(original_path, g_client_state.config.path);
        strncpy(g_client_state.config.path, path, sizeof(g_client_state.config.path) - 1);
        g_client_state.config.path[sizeof(g_client_state.config.path) - 1] = '\0';
        path_changed = true;
    }

    http_client_request_t request = {
        .method = HTTP_CLIENT_METHOD_POST,
        .content_type = content_type ? content_type : "text/plain",
        .body = body,
        .body_length = 0, // Auto-calculate
        .custom_headers = NULL
    };

    esp_err_t ret = esp32_http_client_send_request(&request, response);

    // Restore original path if changed
    if (path_changed) {
        strcpy(g_client_state.config.path, original_path);
    }

    return ret;
}

esp_err_t esp32_http_client_add_header(const char *key, const char *value)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!key || !value) {
        return ESP_ERR_INVALID_ARG;
    }

    if (g_client_state.header_count >= MAX_HEADERS) {
        ESP_LOGE(TAG, "Maximum headers (%d) reached", MAX_HEADERS);
        return ESP_ERR_NO_MEM;
    }

    strncpy(g_client_state.custom_headers[g_client_state.header_count].key, key, 63);
    g_client_state.custom_headers[g_client_state.header_count].key[63] = '\0';
    
    strncpy(g_client_state.custom_headers[g_client_state.header_count].value, value, 191);
    g_client_state.custom_headers[g_client_state.header_count].value[191] = '\0';
    
    g_client_state.header_count++;

    if (g_client_state.config.enable_logging) {
        ESP_LOGD(TAG, "Added header: %s: %s", key, value);
    }

    return ESP_OK;
}

esp_err_t esp32_http_client_clear_headers(void)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    memset(g_client_state.custom_headers, 0, sizeof(g_client_state.custom_headers));
    g_client_state.header_count = 0;

    if (g_client_state.config.enable_logging) {
        ESP_LOGD(TAG, "Cleared all custom headers");
    }

    return ESP_OK;
}

esp_err_t esp32_http_client_get_stats(http_client_stats_t *stats)
{
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(stats, &g_client_state.stats, sizeof(http_client_stats_t));
    return ESP_OK;
}

esp_err_t esp32_http_client_reset_stats(void)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memset(&g_client_state.stats, 0, sizeof(http_client_stats_t));
    ESP_LOGI(TAG, "Statistics reset");
    return ESP_OK;
}

esp_err_t esp32_http_client_update_config(const http_client_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(&g_client_state.config, config, sizeof(http_client_config_t));
    ESP_LOGI(TAG, "Configuration updated");
    return ESP_OK;
}

esp_err_t esp32_http_client_get_config(http_client_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(config, &g_client_state.config, sizeof(http_client_config_t));
    return ESP_OK;
}

void esp32_http_client_free_response(http_client_response_t *response)
{
    if (response && response->body) {
        free(response->body);
        response->body = NULL;
        response->body_length = 0;
    }
}

esp_err_t esp32_http_client_deinit(void)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memset(&g_client_state, 0, sizeof(g_client_state));
    ESP_LOGI(TAG, "HTTP Client deinitialized");
    return ESP_OK;
}