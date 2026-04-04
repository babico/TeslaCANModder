FROM python:3.11-slim

ENV PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PLATFORMIO_BUILD_ENVS=uno,uno_usb

RUN apt-get update \
    && apt-get install -y --no-install-recommends git ca-certificates build-essential \
    && rm -rf /var/lib/apt/lists/*

RUN pip install platformio

WORKDIR /app/hardware

COPY hardware/platformio.ini ./platformio.ini
COPY hardware/src ./src
COPY hardware/lib ./lib
COPY hardware/test ./test

CMD ["sh", "-lc", "set -e; for env in $(printf '%s' \"$PLATFORMIO_BUILD_ENVS\" | tr ',' ' '); do echo \"Building PlatformIO env: $env\"; platformio run -e \"$env\"; done"]
