#!/usr/bin/env bash
set -e
echo "Serial devices:"
ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true
echo
echo "User groups:"
id
