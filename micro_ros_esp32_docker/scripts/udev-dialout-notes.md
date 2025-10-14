# Notes on /dev/ttyUSB* / /dev/ttyACM*

- On Linux you usually need your user in the `dialout` group for serial access.
- On Windows + Docker Desktop (WSL2 backend), attach the USB device to the `docker-desktop` distro using `usbipd`.
- Docker gets access via `devices:` in compose and `group_add: dialout`.
