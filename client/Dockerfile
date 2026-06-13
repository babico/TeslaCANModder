FROM node:22-bookworm-slim AS base
WORKDIR /app

COPY package.json package-lock.json ./
COPY client/package.json client/package.json
COPY packages/protocol/package.json packages/protocol/package.json
COPY tools/package.json tools/package.json

RUN rm -f package-lock.json && npm install --no-audit --no-fund

FROM base AS dev
COPY . .
EXPOSE 4173
CMD ["npm", "run", "web", "-w", "@teslacanmodder/client", "--", "--host", "0.0.0.0", "--port", "4173"]

FROM base AS build
COPY . .
RUN npm run build -w packages/protocol
RUN npm run web:build -w @teslacanmodder/client

FROM nginx:1.27-alpine AS production
COPY client/nginx.conf /etc/nginx/conf.d/default.conf
COPY --from=build /app/client/dist-web /usr/share/nginx/html
EXPOSE 8080
