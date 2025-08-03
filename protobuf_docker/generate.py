import os
import subprocess

proto_file = "protos/messages.proto"
out_dir = "output"

os.makedirs(out_dir, exist_ok=True)

subprocess.run([
    "protoc",
    f"--proto_path=protos",
    f"--python_out={out_dir}",
    proto_file
], check=True)

subprocess.run([
    "protoc",
    f"--proto_path=protos",
    f"--nanopb_out={out_dir}",
    proto_file
], check=True)

print("Generierung abgeschlossen.")
