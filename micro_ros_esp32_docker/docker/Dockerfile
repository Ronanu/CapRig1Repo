FROM ros:humble-ros-base

RUN apt-get update && apt-get install -y \
    curl git nano udev \ 
    && rm -rf /var/lib/apt/lists/*

# micro-ROS Agent (from binary package)
RUN apt-get update && apt-get install -y ros-humble-micro-ros-agent \ 
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -f dialout && usermod -a -G dialout root

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENV ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}
ENTRYPOINT ["/entrypoint.sh"]
