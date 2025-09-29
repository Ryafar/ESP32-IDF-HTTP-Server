# ESP32 Modular HTTP Client Component
# 
# Clean Architecture - Currently building with existing files only
# TODO: Implement driver and task .c files

# Register component with existing files only
idf_component_register(
    SRCS 
        # Core files (existing)
        "main.c"                        # Current working main
        "app_init.c"                    # Initialization coordinator
        
        # Existing working files (keep for now)
        "esp32_http_client.c"           # Current HTTP client
        "hello_world_app.c"             # Current application
        
        # TODO: Add these when implemented:
        # "drivers/wifi_driver.c"
        # "drivers/http_driver.c"
        # "tasks/wifi_task.c"
        # "tasks/hello_world_task.c"
        
    INCLUDE_DIRS 
        "."                             # Root directory
        "config"                        # Configuration files
        "drivers"                       # Driver headers (headers only for now)
        "tasks"                         # Task headers (headers only for now)
        
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
