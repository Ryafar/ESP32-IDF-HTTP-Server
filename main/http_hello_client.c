/**
 * @file http_hello_client.c
 * @brief ESP32 HTTP Hello World Client Implementation
 */

#include "http_hello_client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HTTP_HELLO_CLIENT";

// Global state for the HTTP Hello Client
static struct {
    http_hello_config_t config;
    http_hello_stats_t stats;
    bool initialized;
    int message_counter;
} g_client_state = {0};

/**
 * @brief HTTP event handler for the client
 */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // We could process response data here if needed
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
    }
    return ESP_OK;
}

/**
 * @brief Generate dynamic message content
 */
static esp_err_t generate_message_content(char *buffer, size_t buffer_size, const char *custom_message)
{
    if (!buffer || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get system information
    uint32_t uptime_ms = esp_timer_get_time() / 1000;
    uint32_t uptime_seconds = uptime_ms / 1000;
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    
    // Generate calculations for verification
    int fibonacci = 1;
    for(int i = 0; i < g_client_state.message_counter; i++) {
        fibonacci = fibonacci * 2;
        if(fibonacci > 10000) fibonacci = 1;  // Reset if too big
    }
    
    // Build the message
    int written = snprintf(buffer, buffer_size,
        "🎉 Hello World from ESP32! 🎉\n"
        "═══════════════════════════════════════\n"
        "📊 Message Statistics:\n"
        "   📋 Message Number: %d\n"
        "   ⏰ Uptime: %lu.%03lu seconds (%lu ms total)\n"
        "   🧮 Calculation Result: %d (2^%d simplified)\n"
        "   🆔 Message Hash: %lu\n"
        "\n",
        g_client_state.message_counter,
        (unsigned long)uptime_seconds, 
        (unsigned long)(uptime_ms % 1000), 
        (unsigned long)uptime_ms,
        fibonacci, 
        g_client_state.message_counter,
        (unsigned long)(g_client_state.message_counter * uptime_ms)
    );

    if (written < 0 || written >= buffer_size) {
        ESP_LOGE(TAG, "Message buffer too small");
        return ESP_ERR_NO_MEM;
    }

    // Add system information if enabled
    if (g_client_state.config.include_system_info) {
        int remaining = buffer_size - written;
        int sys_written = snprintf(buffer + written, remaining,
            "💾 System Information:\n"
            "   🔧 Free Heap: %lu bytes\n"
            "   📉 Min Free Heap: %lu bytes\n"
            "   🔋 Heap Usage: %.1f%%\n"
            "\n",
            (unsigned long)free_heap,
            (unsigned long)min_free_heap,
            100.0 - ((float)free_heap / (free_heap + (256*1024 - free_heap)) * 100.0)
        );
        
        if (sys_written < 0 || sys_written >= remaining) {
            ESP_LOGW(TAG, "System info truncated");
        } else {
            written += sys_written;
        }
    }

    // Add network information
    int remaining = buffer_size - written;
    int net_written = snprintf(buffer + written, remaining,
        "🌐 Network Information:\n"
        "   📡 ESP32 is connected to WiFi\n"
        "   🏠 Sending from your local network\n"
        "   📨 HTTP POST to %s:%d%s\n"
        "\n",
        g_client_state.config.host,
        g_client_state.config.port,
        g_client_state.config.path
    );
    
    if (net_written > 0 && net_written < remaining) {
        written += net_written;
        remaining = buffer_size - written;
    }

    // Add random data if enabled
    if (g_client_state.config.include_random_data) {
        int rand_written = snprintf(buffer + written, remaining,
            "🔢 Random Data (changes each message):\n"
            "   🎲 Random Value: %lu\n"
            "   📈 Counter squared: %d\n"
            "   📊 Counter factorial (mod 1000): %d\n"
            "\n",
            esp_random() % 1000,
            g_client_state.message_counter * g_client_state.message_counter,
            (g_client_state.message_counter * (g_client_state.message_counter + 1) / 2) % 1000
        );
        
        if (rand_written > 0 && rand_written < remaining) {
            written += rand_written;
            remaining = buffer_size - written;
        }
    }

    // Add custom message if provided
    if (custom_message && strlen(custom_message) > 0) {
        int custom_written = snprintf(buffer + written, remaining,
            "💬 Custom Message:\n"
            "%s\n"
            "\n",
            custom_message
        );
        
        if (custom_written > 0 && custom_written < remaining) {
            written += custom_written;
            remaining = buffer_size - written;
        }
    }

    // Add footer
    if (remaining > 50) {
        snprintf(buffer + written, remaining,
            "✨ This message was generated at runtime!\n"
            "═══════════════════════════════════════"
        );
    }

    return ESP_OK;
}

esp_err_t http_hello_client_init(const http_hello_config_t *config)
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
    memcpy(&g_client_state.config, config, sizeof(http_hello_config_t));
    
    // Initialize statistics
    memset(&g_client_state.stats, 0, sizeof(http_hello_stats_t));
    g_client_state.message_counter = 0;
    g_client_state.initialized = true;

    ESP_LOGI(TAG, "HTTP Hello Client initialized");
    ESP_LOGI(TAG, "Target: %s:%d%s", config->host, config->port, config->path);
    ESP_LOGI(TAG, "Timeout: %d ms", config->timeout_ms);

    return ESP_OK;
}

esp_err_t http_hello_client_send_message(void)
{
    return http_hello_client_send_custom_message(NULL);
}

