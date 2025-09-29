# ESP32 Hello World HTTP Client Component
# 
# This component includes:
# - main.c: System initialization and application startup
# - esp32_http_client.c/.h: Generic, reusable HTTP client library
# - hello_world_app.c/.h: Hello World application using the HTTP client

# Register component with all required dependencies
idf_component_register(
    SRCS 
        "main.c"
        "esp32_http_client.c"
        "hello_world_app.c"
        # Previous files kept for reference:
        # "hello_world_main.c"
        # "http_hello_client.c"
    INCLUDE_DIRS 
        "."
        "config"
    REQUIRES
        esp_wifi
        esp_netif  
        esp_event
        esp_http_client
        esp_timer
        freertos
        nvs_flash
        log
)
