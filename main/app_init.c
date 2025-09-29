/**
 * @file app_init.c
 * @brief Application Initialization Implementation
 * 
 * This module coordinates the initialization of all application
 * components in the correct order: system -> drivers -> tasks.
 */

#include "app_init.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

// Include driver interfaces (TODO: Uncomment when implemented)
// #include "drivers/wifi_driver.h"
// #include "drivers/http_driver.h"

// Include task interfaces (TODO: Uncomment when implemented)  
// #include "tasks/wifi_task.h"
// #include "tasks/hello_world_task.h"

// For now, use existing modules
#include "esp32_http_client.h"
#include "hello_world_app.h"

// Include configuration
#include "config/credentials.h"
#include "config/esp32-config.h"

static const char *TAG = "APP_INIT";

// Global initialization state
static bool g_system_initialized = false;
static bool g_drivers_initialized = false;
static bool g_tasks_initialized = false;

esp_err_t app_init_system(void)
{
    ESP_LOGI(TAG, "🔧 Initializing system services...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "   ✅ NVS initialized");
    
    // Initialize networking
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "   ✅ Network stack initialized");
    
    g_system_initialized = true;
    ESP_LOGI(TAG, "✅ System services initialized successfully");
    
    return ESP_OK;
}

esp_err_t app_init_drivers(void)
{
    ESP_LOGI(TAG, "🔧 Initializing hardware drivers...");
    
    if (!g_system_initialized) {
        ESP_LOGE(TAG, "❌ System not initialized - call app_init_system() first");
        return ESP_ERR_INVALID_STATE;
    }
    
    // TODO: Replace with modular drivers when implemented
    // For now, drivers are initialized within the existing modules
    ESP_LOGI(TAG, "   ✅ Using existing integrated drivers");
    
    g_drivers_initialized = true;
    ESP_LOGI(TAG, "✅ Hardware drivers initialized successfully");
    
    return ESP_OK;
}

esp_err_t app_init_tasks(void)
{
    ESP_LOGI(TAG, "🔧 Starting application tasks...");
    
    if (!g_drivers_initialized) {
        ESP_LOGE(TAG, "❌ Drivers not initialized - call app_init_drivers() first");
        return ESP_ERR_INVALID_STATE;
    }
    
    // TODO: Replace with modular tasks when implemented
    // For now, use existing Hello World application
    
    // Configure Hello World application using existing module
    hello_world_config_t hello_config = HELLO_WORLD_CONFIG_DEFAULT();
    strncpy(hello_config.target_host, HTTP_SERVER_IP, sizeof(hello_config.target_host) - 1);
    hello_config.target_port = HTTP_SERVER_PORT;
    strncpy(hello_config.target_path, HTTP_ENDPOINT, sizeof(hello_config.target_path) - 1);
    hello_config.include_system_info = true;
    hello_config.include_random_data = true;
    hello_config.message_interval_ms = 5000;
    
    esp_err_t ret = hello_world_app_init(&hello_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "❌ Hello World app initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "   ✅ Hello World application initialized");
    
    g_tasks_initialized = true;
    ESP_LOGI(TAG, "✅ Application tasks started successfully");
    
    // Demonstrate the Hello World functionality
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🎯 Target: %s:%d%s", HTTP_SERVER_IP, HTTP_SERVER_PORT, HTTP_ENDPOINT);
    ESP_LOGI(TAG, "💡 Make sure simple_server.py is running on your computer!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🚀 Starting Hello World demonstration...");
    
    // Send a few test messages using existing module
    hello_world_app_send_message();
    vTaskDelay(pdMS_TO_TICKS(2000));
    hello_world_app_send_custom_message("Modular architecture test message! 🎉");
    vTaskDelay(pdMS_TO_TICKS(2000));
    hello_world_app_send_demo_sequence(3, 1500);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "✨ Demo completed! System is ready for operation.");
    
    return ESP_OK;
}

esp_err_t app_init_all(void)
{
    ESP_LOGI(TAG, "🚀 Starting complete application initialization...");
    ESP_LOGI(TAG, "");
    
    // Initialize in correct order
    esp_err_t ret;
    
    // 1. System services
    ret = app_init_system();
    if (ret != ESP_OK) return ret;
    
    // 2. Hardware drivers
    ret = app_init_drivers();
    if (ret != ESP_OK) return ret;
    
    // 3. Application tasks
    ret = app_init_tasks();
    if (ret != ESP_OK) return ret;
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🎉 Complete application initialization successful!");
    return ESP_OK;
}

bool app_is_initialized(void)
{
    return g_system_initialized && g_drivers_initialized && g_tasks_initialized;
}

esp_err_t app_deinit_all(void)
{
    ESP_LOGI(TAG, "🔄 Shutting down application...");
    
    if (g_tasks_initialized) {
        // TODO: Replace with modular task shutdown when implemented
        hello_world_app_deinit();
        g_tasks_initialized = false;
    }
    
    if (g_drivers_initialized) {
        // TODO: Replace with modular driver shutdown when implemented  
        g_drivers_initialized = false;
    }
    
    g_system_initialized = false;
    
    ESP_LOGI(TAG, "✅ Application shutdown complete");
    return ESP_OK;
}