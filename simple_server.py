#!/usr/bin/env python3
"""
Simple HTTP Server to receive ESP32 Hello World messages
Run with: python simple_server.py
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import json
from datetime import datetime


class ESP32RequestHandler(BaseHTTPRequestHandler):
    message_count = 0  # Class variable to track total messages

    def do_POST(self):
        ESP32RequestHandler.message_count += 1

        # Get the content length
        content_length = int(self.headers.get('Content-Length', 0))

        # Read the POST data
        post_data = self.rfile.read(content_length).decode('utf-8')

        # Extract ESP32 custom headers
        esp32_counter = self.headers.get('X-ESP32-Message-Counter', 'N/A')
        esp32_uptime = self.headers.get('X-ESP32-Uptime-MS', 'N/A')
        uptime_seconds = float(esp32_uptime) / \
            1000 if esp32_uptime != 'N/A' else 0

        # Print received message with enhanced formatting
        timestamp = datetime.now().strftime(
            "%Y-%m-%d %H:%M:%S.%f")[:-3]  # Include milliseconds
        print(f"\n{'='*80}")
        print(
            f"🎉 ESP32 MESSAGE #{ESP32RequestHandler.message_count} RECEIVED - {timestamp}")
        print(f"{'='*80}")
        print(f"📊 Message Tracking:")
        print(f"   🔢 ESP32 Message Counter: {esp32_counter}")
        print(
            f"   📥 Server Received Count: {ESP32RequestHandler.message_count}")
        print(f"   ⏱️  ESP32 Uptime: {uptime_seconds:.3f} seconds")
        print(f"   🌐 From IP: {self.client_address[0]}")
        print(f"   📍 Endpoint: {self.path}")
        print(f"   📋 User-Agent: {self.headers.get('User-Agent', 'Unknown')}")

        # Show all custom headers
        esp32_headers = {k: v for k, v in self.headers.items()
                         if k.startswith('X-ESP32-')}
        if esp32_headers:
            print(f"   🏷️  ESP32 Custom Headers: {esp32_headers}")

        print(f"\n📨 MESSAGE CONTENT:")
        print("─" * 80)
        print(post_data)
        print("─" * 80)
        print(f"📏 Message Length: {len(post_data)} bytes")

        # Analyze content for changing data
        lines = post_data.split('\n')
        changing_data = []
        for line in lines:
            if any(keyword in line.lower() for keyword in ['counter', 'uptime', 'random', 'hash', 'heap', 'message number']):
                changing_data.append(line.strip())

        if changing_data:
            print(f"\n🔍 CHANGING DATA DETECTED:")
            for data in changing_data[:5]:  # Show first 5 changing values
                print(f"   📈 {data}")

        print("="*80 + "\n")

        # Send response back to ESP32
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        response_message = (f"✅ Message #{esp32_counter} received successfully by your computer!\n"
                            f"📊 Server has now received {ESP32RequestHandler.message_count} total messages.\n"
                            f"⏰ Received at: {timestamp}\n"
                            f"🎯 ESP32 uptime: {uptime_seconds:.3f} seconds")
        self.wfile.write(response_message.encode('utf-8'))

    def do_GET(self):
        # Handle GET requests too
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(
            f"\n📡 GET request from {self.client_address[0]} to {self.path} at {timestamp}")

        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()

        html_response = f"""
        <html>
        <body>
            <h1>🎉 ESP32 HTTP Server is Running!</h1>
            <p>✅ Server is ready to receive ESP32 messages</p>
            <p>⏰ Current time: {timestamp}</p>
            <p>💡 Check the console for POST messages from your ESP32</p>
        </body>
        </html>
        """
        self.wfile.write(html_response.encode('utf-8'))

    def log_message(self, format, *args):
        # Suppress default logging to keep output clean
        pass


def run_server():
    server_address = ('', 8000)
    httpd = HTTPServer(server_address, ESP32RequestHandler)

    print("🚀 ESP32 Message Server Starting...")
    print(f"📡 Listening on port 8000")
    print(f"🌐 Your computer's IP should be used in ESP32 code")
    print(f"💡 You can also test by visiting http://localhost:8000 in your browser")
    print(f"🔍 Waiting for ESP32 messages...\n")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\n👋 Server stopped by user")
        httpd.server_close()


if __name__ == '__main__':
    run_server()
