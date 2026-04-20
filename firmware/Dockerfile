FROM python:3.14-slim

ENV PIP_DISABLE_PIP_VERSION_CHECK=1 \
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    BUILD_SERVER_PORT=8100

RUN apt-get update \
    && apt-get install -y --no-install-recommends git ca-certificates build-essential \
    && rm -rf /var/lib/apt/lists/*

RUN pip install platformio

WORKDIR /app/firmware

COPY firmware/platformio.ini ./platformio.ini
COPY firmware/src ./src
COPY firmware/lib ./lib
COPY firmware/test ./test
COPY firmware/scripts ./scripts

EXPOSE 8100

CMD ["python", "scripts/build_server.py"]
