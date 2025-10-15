# Minimal micro-ROS Agent container (UDP) on ROS 2 Humble
FROM ros:humble-ros-core

# Install micro-ROS Agent from apt
RUN apt-get update && apt-get install -y --no-install-recommends     ros-humble-micro-ros-agent  && rm -rf /var/lib/apt/lists/*

# Copy entrypoint from repository's scripts folder
COPY scripts/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
