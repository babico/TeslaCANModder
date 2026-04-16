FROM node:25-bookworm-slim AS base

WORKDIR /app

RUN apt-get update && apt-get upgrade -y && rm -rf /var/lib/apt/lists/*

# Copy workspace root files and package structure
COPY package.json package-lock.json ./
COPY packages/ ./packages/
COPY docs/ ./docs/
COPY web/ ./web/

# Stub workspace entries so npm ci can resolve all workspaces
COPY mobile/package.json ./mobile/package.json
COPY tools/package.json ./tools/package.json

# Install dependencies for entire workspace
RUN npm ci

# Build protocol package as dependency
RUN npm run build -w packages/protocol

FROM base AS dev

WORKDIR /app/web

RUN cp docker-entrypoint.sh /usr/local/bin/web-dev-entrypoint && \
    chmod +x /usr/local/bin/web-dev-entrypoint
ENTRYPOINT ["web-dev-entrypoint"]
CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0", "--port", "4173", "--strictPort"]

FROM base AS build

WORKDIR /app/web

RUN npm run build

FROM nginxinc/nginx-unprivileged:stable-alpine AS production
USER root
RUN apk upgrade --no-cache && \
    apk add --no-cache curl && \
    rm -rf /var/cache/apk/* /tmp/* /usr/share/nginx/html/*
USER nginx
COPY --from=build /app/web/dist /usr/share/nginx/html
COPY web/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 8080
HEALTHCHECK --interval=15s --timeout=5s --retries=3 \
  CMD curl -f http://127.0.0.1:8080/ || exit 1
