CC=gcc
DIRS=build build/client build/server

all: print_vars client server

print_vars:
	@echo "Using C compiler: " $(CC)

 
client: src/client.c build  
	$(CC) -o build/client/$@ $<

server:	src/server.c build
	$(CC) -o build/server/$@ $< -lpthread

clean:
    $(shell rm -rf $(DIRS))

build:
	$(shell mkdir -p $(DIRS))


