/**
 * @file wifi_driver.h
 * @brief WiFi Driver Interface
 * 
 * This driver provides a clean interface for WiFi operations
 * without any FreeRTOS tasks. It handles low-level WiFi setup
 * and connection management.
 */

#ifndef WIFI_DRIVER_H
#define WIFI_DRIVER_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi connection status
 */
typedef enum {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED
} wifi_status_t;

/**
 * @brief WiFi connection information
 */
typedef struct {
    char ssid[32];
    char ip_address[16];
    int8_t rssi;
    uint8_t channel;
    wifi_auth_mode_t auth_mode;
    bool is_connected;
} wifi_info_t;

/**
 * @brief WiFi event callback function type
 */
typedef void (*wifi_event_callback_t)(wifi_status_t status, const wifi_info_t *info);

/**
 * @brief Initialize WiFi driver
 * 
 * Sets up WiFi hardware and registers event handlers.
 * Does not start connection process.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t wifi_driver_init(void);

/**
 * @brief Start WiFi connection
 * 
 * Begins connection process to configured access point.
 * This is a non-blocking call.
 * 
 * @param ssid WiFi network name
 * @param password WiFi password
 * @param callback Event callback function (optional)
 * @return ESP_OK if connection started, ESP_ERR_* on failure
 */
esp_err_t wifi_driver_connect(const char *ssid, const char *password, wifi_event_callback_t callback);

/**
 * @brief Disconnect from WiFi
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_driver_disconnect(void);

/**
 * @brief Get current WiFi status
 * 
 * @return Current WiFi connection status
 */
wifi_status_t wifi_driver_get_status(void);

/**
 * @brief Get WiFi connection information
 * 
 * @param info Pointer to info structure to fill
 * @return ESP_OK if connected and info filled, ESP_ERR_* otherwise
 */
esp_err_t wifi_driver_get_info(wifi_info_t *info);

/**
 * @brief Check if WiFi is connected
 * 
 * @return true if connected, false otherwise
 */
bool wifi_driver_is_connected(void);

/**
 * @brief Wait for WiFi connection with timeout
 * 
 * Blocks until connected or timeout expires.
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return ESP_OK if connected, ESP_ERR_TIMEOUT if timeout
 */
esp_err_t wifi_driver_wait_connected(uint32_t timeout_ms);

/**
 * @brief Deinitialize WiFi driver
 * 
 * @return ESP_OK on success
 */
esp_err_t wifi_driver_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_DRIVER_H