#include <Arduino.h>
#include <micro_ros_arduino.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/string.h>

// ---------- Globale micro-ROS Objekte ----------
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;

rcl_publisher_t pub_echo;         // gibt empfangene Kommandos zurück
rcl_publisher_t pub_heartbeat;    // sendet regelmäßigen "alive"-Ping
rcl_subscription_t sub_cmd;
rcl_timer_t timer;

rclc_executor_t executor;

// ---------- Nachrichten-Puffer ----------
static char in_buf[128];
static char out_buf[128];
static char hb_buf[64];

std_msgs__msg__String in_msg;
std_msgs__msg__String out_msg;
std_msgs__msg__String hb_msg;

// ---------- Hilfsfunktionen ----------
static inline uint64_t get_epoch_ns_or_zero() {
  // Neuer Signatur: gibt int64_t zurück, keine Argumente
  int64_t ns = rmw_uros_epoch_nanos();
  return (ns > 0) ? static_cast<uint64_t>(ns) : 0ULL;
}

// Echo-Callback: antwortet mit gleichem Inhalt auf Topic "echo"
void cmd_callback(const void *msgin) {
  const std_msgs__msg__String *msg = (const std_msgs__msg__String *)msgin;

  size_t n = msg->data.size;
  if (n >= sizeof(out_buf)) n = sizeof(out_buf) - 1;
  memcpy(out_buf, msg->data.data, n);
  out_buf[n] = '\0';

  out_msg.data.data = out_buf;
  out_msg.data.size = strlen(out_buf);
  out_msg.data.capacity = sizeof(out_buf);

  (void) rcl_publish(&pub_echo, &out_msg, nullptr);
}

// Heartbeat: sendet jede Sekunde einen kleinen Ping mit Zeitstempel
void timer_callback(rcl_timer_t * /*timer*/, int64_t /*last_call_time*/) {
  uint64_t ns = get_epoch_ns_or_zero();
  int written = snprintf(hb_buf, sizeof(hb_buf), "esp32 alive, t(ns)=%llu",
                         (unsigned long long)ns);
  if (written < 0) return;

  hb_msg.data.data = hb_buf;
  hb_msg.data.size = strnlen(hb_buf, sizeof(hb_buf));
  hb_msg.data.capacity = sizeof(hb_buf);

  (void) rcl_publish(&pub_heartbeat, &hb_msg, nullptr);
}

void setup() {
  // Serielle Schnittstelle für micro-ROS Serial-Transport
  Serial.begin(115200);
  delay(50);

  // WICHTIG: Damit diese Funktion sichtbar ist, in platformio.ini das Flag
  // -D MICRO_ROS_TRANSPORT_SERIAL setzen (siehe unten).
  set_microros_serial_transports(Serial);

  allocator = rcl_get_default_allocator();

  // micro-ROS Grundinitialisierung
  rclc_support_init(&support, 0, nullptr, &allocator);

  // Node
  rclc_node_init_default(
      &node, "esp32_node", "", &support);

  // Publisher: echo
  rclc_publisher_init_default(
      &pub_echo, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
      "echo");

  // Publisher: heartbeat
  rclc_publisher_init_default(
      &pub_heartbeat, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
      "heartbeat");

  // Subscription: cmd
  rclc_subscription_init_default(
      &sub_cmd, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
      "cmd");

  // Nachrichtenpuffer vorbereiten (für das Subscription-Objekt)
  in_msg.data.data = in_buf;
  in_msg.data.size = 0;
  in_msg.data.capacity = sizeof(in_buf);

  // Timer (1000 ms)
  rclc_timer_init_default(
      &timer, &support, RCL_MS_TO_NS(1000), timer_callback);

  // Executor (2 Handles: sub + timer)
  rclc_executor_init(&executor, &support.context, 2, &allocator);
  rclc_executor_add_subscription(&executor, &sub_cmd, &in_msg, &cmd_callback, ON_NEW_DATA);
  rclc_executor_add_timer(&executor, &timer);
}

void loop() {
  // Bearbeite Callbacks
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(5));
  delay(1);
}
