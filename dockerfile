FROM alpine:latest as builder

RUN apk add --no-cache \
    build-base \
    libmicrohttpd-dev \
    lmdb-dev \
    jansson-dev \
    util-linux-dev

WORKDIR /app
COPY . .

RUN gcc -o myapp $(find src -name "*.c") -Iinclude -lmicrohttpd -llmdb -ljansson

FROM alpine:latest

RUN apk add --no-cache \
    libmicrohttpd \
    lmdb \
    jansson

WORKDIR /app
COPY --from=builder /app/myapp /app/myapp

EXPOSE 8080
CMD ["./myapp"]