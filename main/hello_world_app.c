/**
 * @file hello_world_app.c
 * @brief Hello World Application Implementation
 */

#include "hello_world_app.h"
#include "esp32_http_client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "HELLO_WORLD_APP";

/**
 * @brief Application state
 */
static struct {
    hello_world_config_t config;
    hello_world_stats_t stats;
    bool initialized;
    int message_counter;
    TaskHandle_t auto_task_handle;
    bool auto_mode_running;
} g_app_state = {0};

/**
 * @brief Generate Hello World message content
 */
static esp_err_t generate_hello_world_content(char *buffer, size_t buffer_size, const char *custom_message)
{
    if (!buffer || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get system information
    uint32_t uptime_ms = esp_timer_get_time() / 1000;
    uint32_t uptime_seconds = uptime_ms / 1000;
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    
    // Generate some changing calculations for verification
    int calculation_result = 1;
    for(int i = 0; i < g_app_state.message_counter; i++) {
        calculation_result = calculation_result * 2;
        if(calculation_result > 10000) calculation_result = 1;  // Reset if too big
    }
    
    // Build the main message
    int written = snprintf(buffer, buffer_size,
        "🎉 Hello World from ESP32! 🎉\n"
        "═══════════════════════════════════════\n"
        "📊 Message Information:\n"
        "   📋 Message Number: %d\n"
        "   ⏰ ESP32 Uptime: %lu.%03lu seconds (%lu ms total)\n"
        "   🧮 Verification Value: %d (calc: 2^%d)\n"
        "   🆔 Message Hash: %lu\n"
        "\n",
        g_app_state.message_counter,
        (unsigned long)uptime_seconds, 
        (unsigned long)(uptime_ms % 1000), 
        (unsigned long)uptime_ms,
        calculation_result, 
        g_app_state.message_counter,
        (unsigned long)(g_app_state.message_counter * uptime_ms)  // Simple hash
    );

    if (written < 0 || written >= buffer_size) {
        ESP_LOGE(TAG, "Message buffer too small");
        return ESP_ERR_NO_MEM;
    }

    // Add system information if enabled
    if (g_app_state.config.include_system_info) {
        int remaining = buffer_size - written;
        int sys_written = snprintf(buffer + written, remaining,
            "💾 ESP32 System Information:\n"
            "   🔧 Free Heap Memory: %lu bytes\n"
            "   📉 Minimum Free Heap: %lu bytes\n"
            "   🔋 Memory Usage: %.1f%%\n"
            "   🏭 Target Server: %s:%d%s\n"
            "\n",
            (unsigned long)free_heap,
            (unsigned long)min_free_heap,
            100.0 - ((float)free_heap / (free_heap + (256*1024 - free_heap)) * 100.0),
            g_app_state.config.target_host,
            g_app_state.config.target_port,
            g_app_state.config.target_path
        );
        
        if (sys_written > 0 && sys_written < remaining) {
            written += sys_written;
        }
    }

    // Add random data if enabled
    if (g_app_state.config.include_random_data) {
        int remaining = buffer_size - written;
        int rand_written = snprintf(buffer + written, remaining,
            "🔢 Verification Data (changes each message):\n"
            "   🎲 Random Value: %lu\n"
            "   📈 Counter Squared: %d\n"
            "   📊 Sum Formula: %d\n"
            "\n",
            esp_random() % 1000,
            g_app_state.message_counter * g_app_state.message_counter,
            (g_app_state.message_counter * (g_app_state.message_counter + 1) / 2) % 1000
        );
        
        if (rand_written > 0 && rand_written < remaining) {
            written += rand_written;
        }
    }

    // Add custom message if provided
    if (custom_message && strlen(custom_message) > 0) {
        int remaining = buffer_size - written;
        int custom_written = snprintf(buffer + written, remaining,
            "💬 Custom Message:\n"
            "%s\n"
            "\n",
            custom_message
        );
        
        if (custom_written > 0 && custom_written < remaining) {
            written += custom_written;
        }
    }

    // Add network info and footer
    int remaining = buffer_size - written;
    if (remaining > 100) {
        snprintf(buffer + written, remaining,
            "🌐 Network Information:\n"
            "   📡 ESP32 connected to WiFi\n"
            "   🏠 Local network communication\n"
            "   📨 HTTP POST request\n"
            "\n"
            "✨ Generated at runtime by ESP32!\n"
            "═══════════════════════════════════════"
        );
    }

    return ESP_OK;
}

/**
 * @brief Background task for auto mode
 */
static void auto_message_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🤖 Auto mode started - sending messages every %d ms", 
             g_app_state.config.message_interval_ms);

    while (g_app_state.auto_mode_running) {
        esp_err_t ret = hello_world_app_send_message();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send auto message: %s", esp_err_to_name(ret));
        }
        
        // Wait for the specified interval
        vTaskDelay(pdMS_TO_TICKS(g_app_state.config.message_interval_ms));
    }

    ESP_LOGI(TAG, "🛑 Auto mode stopped");
    g_app_state.auto_task_handle = NULL;
    vTaskDelete(NULL);
}

