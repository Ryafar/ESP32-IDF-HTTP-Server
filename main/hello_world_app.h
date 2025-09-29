/**
 * @file hello_world_app.h
 * @brief Hello World Application using Generic HTTP Client
 * 
 * Application-specific logic for sending "Hello World" messages
 * with dynamic ESP32 system information.
 */

#ifndef HELLO_WORLD_APP_H
#define HELLO_WORLD_APP_H

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Hello World application configuration
 */
typedef struct {
    char target_host[64];       /**< Target server host */
    int target_port;            /**< Target server port */
    char target_path[128];      /**< Target path on server */
    bool include_system_info;   /**< Include ESP32 system information */
    bool include_random_data;   /**< Include random data for verification */
    int message_interval_ms;    /**< Interval between messages (for auto mode) */
} hello_world_config_t;

/**
 * @brief Hello World application statistics
 */
typedef struct {
    int messages_sent;          /**< Total hello world messages sent */
    int messages_failed;        /**< Failed messages */
    uint64_t last_message_time; /**< Timestamp of last message */
    uint32_t uptime_at_last_message; /**< ESP32 uptime when last message was sent */
} hello_world_stats_t;

/**
 * @brief Default Hello World configuration
 */
#define HELLO_WORLD_CONFIG_DEFAULT() {              \
    .target_host = "192.168.1.100",                \
    .target_port = 8000,                           \
    .target_path = "/hello",                       \
    .include_system_info = true,                   \
    .include_random_data = true,                   \
    .message_interval_ms = 5000                    \
}

/**
 * @brief Initialize the Hello World application
 * 
 * This sets up the HTTP client and prepares for sending messages.
 * 
 * @param config Application configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_init(const hello_world_config_t *config);

/**
 * @brief Send a single Hello World message
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_send_message(void);

/**
 * @brief Send a Hello World message with custom content
 * 
 * @param custom_message Additional custom content to include
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_send_custom_message(const char *custom_message);

/**
 * @brief Send a sequence of Hello World messages for demonstration
 * 
 * @param count Number of messages to send
 * @param base_delay_ms Base delay between messages (will increase progressively)
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_send_demo_sequence(int count, int base_delay_ms);

/**
 * @brief Get Hello World application statistics
 * 
 * @param stats Pointer to statistics structure to fill
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_get_stats(hello_world_stats_t *stats);

/**
 * @brief Update application configuration
 * 
 * @param config New configuration
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_update_config(const hello_world_config_t *config);

/**
 * @brief Start automatic message sending (runs in background task)
 * 
 * Sends messages at configured intervals until stopped.
 * 
 * @return ESP_OK on success, ESP_ERR_* on failure
 */
esp_err_t hello_world_app_start_auto_mode(void);

/**
 * @brief Stop automatic message sending
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_app_stop_auto_mode(void);

/**
 * @brief Check if auto mode is running
 * 
 * @return true if auto mode is active, false otherwise
 */
bool hello_world_app_is_auto_mode_running(void);

/**
 * @brief Deinitialize the Hello World application
 * 
 * @return ESP_OK on success
 */
esp_err_t hello_world_app_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // HELLO_WORLD_APP_H