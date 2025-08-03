import os
import subprocess

proto_file = "protobuf_msgs/messages.proto"
out_dir = "protobuf_gnerated"

os.makedirs(out_dir, exist_ok=True)

subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--python_out={out_dir}",
    proto_file
], check=True)

subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--nanopb_out={out_dir}",
    proto_file
], check=True)

print("Generierung abgeschlossen.")