esp_err_t hello_world_app_init(const hello_world_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "Configuration cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (strlen(config->target_host) == 0) {
        ESP_LOGE(TAG, "Target host cannot be empty");
        return ESP_ERR_INVALID_ARG;
    }

    // Copy configuration
    memcpy(&g_app_state.config, config, sizeof(hello_world_config_t));
    
    // Initialize statistics
    memset(&g_app_state.stats, 0, sizeof(hello_world_stats_t));
    g_app_state.message_counter = 0;
    g_app_state.initialized = true;
    g_app_state.auto_mode_running = false;
    g_app_state.auto_task_handle = NULL;

    // Initialize the generic HTTP client
    http_client_config_t http_config = HTTP_CLIENT_CONFIG_DEFAULT();
    strcpy(http_config.host, config->target_host);
    http_config.port = config->target_port;
    strcpy(http_config.path, config->target_path);
    strcpy(http_config.user_agent, "ESP32-HelloWorld-App/1.0");
    http_config.enable_logging = true;

    esp_err_t ret = esp32_http_client_init(&http_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Hello World App initialized");
    ESP_LOGI(TAG, "Target: %s:%d%s", config->target_host, config->target_port, config->target_path);
    ESP_LOGI(TAG, "System info: %s, Random data: %s", 
             config->include_system_info ? "enabled" : "disabled",
             config->include_random_data ? "enabled" : "disabled");

    return ESP_OK;
}

esp_err_t hello_world_app_send_message(void)
{
    return hello_world_app_send_custom_message(NULL);
}

esp_err_t hello_world_app_send_custom_message(const char *custom_message)
{
    if (!g_app_state.initialized) {
        ESP_LOGE(TAG, "App not initialized. Call hello_world_app_init() first.");
        return ESP_ERR_INVALID_STATE;
    }

    g_app_state.message_counter++;
    
    ESP_LOGI(TAG, "📤 Sending Hello World message #%d", g_app_state.message_counter);

    // Generate message content
    char message_buffer[1024];
    esp_err_t ret = generate_hello_world_content(message_buffer, sizeof(message_buffer), custom_message);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to generate message content");
        g_app_state.stats.messages_failed++;
        return ret;
    }

    // Add custom headers with ESP32 information
    char counter_header[32];
    char uptime_header[32];
    uint32_t uptime_ms = esp_timer_get_time() / 1000;
    
    snprintf(counter_header, sizeof(counter_header), "%d", g_app_state.message_counter);
    snprintf(uptime_header, sizeof(uptime_header), "%lu", (unsigned long)uptime_ms);
    
    esp32_http_client_clear_headers();
    esp32_http_client_add_header("X-ESP32-Message-Counter", counter_header);
    esp32_http_client_add_header("X-ESP32-Uptime-MS", uptime_header);
    esp32_http_client_add_header("X-ESP32-App", "HelloWorld");

    // Send the HTTP POST request
    http_client_response_t response;
    ret = esp32_http_client_post(g_app_state.config.target_path, "text/plain; charset=utf-8", message_buffer, &response);
    
    if (ret == ESP_OK) {
        g_app_state.stats.last_message_time = esp_timer_get_time() / 1000;
        g_app_state.stats.uptime_at_last_message = uptime_ms;
        
        if (response.status_code == 200) {
            ESP_LOGI(TAG, "✅ Hello World message #%d sent successfully!", g_app_state.message_counter);
            ESP_LOGI(TAG, "📊 Response: %d, Uptime: %lu.%03lu seconds", 
                    response.status_code, uptime_ms / 1000, uptime_ms % 1000);
            g_app_state.stats.messages_sent++;
        } else {
            ESP_LOGW(TAG, "⚠️ Received response code: %d for message #%d", response.status_code, g_app_state.message_counter);
            g_app_state.stats.messages_failed++;
        }

        // Log response if available
        if (response.body && response.body_length > 0) {
            ESP_LOGI(TAG, "📥 Server response: %.*s", (int)response.body_length, response.body);
        }
    } else {
        ESP_LOGE(TAG, "❌ Failed to send Hello World message #%d: %s", g_app_state.message_counter, esp_err_to_name(ret));
        g_app_state.stats.messages_failed++;
    }
    
    // Clean up response
    esp32_http_client_free_response(&response);
    
    return ret;
}

