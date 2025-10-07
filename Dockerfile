FROM ros:humble-ros-core

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions python3-pip git tmux \
    ros-humble-rmw-fastrtps-cpp \
    && rm -rf /var/lib/apt/lists/*

# micro-ROS Agent
RUN pip3 install micro-ros-agent==2.0.6

WORKDIR /root/ros2_ws
COPY ros2_ws /root/ros2_ws

RUN /bin/bash -c "source /opt/ros/humble/setup.bash && colcon build"

ENV ROS_DISTRO=humble
RUN echo 'source /opt/ros/humble/setup.bash' >> /root/.bashrc && \
    echo 'source /root/ros2_ws/install/setup.bash' >> /root/.bashrc

COPY start_ros_stack.sh /usr/local/bin/start_ros_stack.sh
RUN chmod +x /usr/local/bin/start_ros_stack.sh

CMD ["/bin/bash"]
