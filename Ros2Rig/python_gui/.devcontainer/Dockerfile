FROM ros:humble-ros-base

RUN apt-get update && apt-get install -y \
    python3-pip python3-tk python3-dev \
    micro-ros-agent \
    ros-humble-rclpy ros-humble-std-msgs \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace/ros2_ws
