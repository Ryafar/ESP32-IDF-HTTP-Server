| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# ESP32 Hello World HTTP Client

A simple ESP32 HTTP client example that sends "Hello World" messages with dynamic data to your computer over WiFi.

## What This Example Does

This project demonstrates ESP32 HTTP client functionality by:

🎯 **Sending HTTP POST requests** from ESP32 to your computer  
📊 **Including dynamic changing data** in each message:
- Message counter (increments with each message)
- ESP32 uptime in milliseconds  
- System memory information (free heap, minimum heap)
- Random numbers and calculations
- Custom HTTP headers with ESP32 statistics

🔄 **Sends 5 messages total** with increasing delays (immediate, then 3s, 6s, 9s, 12s intervals)

## Quick Start

### 1. Set Up HTTP Server on Your Computer

Run the included Python server to receive ESP32 messages:
```bash
python simple_server.py
```

The server will:
- Listen on port 8000
- Display received messages with formatting
- Show changing data analysis
- Respond back to ESP32

### 2. Configure ESP32

1. **Set WiFi credentials:**
   ```bash
   idf.py menuconfig
   ```
   Navigate to: `Example Connection Configuration` → Set your WiFi SSID and password

2. **Update computer IP address** in `main/esp_http_client_example.c`:
   ```c
   .host = "192.168.1.13",  // CHANGE THIS TO YOUR COMPUTER'S IP ADDRESS
   ```

   Find your computer's IP:
   - Windows: `ipconfig`
   - Linux/Mac: `ifconfig` or `ip addr show`

### 3. Build and Flash

```bash
idf.py build
idf.py flash monitor
```

## Expected Output

**ESP32 Console:**
```
🚀 Starting ESP32 Hello World HTTP Client Demo
📡 Will send multiple messages with changing data...
Sending Hello World message #1 to your computer...
✅ SUCCESS! Message #1 sent - HTTP POST Status = 200
📊 Uptime: 5.234 seconds, Free heap: 245628 bytes
⏳ Waiting 3 seconds before sending message #2...
```

**Your Computer (simple_server.py):**
```
🎉 ESP32 MESSAGE #1 RECEIVED - 2025-09-25 14:30:25.123
📊 Message Tracking:
   🔢 ESP32 Message Counter: 1
   ⏱️  ESP32 Uptime: 5.234 seconds
   🌐 From IP: 192.168.1.X

🔍 CHANGING DATA DETECTED:
   📈    📋 Message Number: 1
   📈    ⏰ Uptime: 5.234 seconds (5234 ms total)
   📈    🎲 Random Value: 742
   📈    🧮 Calculation Result: 2 (2^1 simplified)
   📈    🔧 Free Heap: 245628 bytes
```

## Files Included

- **`main/esp_http_client_example.c`** - ESP32 HTTP client with hello world functionality
- **`simple_server.py`** - Python HTTP server to receive and display ESP32 messages  
- **`main/Kconfig.projbuild`** - Configuration options for WiFi credentials
- **`dependencies.lock`** - Locked component versions for reproducible builds

## Learning Objectives

This example demonstrates:
- ✅ ESP32 WiFi connection using `protocol_examples_common`
- ✅ HTTP POST requests with custom headers
- ✅ Dynamic message generation with system information
- ✅ Real-time data exchange between ESP32 and computer
- ✅ Proper error handling and logging
- ✅ ESP-IDF project structure and configuration

## Customization

**Send Different Data:**
Modify the `send_hello_world()` function in `esp_http_client_example.c` to include your own data.

**Change Message Frequency:**  
Adjust delays in `http_test_task()` function.

**Different Server:**
Update the server configuration in the ESP32 code or modify `simple_server.py`.

---

Perfect for learning ESP32 networking, HTTP clients, and WiFi communication! 🚀
