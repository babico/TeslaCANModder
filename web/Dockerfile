FROM node:20-alpine AS base

WORKDIR /app/web

RUN apk add --no-cache wget

COPY web/package.json web/package-lock.json ./
RUN npm ci

FROM base AS dev
COPY web/docker-entrypoint.sh /usr/local/bin/web-dev-entrypoint
RUN chmod +x /usr/local/bin/web-dev-entrypoint
ENTRYPOINT ["web-dev-entrypoint"]
CMD ["npm", "run", "dev", "--", "--host", "0.0.0.0", "--port", "4173", "--strictPort"]

FROM base AS build
COPY web ./
RUN npm run build

FROM nginx:1.27-alpine AS production
RUN apk add --no-cache wget
COPY --from=build /app/web/dist /usr/share/nginx/html
COPY web/nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
