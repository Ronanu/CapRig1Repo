#include <Arduino.h>
#include <micro_ros_platformio.h>    // Header aus micro_ros_platformio

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

// --- micro-ROS Objekte ---
static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_node_t node;

static rcl_publisher_t pub_echo;
static rcl_publisher_t pub_heartbeat;
static rcl_subscription_t sub_cmd;
static rcl_timer_t timer;
static rclc_executor_t executor;

// --- Puffer ---
static char in_buf[128], out_buf[128], hb_buf[64];
static std_msgs__msg__String in_msg, out_msg, hb_msg;

// Zeit vom Agent (falls synchronisiert), sonst 0
static inline uint64_t epoch_ns_or_zero() {
  int64_t ns = rmw_uros_epoch_nanos();     // neue Signatur: keine Args
  return (ns > 0) ? (uint64_t)ns : 0ULL;
}

static void cmd_callback(const void *msgin) {
  const auto *msg = static_cast<const std_msgs__msg__String *>(msgin);
  size_t n = msg->data.size; if (n >= sizeof(out_buf)) n = sizeof(out_buf)-1;
  memcpy(out_buf, msg->data.data, n); out_buf[n] = '\0';

  out_msg.data.data = out_buf;
  out_msg.data.size = strlen(out_buf);
  out_msg.data.capacity = sizeof(out_buf);
  (void) rcl_publish(&pub_echo, &out_msg, nullptr);
}

static void timer_callback(rcl_timer_t*, int64_t) {
  uint64_t ns = epoch_ns_or_zero();
  (void)snprintf(hb_buf, sizeof(hb_buf), "esp32 alive, t(ns)=%llu",
                 (unsigned long long)ns);
  hb_msg.data.data = hb_buf;
  hb_msg.data.size = strnlen(hb_buf, sizeof(hb_buf));
  hb_msg.data.capacity = sizeof(hb_buf);
  (void) rcl_publish(&pub_heartbeat, &hb_msg, nullptr);
}

void setup() {
  // USB-Serial des ESP32
  Serial.begin(115200);
  delay(50);

  // Transport aus micro_ros_platformio (Serial)
  set_microros_serial_transports(Serial);   // kommt mit der Lib

  allocator = rcl_get_default_allocator();
  rclc_support_init(&support, 0, nullptr, &allocator);

  rclc_node_init_default(&node, "esp32_node", "", &support);

  rclc_publisher_init_default(
      &pub_echo, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "echo");

  rclc_publisher_init_default(
      &pub_heartbeat, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "heartbeat");

  rclc_subscription_init_default(
      &sub_cmd, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String), "cmd");

  in_msg.data.data = in_buf; in_msg.data.size = 0; in_msg.data.capacity = sizeof(in_buf);
  out_msg.data.data = out_buf; out_msg.data.size = 0; out_msg.data.capacity = sizeof(out_buf);
  hb_msg.data.data  = hb_buf;  hb_msg.data.size  = 0; hb_msg.data.capacity  = sizeof(hb_buf);

  rcl_clock_type_t clock_type = RCL_STEADY_TIME;
  rclc_timer_init_default2(&timer, &support, RCL_MS_TO_NS(1000), timer_callback, clock_type);

  rclc_executor_init(&executor, &support.context, 2, &allocator);
  rclc_executor_add_subscription(&executor, &sub_cmd, &in_msg, &cmd_callback, ON_NEW_DATA);
  rclc_executor_add_timer(&executor, &timer);
}

void loop() {
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(5));
  delay(1);
}
