/**
 * @file main.c
 * @brief ESP32 Main Entry Point
 * 
 * This file contains only system initialization and application startup.
 * The actual application logic is in separate modules.
 */

#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// Include application modules
#include "hello_world_app.h"

// Include configuration and credentials
#include "config/esp32-config.h"

// WiFi configuration from credentials file
#define WIFI_MAXIMUM_RETRY  WIFI_MAX_RETRY

static const char *TAG = "MAIN";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed the maximum number of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

/**
 * @brief WiFi event handler
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP (%d/%d)", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 */
static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "✅ NVS initialized");
    return ret;
}

/**
 * @brief Initialize networking stack
 */
static esp_err_t init_networking(void)
{
    // Note: esp_netif_init() and esp_event_loop_create_default() are now called in connect_wifi()
    ESP_LOGI(TAG, "✅ Networking will be initialized during WiFi connection");
    return ESP_OK;
}

/**
 * @brief Connect to WiFi
 */
static esp_err_t connect_wifi(void)
{
    ESP_LOGI(TAG, "🌐 Connecting to WiFi...");
    
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "WiFi init finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "✅ Connected to WiFi SSID:%s", WIFI_SSID);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "❌ Failed to connect to SSID:%s", WIFI_SSID);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "❌ UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

/**
 * @brief Application task that runs the Hello World demo
 */
static void app_main_task(void *pvParameters)
{
    ESP_LOGI(TAG, "🚀 Starting Hello World Application...");
    
    // Configure Hello World application using config file values
    hello_world_config_t app_config = HELLO_WORLD_CONFIG_DEFAULT();
    
    // Use configuration from esp32-config.h
    strcpy(app_config.target_host, HTTP_SERVER_IP);
    app_config.target_port = HTTP_SERVER_PORT;
    strcpy(app_config.target_path, HTTP_ENDPOINT);
    app_config.include_system_info = true;
    app_config.include_random_data = true;
    app_config.message_interval_ms = 5000;  // 5 seconds for auto mode
    
    // Initialize the Hello World application
    esp_err_t ret = hello_world_app_init(&app_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Failed to initialize Hello World app: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "📡 Hello World App initialized");
    ESP_LOGI(TAG, "🎯 Target: %s:%d%s", app_config.target_host, app_config.target_port, app_config.target_path);
    ESP_LOGI(TAG, "💡 Make sure simple_server.py is running on your computer!");
    
    // Demonstrate different ways to use the application
    ESP_LOGI(TAG, "\n=== Demo 1: Individual Messages ===");
    
    // Send single message
    ret = hello_world_app_send_message();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ First message sent");
    }
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Send custom message
    ret = hello_world_app_send_custom_message("This is a custom message from the new modular ESP32 app! 🌟");
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Custom message sent");
    }
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Send demo sequence
    ESP_LOGI(TAG, "\n=== Demo 2: Automated Sequence ===");
    ret = hello_world_app_send_demo_sequence(3, 2000);  // 3 messages, 2s base delay
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Demo sequence completed");
    }
    
    // Show statistics
    hello_world_stats_t stats;
    ret = hello_world_app_get_stats(&stats);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "\n📊 Final Statistics:");
        ESP_LOGI(TAG, "   📈 Messages sent: %d", stats.messages_sent);
        ESP_LOGI(TAG, "   ❌ Messages failed: %d", stats.messages_failed);
        ESP_LOGI(TAG, "   ⏰ Last message time: %llu ms", stats.last_message_time);
        ESP_LOGI(TAG, "   🕐 ESP32 uptime: %lu ms", (unsigned long)stats.uptime_at_last_message);
    }
    
    // Optional: Start auto mode for continuous sending
    ESP_LOGI(TAG, "\n=== Demo 3: Auto Mode (Optional) ===");
    ESP_LOGI(TAG, "💡 Uncomment the code below to enable continuous message sending");
    
    /*
    ESP_LOGI(TAG, "🤖 Starting auto mode - messages every %d ms", app_config.message_interval_ms);
    ret = hello_world_app_start_auto_mode();
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Auto mode started - will send messages continuously");
        ESP_LOGI(TAG, "🛑 Auto mode will run forever (restart ESP32 to stop)");
        
        // Let it run for a while, then stop (optional)
        // vTaskDelay(pdMS_TO_TICKS(30000));  // Run for 30 seconds
        // hello_world_app_stop_auto_mode();
        // ESP_LOGI(TAG, "🛑 Auto mode stopped");
    }
    */
    
    // Clean up
    hello_world_app_deinit();
    
    ESP_LOGI(TAG, "🏁 Hello World Demo completed!");
    ESP_LOGI(TAG, "💡 Check your computer to see all the messages received");
    ESP_LOGI(TAG, "🔄 Restart the ESP32 to run the demo again");
    
    vTaskDelete(NULL);
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🎉 ESP32 Hello World HTTP Client Starting...");
    ESP_LOGI(TAG, "📋 System Initialization Phase");
    
    // System initialization sequence
    ESP_LOGI(TAG, "1️⃣ Initializing NVS...");
    init_nvs();
    
    ESP_LOGI(TAG, "2️⃣ Initializing networking...");
    init_networking();
    
    ESP_LOGI(TAG, "3️⃣ Connecting to WiFi...");
    connect_wifi();
    
    ESP_LOGI(TAG, "✅ System initialization completed successfully!");
    ESP_LOGI(TAG, "🚀 Starting application...");
    
    // Start the application task
    BaseType_t result = xTaskCreate(app_main_task, "app_main", 8192, NULL, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG, "❌ Failed to create application task");
        return;
    }
    
    ESP_LOGI(TAG, "📱 Application task started successfully");
}