/**
 * @file wifi_task.h
 * @brief WiFi Management Task
 * 
 * This task handles WiFi connection management, monitoring,
 * and automatic reconnection. It uses the WiFi driver for
 * all hardware operations.
 */

#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "drivers/wifi_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi task events
 */
#define WIFI_TASK_CONNECTED_BIT     BIT0
#define WIFI_TASK_DISCONNECTED_BIT  BIT1
#define WIFI_TASK_FAILED_BIT        BIT2

/**
 * @brief WiFi task configuration
 */
typedef struct {
    char ssid[32];
    char password[64];
    int max_retries;
    int retry_delay_ms;
    bool auto_reconnect;
    int task_priority;
    int task_stack_size;
} wifi_task_config_t;

/**
 * @brief Default WiFi task configuration
 */
#define WIFI_TASK_CONFIG_DEFAULT() {           \
    .ssid = "",                                \
    .password = "",                            \
    .max_retries = 10,                         \
    .retry_delay_ms = 2000,                    \
    .auto_reconnect = true,                    \
    .task_priority = 5,                        \
    .task_stack_size = 4096                    \
}

/**
 * @brief Initialize and start WiFi management task
 * 
 * Creates the WiFi task that handles connection management.
 * 
 * @param config WiFi task configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t wifi_task_start(const wifi_task_config_t *config);

/**
 * @brief Stop WiFi management task
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_task_stop(void);

/**
 * @brief Get WiFi task event group handle
 * 
 * Use this to wait for WiFi events in other tasks.
 * 
 * @return Event group handle or NULL if task not started
 */
EventGroupHandle_t wifi_task_get_event_group(void);

/**
 * @brief Wait for WiFi connection
 * 
 * Blocks until WiFi is connected or timeout expires.
 * 
 * @param timeout_ms Timeout in milliseconds (portMAX_DELAY for infinite)
 * @return ESP_OK if connected, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t wifi_task_wait_connected(uint32_t timeout_ms);

/**
 * @brief Trigger manual reconnection
 * 
 * Forces a reconnection attempt even if auto-reconnect is disabled.
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_task_reconnect(void);

/**
 * @brief Get current WiFi connection info
 * 
 * @param info Pointer to info structure to fill
 * @return ESP_OK if connected, ESP_ERR_* otherwise
 */
esp_err_t wifi_task_get_info(wifi_info_t *info);

/**
 * @brief Check if WiFi task is running
 * 
 * @return true if task is running, false otherwise
 */
bool wifi_task_is_running(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_TASK_H