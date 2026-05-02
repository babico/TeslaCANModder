"""
Post-build script: create a flash-ready merged ESP32 image at
build/firmware/<env_name>.bin.
"""

import os
import subprocess
from typing import Any

try:
    from SCons.Script import Import  # type: ignore[import-not-found]
except ModuleNotFoundError:
    def Import(*_args: str) -> None:
        return None


env: Any = globals().get("env")
Import("env")
env = globals().get("env", env)


FLASH_SEGMENTS = (
    ("0x1000", "bootloader.bin"),
    ("0x8000", "partitions.bin"),
    ("0xe000", "boot_app0.bin"),
    ("0x10000", "firmware.bin"),
)


def merge_firmware(source, target, env):
    env_name = env["PIOENV"]
    project_dir = env["PROJECT_DIR"]
    build_dir = env.subst("$BUILD_DIR")
    fw_dir = os.path.join(project_dir, "build", "firmware")
    os.makedirs(fw_dir, exist_ok=True)

    platform = env.PioPlatform()
    framework_dir = platform.get_package_dir("framework-arduinoespressif32")
    esptool_dir = platform.get_package_dir("tool-esptoolpy")
    if not framework_dir or not esptool_dir:
        raise RuntimeError(
            "ESP32 framework and esptool packages are required to merge firmware images"
        )

    segment_args = []
    for address, filename in FLASH_SEGMENTS:
        if filename == "boot_app0.bin":
            path = os.path.join(framework_dir, "tools", "partitions", filename)
        else:
            path = os.path.join(build_dir, filename)
        if not os.path.isfile(path):
            raise FileNotFoundError(f"Required flash segment not found: {path}")
        segment_args.extend([address, path])

    dst = os.path.join(fw_dir, f"{env_name}.bin")
    esptool_py = os.path.join(esptool_dir, "esptool.py")
    command = [
        env.subst("$PYTHONEXE"),
        esptool_py,
        "--chip",
        "esp32",
        "merge_bin",
        "-o",
        dst,
        "--flash_mode",
        "dio",
        "--flash_freq",
        "40m",
        "--flash_size",
        "4MB",
        *segment_args,
    ]
    subprocess.run(command, check=True)
    print(f"  Firmware merged -> build/firmware/{env_name}.bin")


if env is not None:
    env.AddPostAction("$BUILD_DIR/firmware.bin", merge_firmware)
