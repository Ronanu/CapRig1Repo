Set sh = CreateObject("WScript.Shell")
cmd = "cmd /k docker exec -it microros-agent bash -lc ""set +u; " & _
      "if [ -f /opt/ros/humble/setup.bash ]; then source /opt/ros/humble/setup.bash; echo '[shell] Sourced /opt/ros/humble/setup.bash'; else echo '[shell] WARNING: ROS setup not found'; fi; " & _
      "if [ -f /uros_ws/install/setup.bash ]; then source /uros_ws/install/setup.bash; echo '[shell] Sourced /uros_ws/install/setup.bash'; " & _
      "elif [ -f /opt/uros_ws/install/setup.bash ]; then source /opt/uros_ws/install/setup.bash; echo '[shell] Sourced /opt/uros_ws/install/setup.bash'; " & _
      "else echo '[shell] INFO: No micro-ROS workspace found'; fi; " & _
      "set -u; exec bash"""
' 1 = normales Fenster, False = nicht warten (Fenster bleibt wegen /k offen)
sh.Run cmd, 1, False
