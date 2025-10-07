import json
import rclpy
from rclpy.node import Node
from std_msgs.msg import String, UInt64

class EspBridgeNode(Node):
    def __init__(self):
        super().__init__('esp_bridge_node')
        self.create_subscription(UInt64, '/esp/heartbeat_us', self.on_heartbeat, 10)
        self.echo_pub = self.create_publisher(String, '/esp/echo_request', 10)
        self.create_subscription(String, '/esp/echo_response', self.on_echo_response, 10)
        self.timer = self.create_timer(3.0, self.send_echo_request)
        self.req_count = 0
        self.get_logger().info('CapRig esp_bridge ready: listening on /esp/heartbeat_us and /esp/echo_response.')

    def on_heartbeat(self, msg: UInt64):
        self.get_logger().info(f'HB esp_us={msg.data}')

    def send_echo_request(self):
        self.req_count += 1
        payload = f'Ping #{self.req_count}'
        self.echo_pub.publish(String(data=payload))
        self.get_logger().info(f'Sent /esp/echo_request: "{payload}"')

    def on_echo_response(self, msg: String):
        # Expect simple JSON: {"esp_us":..., "echo":"..."}
        try:
            data = json.loads(msg.data)
            esp_us = data.get('esp_us')
            echo = data.get('echo')
            self.get_logger().info(f'Echo from ESP: esp_us={esp_us}, echo="{echo}"')
        except Exception:
            self.get_logger().info(f'Echo (raw): {msg.data}')

def main():
    rclpy.init()
    node = EspBridgeNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
