# ESP32 Hello World HTTP Client Component
# 
# This component includes:
# - hello_world_main.c: Main application demonstrating the HTTP client
# - http_hello_client.c/.h: Modular HTTP client library
# - esp_http_client_example.c: Original complex example (kept for reference)

set(requires "")
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires esp_stubs esp_event esp-tls esp_http_client protocol_examples_common nvs_flash)
endif()

# Register component with both new modular files and original example
# Note: The build system will use hello_world_main.c as the main entry point
idf_component_register(
    SRCS 
        "hello_world_main.c"          # New simplified main file using modular client
        "http_hello_client.c"         # Modular HTTP client implementation
        # "esp_http_client_example.c" # Original complex example (commented out to avoid conflicts)
    INCLUDE_DIRS 
        "."                           # Include current directory for headers
    REQUIRES 
        ${requires}
    EMBED_TXTFILES 
        howsmyssl_com_root_cert.pem   # Certificates for HTTPS (kept for compatibility)
        postman_root_cert.pem
)
