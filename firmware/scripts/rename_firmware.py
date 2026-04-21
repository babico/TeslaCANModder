"""
Post-build script: copies firmware to build/firmware/<env_name>.bin
"""
import shutil, os
Import("env")

def copy_firmware(source, target, env):
    env_name = env["PIOENV"]
    fw_dir = os.path.join(env["PROJECT_DIR"], "build", "firmware")
    os.makedirs(fw_dir, exist_ok=True)

    src = str(source[0])
    dst = os.path.join(fw_dir, f"{env_name}.bin")
    shutil.copy2(src, dst)
    print(f"  Firmware copied -> build/firmware/{env_name}.bin")

env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware)
