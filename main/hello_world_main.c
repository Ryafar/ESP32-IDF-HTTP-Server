/* ESP HTTP Client Hello World Example

   This example demonstrates how to use the modular HTTP Hello Client
   to send messages from ESP32 to your computer over WiFi.

   This example code is in the Public Domain (or CC0 licensed, at your option.)
*/

#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

// Include our modular HTTP Hello Client
#include "http_hello_client.h"

static const char *TAG = "HELLO_WORLD_MAIN";

/**
 * @brief Main task that demonstrates the HTTP Hello Client
 */
static void hello_world_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting ESP32 Hello World HTTP Client Demo");
    
    // Configure the HTTP Hello Client
    http_hello_config_t config = HTTP_HELLO_CONFIG_DEFAULT();
    
    // UPDATE THIS IP ADDRESS TO YOUR COMPUTER'S IP!
    strcpy(config.host, "192.168.1.13");  // Change this to your computer's IP
    config.port = 8000;
    strcpy(config.path, "/hello");
    config.timeout_ms = 5000;
    config.include_system_info = true;
    config.include_random_data = true;
    strcpy(config.user_agent, "ESP32-Hello-World/1.0");
    
    // Initialize the HTTP client
    esp_err_t ret = http_hello_client_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "📡 HTTP Client initialized successfully");
    ESP_LOGI(TAG, "🎯 Target: %s:%d%s", config.host, config.port, config.path);
    ESP_LOGI(TAG, "💡 Make sure simple_server.py is running on your computer!");
    
    // Send a series of messages to demonstrate functionality
    ESP_LOGI(TAG, "📨 Sending test sequence of messages...");
    
    // Method 1: Send individual messages with custom content
    ESP_LOGI(TAG, "\n=== Method 1: Individual Messages ===");
    
    // Send first message
    ret = http_hello_client_send_message();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ First message sent successfully");
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));  // Wait 3 seconds
    
    // Send custom message
    ret = http_hello_client_send_custom_message("This is a custom message from ESP32! 🌟");
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Custom message sent successfully");
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));  // Wait 2 seconds
    
    // Method 2: Send test sequence (automated)
    ESP_LOGI(TAG, "\n=== Method 2: Automated Test Sequence ===");
    ret = http_hello_client_send_test_sequence(3, 2000);  // 3 messages, 2s base delay
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Test sequence completed successfully");
    }
    
    // Display final statistics
    http_hello_stats_t stats;
    ret = http_hello_client_get_stats(&stats);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "\n📊 Final Statistics:");
        ESP_LOGI(TAG, "   📈 Messages sent: %d", stats.messages_sent);
        ESP_LOGI(TAG, "   ❌ Messages failed: %d", stats.messages_failed);
        ESP_LOGI(TAG, "   📋 Last response code: %lu", (unsigned long)stats.last_response_code);
        ESP_LOGI(TAG, "   ⏰ Last send time: %llu ms", stats.last_send_time_ms);
        ESP_LOGI(TAG, "   🕐 Total uptime: %lu ms", (unsigned long)stats.total_uptime_ms);
    }
    
    // Clean up
    http_hello_client_deinit();
    
    ESP_LOGI(TAG, "🏁 Demo completed! Check your computer to see all the messages.");
    ESP_LOGI(TAG, "💡 You can restart the ESP32 to run the demo again.");
    ESP_LOGI(TAG, "Finish http example - Hello World sent!");
    
#if !CONFIG_IDF_TARGET_LINUX
    vTaskDelete(NULL);
#endif
}

void app_main(void)
{
    ESP_LOGI(TAG, "🎉 ESP32 Hello World HTTP Client Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize networking
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_LOGI(TAG, "🌐 Connecting to WiFi...");
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "✅ Connected to WiFi! Starting HTTP client demo...");

#if CONFIG_IDF_TARGET_LINUX
    hello_world_task(NULL);
#else
    xTaskCreate(&hello_world_task, "hello_world_task", 8192, NULL, 5, NULL);
#endif
}