esp_err_t hello_world_app_send_demo_sequence(int count, int base_delay_ms)
{
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (count <= 0 || base_delay_ms < 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "🚀 Starting Hello World demo: %d messages", count);
    
    for (int i = 0; i < count; i++) {
        char demo_message[128];
        snprintf(demo_message, sizeof(demo_message), "Demo sequence message %d of %d", i + 1, count);
        
        esp_err_t ret = hello_world_app_send_custom_message(demo_message);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send demo message %d", i + 1);
            return ret;
        }
        
        // Wait before next message (except for last message)
        if (i < count - 1) {
            int delay_ms = base_delay_ms + (i * 1000);  // Increasing delay
            ESP_LOGI(TAG, "⏳ Waiting %d ms before next message...", delay_ms);
            vTaskDelay(pdMS_TO_TICKS(delay_ms));
        }
    }
    
    ESP_LOGI(TAG, "🏁 Demo sequence completed! %d messages sent.", count);
    return ESP_OK;
}

esp_err_t hello_world_app_get_stats(hello_world_stats_t *stats)
{
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(stats, &g_app_state.stats, sizeof(hello_world_stats_t));
    return ESP_OK;
}

esp_err_t hello_world_app_update_config(const hello_world_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(&g_app_state.config, config, sizeof(hello_world_config_t));
    
    // Update HTTP client config too
    http_client_config_t http_config;
    esp32_http_client_get_config(&http_config);
    strcpy(http_config.host, config->target_host);
    http_config.port = config->target_port;
    strcpy(http_config.path, config->target_path);
    esp32_http_client_update_config(&http_config);
    
    ESP_LOGI(TAG, "Configuration updated");
    return ESP_OK;
}

esp_err_t hello_world_app_start_auto_mode(void)
{
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (g_app_state.auto_mode_running) {
        ESP_LOGW(TAG, "Auto mode already running");
        return ESP_OK;
    }
    
    g_app_state.auto_mode_running = true;
    
    BaseType_t result = xTaskCreate(auto_message_task, "hello_auto", 4096, NULL, 5, &g_app_state.auto_task_handle);
    if (result != pdPASS) {
        g_app_state.auto_mode_running = false;
        ESP_LOGE(TAG, "Failed to create auto mode task");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t hello_world_app_stop_auto_mode(void)
{
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!g_app_state.auto_mode_running) {
        ESP_LOGW(TAG, "Auto mode not running");
        return ESP_OK;
    }
    
    g_app_state.auto_mode_running = false;
    
    // Wait for task to finish
    if (g_app_state.auto_task_handle) {
        // Task will delete itself
        g_app_state.auto_task_handle = NULL;
    }
    
    return ESP_OK;
}

bool hello_world_app_is_auto_mode_running(void)
{
    return g_app_state.auto_mode_running;
}

esp_err_t hello_world_app_deinit(void)
{
    if (!g_app_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Stop auto mode if running
    hello_world_app_stop_auto_mode();
    
    // Deinitialize HTTP client
    esp32_http_client_deinit();
    
    // Clear state
    memset(&g_app_state, 0, sizeof(g_app_state));
    
    ESP_LOGI(TAG, "Hello World App deinitialized");
    return ESP_OK;
}