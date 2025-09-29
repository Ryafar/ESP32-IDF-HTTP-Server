/**
 * @file hello_world_task.h
 * @brief Hello World Application Task
 * 
 * This task handles the Hello World application logic,
 * sending periodic HTTP messages with system information.
 * It uses the HTTP driver for communication.
 */

#ifndef HELLO_WORLD_TASK_H
#define HELLO_WORLD_TASK_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hello World task configuration
 */
typedef struct {
    char target_host[64];           /**< Target server IP/hostname */
    int target_port;                /**< Target server port */
    char target_path[128];          /**< HTTP endpoint path */
    int message_interval_ms;        /**< Interval between messages */
    bool include_system_info;       /**< Include ESP32 system info */
    bool include_random_data;       /**< Include random data */
    bool auto_mode;                 /**< Automatic continuous sending */
    int task_priority;              /**< Task priority */
    int task_stack_size;            /**< Task stack size */
} hello_world_task_config_t;

/**
 * @brief Hello World task statistics
 */
typedef struct {
    uint32_t messages_sent;
    uint32_t messages_failed;
    uint32_t uptime_at_last_message;
    uint64_t last_message_time;
    int last_response_code;
} hello_world_task_stats_t;

/**
 * @brief Default Hello World task configuration
 */
#define HELLO_WORLD_TASK_CONFIG_DEFAULT() {    \
    .target_host = "192.168.1.100",            \
    .target_port = 8000,                       \
    .target_path = "/hello",                   \
    .message_interval_ms = 5000,               \
    .include_system_info = true,               \
    .include_random_data = true,               \
    .auto_mode = false,                        \
    .task_priority = 3,                        \
    .task_stack_size = 8192                    \
}

/**
 * @brief Initialize and start Hello World task
 * 
 * Creates the Hello World task that handles periodic messaging.
 * 
 * @param config Task configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_task_start(const hello_world_task_config_t *config);

/**
 * @brief Stop Hello World task
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_stop(void);

/**
 * @brief Send a single message immediately
 * 
 * Sends one message without affecting auto mode.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_task_send_message(void);

/**
 * @brief Send a custom message
 * 
 * @param custom_message Custom message content
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_task_send_custom_message(const char *custom_message);

/**
 * @brief Start automatic message sending
 * 
 * Starts continuous message sending at configured interval.
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_start_auto_mode(void);

/**
 * @brief Stop automatic message sending
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_stop_auto_mode(void);

/**
 * @brief Send a sequence of test messages
 * 
 * @param count Number of messages to send
 * @param base_delay_ms Base delay between messages
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_send_sequence(int count, int base_delay_ms);

/**
 * @brief Get task statistics
 * 
 * @param stats Pointer to statistics structure
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_get_stats(hello_world_task_stats_t *stats);

/**
 * @brief Reset task statistics
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_reset_stats(void);

/**
 * @brief Update task configuration
 * 
 * @param config New configuration
 * @return ESP_OK on success
 */
esp_err_t hello_world_task_update_config(const hello_world_task_config_t *config);

/**
 * @brief Check if task is running
 * 
 * @return true if task is running, false otherwise
 */
bool hello_world_task_is_running(void);

/**
 * @brief Check if auto mode is active
 * 
 * @return true if auto mode is active, false otherwise
 */
bool hello_world_task_is_auto_mode(void);

#ifdef __cplusplus
}
#endif

#endif // HELLO_WORLD_TASK_H