esp_err_t http_hello_client_send_custom_message(const char *custom_message)
{
    if (!g_client_state.initialized) {
        ESP_LOGE(TAG, "Client not initialized. Call http_hello_client_init() first.");
        return ESP_ERR_INVALID_STATE;
    }

    g_client_state.message_counter++;
    
    ESP_LOGI(TAG, "Sending Hello World message #%d to %s:%d", 
             g_client_state.message_counter, 
             g_client_state.config.host, 
             g_client_state.config.port);

    // Configure HTTP client
    esp_http_client_config_t client_config = {
        .host = g_client_state.config.host,
        .port = g_client_state.config.port,
        .path = g_client_state.config.path,
        .method = HTTP_METHOD_POST,
        .timeout_ms = g_client_state.config.timeout_ms,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&client_config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        g_client_state.stats.messages_failed++;
        return ESP_FAIL;
    }

    // Generate message content
    char message_buffer[1024];
    esp_err_t ret = generate_message_content(message_buffer, sizeof(message_buffer), custom_message);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to generate message content");
        esp_http_client_cleanup(client);
        g_client_state.stats.messages_failed++;
        return ret;
    }

    // Set POST data and headers
    esp_http_client_set_post_field(client, message_buffer, strlen(message_buffer));
    esp_http_client_set_header(client, "Content-Type", "text/plain; charset=utf-8");
    esp_http_client_set_header(client, "User-Agent", g_client_state.config.user_agent);
    
    // Add custom headers with ESP32 statistics
    char counter_header[32];
    char uptime_header[32];
    uint32_t uptime_ms = esp_timer_get_time() / 1000;
    
    snprintf(counter_header, sizeof(counter_header), "%d", g_client_state.message_counter);
    snprintf(uptime_header, sizeof(uptime_header), "%lu", (unsigned long)uptime_ms);
    
    esp_http_client_set_header(client, "X-ESP32-Message-Counter", counter_header);
    esp_http_client_set_header(client, "X-ESP32-Uptime-MS", uptime_header);

    // Perform HTTP request
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        int64_t content_length = esp_http_client_get_content_length(client);
        
        g_client_state.stats.last_response_code = status_code;
        g_client_state.stats.last_send_time_ms = esp_timer_get_time() / 1000;
        g_client_state.stats.total_uptime_ms = uptime_ms;
        
        ESP_LOGI(TAG, "✅ SUCCESS! Message #%d sent - HTTP POST Status = %d, content_length = %lld", 
                g_client_state.message_counter, status_code, content_length);
        
        if (status_code == 200) {
            ESP_LOGI(TAG, "🎉 Hello World message #%d successfully sent!", g_client_state.message_counter);
            ESP_LOGI(TAG, "📊 Uptime: %lu.%03lu seconds, Free heap: %lu bytes", 
                    uptime_ms / 1000, uptime_ms % 1000, (unsigned long)esp_get_free_heap_size());
            g_client_state.stats.messages_sent++;
        } else {
            ESP_LOGW(TAG, "⚠️ Received response code: %d for message #%d", status_code, g_client_state.message_counter);
            g_client_state.stats.messages_failed++;
        }
    } else {
        ESP_LOGE(TAG, "❌ HTTP POST request #%d failed: %s", g_client_state.message_counter, esp_err_to_name(err));
        ESP_LOGE(TAG, "💡 Make sure:");
        ESP_LOGE(TAG, "   1. Your computer's IP is correct: %s", g_client_state.config.host);
        ESP_LOGE(TAG, "   2. HTTP server is running on port %d", g_client_state.config.port);
        ESP_LOGE(TAG, "   3. Both devices are on the same network");
        g_client_state.stats.messages_failed++;
    }
    
    esp_http_client_cleanup(client);
    return err;
}

esp_err_t http_hello_client_get_stats(http_hello_stats_t *stats)
{
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(stats, &g_client_state.stats, sizeof(http_hello_stats_t));
    return ESP_OK;
}

esp_err_t http_hello_client_update_config(const http_hello_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(&g_client_state.config, config, sizeof(http_hello_config_t));
    ESP_LOGI(TAG, "Configuration updated");
    return ESP_OK;
}

esp_err_t http_hello_client_reset_stats(void)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memset(&g_client_state.stats, 0, sizeof(http_hello_stats_t));
    g_client_state.message_counter = 0;
    ESP_LOGI(TAG, "Statistics reset");
    return ESP_OK;
}

esp_err_t http_hello_client_get_config(http_hello_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(config, &g_client_state.config, sizeof(http_hello_config_t));
    return ESP_OK;
}

esp_err_t http_hello_client_deinit(void)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memset(&g_client_state, 0, sizeof(g_client_state));
    ESP_LOGI(TAG, "HTTP Hello Client deinitialized");
    return ESP_OK;
}

esp_err_t http_hello_client_send_test_sequence(int message_count, int base_delay_ms)
{
    if (!g_client_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (message_count <= 0 || base_delay_ms < 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "🚀 Starting test sequence: %d messages with %d ms base delay", 
             message_count, base_delay_ms);
    
    for (int i = 0; i < message_count; i++) {
        // Send message
        esp_err_t ret = http_hello_client_send_custom_message("Test sequence message");
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send message %d in test sequence", i + 1);
            return ret;
        }
        
        // Wait before next message (except for last message)
        if (i < message_count - 1) {
            int delay_ms = base_delay_ms * (i + 1);  // Increasing delay
            ESP_LOGI(TAG, "⏳ Waiting %d ms before sending message #%d...", delay_ms, i + 2);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    
    ESP_LOGI(TAG, "🏁 Test sequence completed! %d messages sent.", message_count);
    return ESP_OK;
}