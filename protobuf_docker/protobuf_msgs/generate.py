import os
import subprocess
import shutil

proto_file = "protobuf_msgs/messages.proto"
out_dir = "protobuf_generated"
nanopb_src_dir = "/nanopb"

nanopb_files = [
    "pb_common.c",
    "pb_common.h",
    "pb_decode.c",
    "pb_decode.h",
    "pb_encode.c",
    "pb_encode.h",
    "pb.h"
]

os.makedirs(out_dir, exist_ok=True)

print("Starting protobuf generation...")

# Generate Python bindings
subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--python_out={out_dir}",
    proto_file
], check=True)

print("Python generation completed.")

# Generate Nanopb C files
subprocess.run([
    "protoc",
    f"--proto_path=protobuf_msgs",
    f"--nanopb_out={out_dir}",
    proto_file
], check=True)

print("Nanopb generation completed.")

# Copy nanopb core files
print("Copying nanopb core files...")
for filename in nanopb_files:
    src = os.path.join(nanopb_src_dir, filename)
    dst = os.path.join(out_dir, filename)
    shutil.copyfile(src, dst)

print("Nanopb core files copied.")
