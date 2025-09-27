CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -O2
LDFLAGS = -lmicrohttpd -llmdb -ljansson
SRC = $(shell find src -name "*.c")
OBJ = $(SRC:src/%.c=build/%.o)
BIN = build/myapp

IMAGE_NAME = filehub_container
CONTAINER_NAME = filehub_container
STATIC_DIR = ./resource

.PHONY: all clean run build up down restart status docker-build docker-run

# --- Native build ---
all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p build
	$(CC) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf build
	docker rm -f $(CONTAINER_NAME) 2>/dev/null || true
	docker rmi -f $(IMAGE_NAME) 2>/dev/null || true

# --- Docker build & run ---
docker-build:
	docker build -t $(IMAGE_NAME) .

docker-run: docker-build
	docker run --rm -it \
		--name $(CONTAINER_NAME) \
		-p 8080:8080 \
		-v $(STATIC_DIR):/resource \
		$(IMAGE_NAME)

down:
	docker stop $(CONTAINER_NAME) || true

restart: down docker-run

status:
	docker ps -f name=$(CONTAINER_NAME)