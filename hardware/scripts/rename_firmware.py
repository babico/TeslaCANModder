"""
Post-build script: copies firmware to build/firmware/<env_name>.<ext>
Uno → .hex, ESP32 → .bin
"""
import shutil, os
Import("env")

def copy_firmware(source, target, env):
    env_name = env["PIOENV"]
    fw_dir = os.path.join(env["PROJECT_DIR"], "build", "firmware")
    os.makedirs(fw_dir, exist_ok=True)

    # Determine source path and extension
    src = str(source[0])
    if env_name.startswith("esp32"):
        ext = ".bin"
    else:
        ext = ".hex"

    dst = os.path.join(fw_dir, f"{env_name}{ext}")
    shutil.copy2(src, dst)
    print(f"  Firmware copied -> build/firmware/{env_name}{ext}")

env.AddPostAction("$BUILD_DIR/firmware.hex", copy_firmware)
env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)
