FROM microros/micro-ros-agent:humble
ENV LANG=C.UTF-8 LC_ALL=C.UTF-8 MICROROS_DISABLE_SHM=1
COPY scripts/ /opt/agent/
RUN sed -i 's/\r$//' /opt/agent/*.sh && chmod +x /opt/agent/*.sh
WORKDIR /ws
ENTRYPOINT ["/opt/agent/monitor.sh"]
