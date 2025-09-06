FROM alpine:latest as builder

RUN apk add --no-cache \
    build-base \
    libmicrohttpd-dev \
    lmdb-dev \
    cjson-dev \
    util-linux-dev

WORKDIR /app
COPY . .

RUN gcc -o myapp $(find src -name "*.c") -Iinclude -lmicrohttpd -llmdb -lcjson

FROM alpine:latest

RUN apk add --no-cache \
    libmicrohttpd \
    lmdb \
    cjson

WORKDIR /app
COPY --from=builder /app/myapp /app/myapp

EXPOSE 8080
CMD ["./myapp"]
