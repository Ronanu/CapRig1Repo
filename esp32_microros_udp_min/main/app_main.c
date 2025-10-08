#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/sockets.h"

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rmw_microros/rmw_microros.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/string.h>

#include "sdkconfig.h"

static const char *TAG = "APP";

static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected; reconnecting...");
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    strncpy((char*)wifi_config.sta.ssid, CONFIG_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi‑Fi init done, connecting to %s", CONFIG_WIFI_SSID);
}

typedef enum {
    STATE_NET_DOWN = 0,
    STATE_AGENT_WAIT,
    STATE_ROS_OK,
} app_state_t;

static app_state_t s_state = STATE_NET_DOWN;

static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_node_t node;
static rcl_publisher_t pub_heartbeat;
static rcl_subscription_t sub_echo;
static rclc_executor_t executor;
static std_msgs__msg__int32 hb_msg;
static std_msgs__msg__String echo_buf;

static bool create_ros_entities(void);
static void destroy_ros_entities(void);

static rmw_qos_profile_t qos_best_effort(void)
{
    rmw_qos_profile_t qos = rmw_qos_profile_default;
    qos.reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
    qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
    qos.depth = 5;
    return qos;
}

static void sub_echo_cb(const void * msg_in)
{
    const std_msgs__msg__String * msg = (const std_msgs__msg__String *)msg_in;
    ESP_LOGI(TAG, "RX echo: %s", msg->data ? msg->data : "<null>");
}

static bool agent_is_reachable(void)
{
    rmw_uros_set_udp_address(CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT);
    const int timeout_ms = 500;
    return (RMW_RET_OK == rmw_uros_ping_agent(timeout_ms, 1));
}

static bool create_ros_entities(void)
{
    rcl_ret_t rc;

    allocator = rcl_get_default_allocator();
    rc = rclc_support_init(&support, 0, NULL, &allocator);
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "support_init failed: %s", rcl_get_error_string().str);
        return false;
    }

    rc = rclc_node_init_default(&node, "esp32_node", "", &support);
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "node_init failed: %s", rcl_get_error_string().str);
        return false;
    }

    hb_msg.data = 0;
    rc = rclc_publisher_init(&pub_heartbeat, &node,
                              ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
                              "/esp/heartbeat",
                              &qos_best_effort());
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "pub_init failed: %s", rcl_get_error_string().str);
        return false;
    }

    echo_buf.data = NULL; echo_buf.size = 0; echo_buf.capacity = 0;
    rc = rclc_subscription_init(&sub_echo, &node,
                                ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
                                "/esp/echo",
                                &qos_best_effort());
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "sub_init failed: %s", rcl_get_error_string().str);
        return false;
    }

    rc = rclc_executor_init(&executor, &support.context, 1, &allocator);
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "executor_init failed: %s", rcl_get_error_string().str);
        return false;
    }

    rc = rclc_executor_add_subscription(&executor, &sub_echo, &echo_buf, &sub_echo_cb, ON_NEW_DATA);
    if (rc != RCL_RET_OK) {
        ESP_LOGE(TAG, "executor_add_sub failed: %s", rcl_get_error_string().str);
        return false;
    }

    return true;
}

static void destroy_ros_entities(void)
{
    rcl_ret_t rc;
    rc = rcl_publisher_fini(&pub_heartbeat, &node);
    rc = rcl_subscription_fini(&sub_echo, &node);
    rc = rcl_node_fini(&node);
    rc = rclc_executor_fini(&executor);
    rc = rclc_support_fini(&support);
    (void)rc;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();

    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t pub_period = pdMS_TO_TICKS(CONFIG_PUB_PERIOD_MS);

    while (true) {
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
        bool net_up = (bits & WIFI_CONNECTED_BIT) != 0;

        switch (s_state) {
            case STATE_NET_DOWN:
                if (net_up) {
                    s_state = STATE_AGENT_WAIT;
                    ESP_LOGI(TAG, "NET_UP → checking micro‑ROS agent %s:%d",
                             CONFIG_MICRO_ROS_AGENT_IP, CONFIG_MICRO_ROS_AGENT_PORT);
                }
                vTaskDelay(pdMS_TO_TICKS(200));
                break;

            case STATE_AGENT_WAIT:
                if (!net_up) { s_state = STATE_NET_DOWN; break; }
                if (agent_is_reachable()) {
                    ESP_LOGI(TAG, "Agent reachable. Creating ROS entities...");
                    if (create_ros_entities()) {
                        s_state = STATE_ROS_OK;
                        last_wake = xTaskGetTickCount();
                    } else {
                        ESP_LOGW(TAG, "Entity creation failed. Retrying...");
                        vTaskDelay(pdMS_TO_TICKS(1000));
                    }
                } else {
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
                break;

            case STATE_ROS_OK: {
                if (!net_up) {
                    ESP_LOGW(TAG, "NET_DOWN → tearing down ROS entities");
                    destroy_ros_entities();
                    s_state = STATE_NET_DOWN;
                    break;
                }

                (void) rclc_executor_spin_some(&executor, 0);

                if (xTaskGetTickCount() - last_wake >= pub_period) {
                    last_wake += pub_period;
                    hb_msg.data++;
                    rcl_ret_t rc = rcl_publish(&pub_heartbeat, &hb_msg, NULL);
                    if (rc != RCL_RET_OK) {
                        ESP_LOGW(TAG, "publish failed: %d → will re-check agent", (int)rc);
                        destroy_ros_entities();
                        s_state = STATE_AGENT_WAIT;
                    }
                }
                vTaskDelay(pdMS_TO_TICKS(10));
                break; }
        }
    }
}