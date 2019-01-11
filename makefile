SRC_FILE=gy_server.c
OUTPUT=server

$(OUTPUT):$(SRC_FILE)
	gcc -g -o $(OUTPUT) $(SRC_FILE)
