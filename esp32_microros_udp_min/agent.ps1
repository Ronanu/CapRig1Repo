param(
  [int]$Port = 8888,
  [string]$Tag = 'jazzy'
)
docker run -it --rm `
  -p $Port:$Port/udp `
  --name microros-agent `
  microros/micro-ros-agent:$Tag `
  udp4 --port $Port -v6