FROM node:22-alpine AS base

WORKDIR /app

RUN apk upgrade --no-cache && apk add --no-cache wget

# Copy workspace root files and package structure
COPY package.json package-lock.json ./
COPY packages/ ./packages/
COPY docs/ ./docs/
COPY web/ ./web/

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

FROM nginx:1.28-alpine AS production
RUN apk upgrade --no-cache && apk add --no-cache wget
COPY --from=build /app/web/dist /usr/share/nginx/html
COPY web/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
