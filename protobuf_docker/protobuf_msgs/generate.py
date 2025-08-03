import os
import subprocess

proto_file = "protobuf_msgs/messages.proto"
out_dir = "protobuf_generated"

os.makedirs(out_dir, exist_ok=True)

print("Starting protobuf generation...")

subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--python_out={out_dir}",
    proto_file
], check=True)

print("Python generation completed.")

subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--nanopb_out={out_dir}",
    proto_file
], check=True)

print("Nanopb generation completed.")
