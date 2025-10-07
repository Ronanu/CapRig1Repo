FROM ros:humble-ros-core

# Tools for building your ROS workspace
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions git tmux \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /root/ros2_ws
COPY ros2_ws /root/ros2_ws

# Build your workspace
RUN /bin/bash -lc "source /opt/ros/humble/setup.bash && colcon build"

# Convenience
ENV ROS_DISTRO=humble
RUN echo 'source /opt/ros/humble/setup.bash' >> /root/.bashrc && \
    echo 'source /root/ros2_ws/install/setup.bash' >> /root/.bashrc

# Start your ROS side
COPY start_ros_stack.sh /usr/local/bin/start_ros_stack.sh
RUN chmod +x /usr/local/bin/start_ros_stack.sh

CMD ["/bin/bash"]
