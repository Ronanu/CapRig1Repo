#include <Arduino.h>
#include <WiFi.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/uint64.h>
#include <std_msgs/msg/string.h>

// WiFi credentials
#define WIFI_SSID   "Magenta-770707"
#define WIFI_PASS   "6e632mgakgu9"

// micro-ROS agent target (Windows host)
#define AGENT_IP    "172.27.64.1"
#define AGENT_PORT  8888

static const uint32_t HEARTBEAT_MS = 1000;
static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 20000;   // 20s
static const uint32_t WIFI_RETRY_DELAY_MS     = 1000;    // 1s

rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rcl_publisher_t pub_heartbeat;
rcl_publisher_t pub_echo_resp;
rcl_subscription_t sub_echo_req;
rcl_timer_t timer_heartbeat;
rclc_executor_t executor;

std_msgs__msg__UInt64 hb_msg;
std_msgs__msg__String echo_resp_msg;
std_msgs__msg__String echo_req_msg;

char echo_buffer[256];

static bool wifi_connect_blocking()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  const uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - t0 > WIFI_CONNECT_TIMEOUT_MS) {
      Serial.println("[WiFi] Timeout. Restarting connection attempt...");
      WiFi.disconnect(true, true);
      delay(WIFI_RETRY_DELAY_MS);
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      // reset timer
      return wifi_connect_blocking(); // einfacher Re-Try (tail recursion)
    }
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("[WiFi] Connected. IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WiFi] RSSI: ");
  Serial.println(WiFi.RSSI());
  return true;
}

void echo_request_cb(const void* msgin)
{
  const auto* in = (const std_msgs__msg__String*) msgin;
  uint64_t esp_us = (uint64_t) esp_timer_get_time();

  size_t len_in = (in && in->data.data) ? strlen(in->data.data) : 0;
  if (len_in > 150) len_in = 150;

  int n = snprintf(echo_buffer, sizeof(echo_buffer),
                   "{\"esp_us\":%llu,\"echo\":\"%.*s\"}",
                   (unsigned long long) esp_us, (int)len_in,
                   (in && in->data.data) ? in->data.data : "");
  if (n < 0) return;

  echo_resp_msg.data.data = echo_buffer;
  echo_resp_msg.data.size = (size_t) n;
  echo_resp_msg.data.capacity = sizeof(echo_buffer);
  rcl_publish(&pub_echo_resp, &echo_resp_msg, NULL);
}

void heartbeat_timer_cb(rcl_timer_t*, int64_t)
{
  hb_msg.data = (uint64_t) esp_timer_get_time();
  rcl_publish(&pub_heartbeat, &hb_msg, NULL);
}

void setup()
{
  delay(300);
  Serial.begin(115200);
  Serial.println("\n=== Boot ===");

  // 1) WLAN verbinden (sichtbar & robust)
  wifi_connect_blocking();

  // 2) micro-ROS WiFi+UDP Transport setzen
  IPAddress agent_ip; agent_ip.fromString(AGENT_IP);
  set_microros_wifi_transports(WIFI_SSID, WIFI_PASS, agent_ip, AGENT_PORT);

  // 3) rcl/rclc Setup
  allocator = rcl_get_default_allocator();

  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_default(&node, "device_1", "", &support);

  rclc_publisher_init_default(
    &pub_heartbeat, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, UInt64),
    "/esp/heartbeat_us");

  rclc_publisher_init_default(
    &pub_echo_resp, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "/esp/echo_response");

  rclc_subscription_init_default(
    &sub_echo_req, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
    "/esp/echo_request");

  rclc_timer_init_default(
    &timer_heartbeat, &support,
    RCL_MS_TO_NS(HEARTBEAT_MS),
    heartbeat_timer_cb);

  rclc_executor_init(&executor, &support.context, 2, &allocator);
  rclc_executor_add_subscription(&executor, &sub_echo_req, &echo_req_msg, &echo_request_cb, ON_NEW_DATA);
  rclc_executor_add_timer(&executor, &timer_heartbeat);
}

void loop()
{
  // optional: Reconnect-Guard (falls WiFi unerwartet weg ist)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Lost connection. Reconnecting...");
    wifi_connect_blocking();
    // Transport bleibt gültig; Agent muss erreichbar sein.
  }

  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(50));
  delay(10);
}
