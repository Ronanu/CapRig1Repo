#!/bin/bash
set -e

python3 /workspace/protobuf_msgs/generate.py || echo "generate.py nicht gefunden oder fehlgeschlagen"

echo "✔️ Done. Dropping into shell..."
exec bash