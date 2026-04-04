#!/bin/sh
set -eu

cd /app/web

if [ ! -x node_modules/.bin/vite ]; then
  echo "Installing web dependencies into bind-mounted workspace..."
  npm ci
fi

exec "$@"
