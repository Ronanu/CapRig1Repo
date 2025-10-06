@echo off
REM Start micro-ROS agent on COM4 on the Windows HOST (outside the container).
REM Requires either microros agent installed locally, or use docker image.
where micro-ros-agent >nul 2>nul
if %ERRORLEVEL%==0 (
  echo Starting local micro-ROS agent on COM4 ...
  micro-ros-agent serial --dev COM4 -v6
) else (
  echo micro-ros-agent not found locally - using docker image
  docker run --rm -it --network host --name microros-agent --volume %CD%:/w microros/micro-ros-agent:humble serial --dev COM4 -v6
)
