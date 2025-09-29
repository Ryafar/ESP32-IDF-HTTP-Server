/**
 * @file main.c
 * @brief ESP32 Application Entry Point
 * 
 * This is the front door of the application. It only handles
 * the basic system startup and delegates all initialization
 * to the app_init module.
 * 
 * Architecture:
 * main.c -> app_init.c -> drivers/ + tasks/
 */

#include "esp_log.h"
#include "app_init.h"

static const char *TAG = "MAIN";

/**
 * @brief Main application entry point
 * 
 * This is the front door - keeps things simple and clean.
 * All the real work is delegated to the app_init module.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🚀 ESP32 Hello World HTTP Client");
    ESP_LOGI(TAG, "🏛️  Modular Architecture Starting...");
    ESP_LOGI(TAG, "");
    
    // Initialize everything through the app_init module
    esp_err_t ret = app_init_all();
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "✅ Application initialized successfully!");
        ESP_LOGI(TAG, "🎯 System is now running...");
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "📋 Architecture:");
        ESP_LOGI(TAG, "   📂 main.c        - Entry point (this file)");
        ESP_LOGI(TAG, "   📂 app_init.c    - Initialization coordinator");
        ESP_LOGI(TAG, "   📂 drivers/      - Hardware interfaces (no tasks)");
        ESP_LOGI(TAG, "   📂 tasks/        - FreeRTOS application logic");
        ESP_LOGI(TAG, "   📂 config/       - Configuration files");
        ESP_LOGI(TAG, "");
    } else {
        ESP_LOGE(TAG, "❌ Application initialization failed: %s", esp_err_to_name(ret));
        ESP_LOGE(TAG, "🔄 System will restart in 5 seconds...");
        
        // Wait a bit then restart
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    }
    
    // Main task is done - everything runs in dedicated tasks now
    ESP_LOGI(TAG, "🏁 Main task completed - application running in background tasks");
}