import os
import subprocess
import shutil

# Konfiguration
proto_dir = "protobuf_msgs"
out_dir = "protobuf_generated"
nanopb_src_dir = "/nanopb"

# Pfade zu Protobuf-Dateien
proto_python = os.path.join(proto_dir, "messages.proto")
proto_nanopb = os.path.join(proto_dir, "messages_nanopb.proto")

# Nanopb-Basiskomponenten
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

print("🔁 Starte Protobuf-Generierung...")

# 1. Python-Dateien generieren
print("🐍 Generiere Python-Protobuf-Bindings...")
subprocess.run([
    "protoc",
    f"--proto_path={proto_dir}",
    f"--python_out={out_dir}",
    proto_python
], check=True)
print("✅ Python-Generierung abgeschlossen.")

# 2. Nanopb-C-Dateien generieren
print("⚙️  Generiere Nanopb-C-Dateien...")
subprocess.run([
    "protoc",
    f"--proto_path={proto_dir}",
    f"--nanopb_out={out_dir}",
    proto_nanopb
], check=True)
print("✅ Nanopb-Generierung abgeschlossen.")

# 3. Kopiere Nanopb-Kernbibliothek
print("📦 Kopiere Nanopb-Core-Dateien...")
for filename in nanopb_files:
    src = os.path.join(nanopb_src_dir, filename)
    dst = os.path.join(out_dir, filename)
    shutil.copyfile(src, dst)
print("✅ Nanopb-Core-Dateien kopiert.")

print("🏁 Fertig.")
