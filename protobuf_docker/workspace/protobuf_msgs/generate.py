import os
import subprocess
import shutil

# Konfiguration

# Sourceverzeichnise für Protobuf-Dateien
proto_dir = "protobuf_msgs"
nanopb_src_dir = "/nanopb"

# Zielverzeichnisse für generierte Dateien
out_dir_py = 'pythonProject'
out_dir_esp = "esp32project/lib/proto"


# Pfade zu Protobuf-Dateien
proto_python = os.path.join(proto_dir, "messages.proto")
proto_esp = os.path.join(proto_dir, "messages_esp.proto")
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

os.makedirs(out_dir_esp, exist_ok=True)
os.makedirs(out_dir_py, exist_ok=True)

print("🔁 Starte Protobuf-Generierung...")

# 1. Python-Dateien generieren
print("Generiere Python-Protobuf-Bindings...")
subprocess.run([
    "protoc",
    f"--proto_path={proto_dir}",
    f"--python_out={out_dir_py}",
    proto_python
], check=True)
print("Python-Generierung abgeschlossen.")

# 2. Nanopb-C-Dateien generieren
print("Generiere Nanopb-C-Dateien...")
subprocess.run([
    "protoc",
    f"--proto_path={proto_dir}",
    f"--nanopb_out={out_dir_esp}",
    proto_nanopb
], check=True)
print("Nanopb-Generierung abgeschlossen.")

# 3. Kopiere Nanopb-Kernbibliothek
print("Kopiere Nanopb-Core-Dateien...")
for filename in nanopb_files:
    src = os.path.join(nanopb_src_dir, filename)
    dst = os.path.join(out_dir_esp, filename)
    shutil.copyfile(src, dst)
print("Nanopb-Core-Dateien kopiert.")

print("Fertig.")
