#!/bin/bash
echo "Running generate.py..."
python3 /generate.py

echo "Done. Dropping into shell..."
exec bash

