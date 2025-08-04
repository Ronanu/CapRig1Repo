#!/bin/bash
set -e

python3 protobuf_msgs/generate.py 2>&1 || echo "generate.py nicht gefunden oder fehlgeschlagen"


echo "✔️ Done. Dropping into shell..."
exec bash