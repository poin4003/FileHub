IMAGE_NAME = c_server_container
CONTAINER_NAME = c_server_container

STATIC_DIR = ./resource

build:
	docker build -t $(IMAGE_NAME) .

up: build
	docker run --rm -it \
		--name $(CONTAINER_NAME) \
		-p 8080:8080 \
		-v $(STATIC_DIR):/resource \
		$(IMAGE_NAME)

down:
	docker stop $(CONTAINER_NAME) || true

clean:
	docker rm $(CONTAINER_NAME) || true
	docker rmi $(IMAGE_NAME) || true

restart: down up

status:
	docker ps -f name=$(CONTAINER_NAME)

.PHONY: build up down clean restart status