/**
 * @file app_init.h
 * @brief Application Initialization Header
 * 
 * This module handles the initialization of all application components,
 * drivers, and FreeRTOS tasks. It provides a clean interface for
 * setting up the entire application.
 */

#ifndef APP_INIT_H
#define APP_INIT_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all system components
 * 
 * This function initializes all system components in the correct order:
 * 1. Hardware drivers (WiFi, HTTP client)
 * 2. System services
 * 3. Application tasks
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_init_all(void);

/**
 * @brief Initialize hardware drivers
 * 
 * Initializes all hardware drivers without starting tasks.
 * Drivers provide clean interfaces for hardware access.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_init_drivers(void);

/**
 * @brief Initialize and start all application tasks
 * 
 * Creates and starts all FreeRTOS tasks for the application.
 * Tasks handle the application logic and coordination.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_init_tasks(void);

/**
 * @brief Initialize system services
 * 
 * Sets up system-level services like NVS, networking, etc.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t app_init_system(void);

/**
 * @brief Get initialization status
 * 
 * @return true if all components are initialized, false otherwise
 */
bool app_is_initialized(void);

/**
 * @brief Deinitialize application
 * 
 * Cleanly shuts down all tasks and drivers.
 * 
 * @return ESP_OK on success
 */
esp_err_t app_deinit_all(void);

#ifdef __cplusplus
}
#endif

#endif // APP_INIT_H