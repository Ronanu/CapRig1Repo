FROM microros/micro-ros-agent:humble
ENV LANG=C.UTF-8 LC_ALL=C.UTF-8 MICROROS_DISABLE_SHM=1
# Copy entrypoint from repository's scripts folder
COPY scripts/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

