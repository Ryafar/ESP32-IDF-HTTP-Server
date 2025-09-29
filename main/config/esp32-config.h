/**
 * @file esp32-config.h
 * @brief ESP32 Hardware Configuration for Soil Moisture Sensor Project
 * 
 * This file contains all hardware-specific configurations including
 * pin assignments, ADC settings, and project constants.
 */

#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

#include "credentials.h"

// ============================================================================
// ADC Configuration
// ============================================================================



// ============================================================================
// Sensor Configuration
// ============================================================================


// ============================================================================
// Task Configuration
// ============================================================================

// ============================================================================
// Logging Configuration
// ============================================================================

#define SOIL_ENABLE_DETAILED_LOGGING    1
#define SOIL_LOG_LEVEL          ESP_LOG_INFO

// ============================================================================
// Calibration Configuration
// ============================================================================

#define SOIL_AUTO_CALIBRATION_ENABLE    0
#define SOIL_CALIBRATION_TIMEOUT_MS     10000
#define SOIL_CALIBRATION_SAMPLES        10

// ============================================================================
// WiFi Configuration
// ============================================================================

#define WIFI_MAX_RETRY          10
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

// ============================================================================
// HTTP Configuration
// ============================================================================

#define HTTP_SERVER_IP          "192.168.1.13"    // Your PC's IP address
#define HTTP_SERVER_PORT        8000              // Python server port
#define HTTP_ENDPOINT           "/hello"          // Hello World endpoint
#define HTTP_TIMEOUT_MS         5000
#define HTTP_MAX_RETRIES        3

#endif // ESP32_CONFIG_H




