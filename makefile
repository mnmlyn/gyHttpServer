SRC_FILE=gy_server.cpp Connection.cpp http_parse_util.cpp
OUTPUT=server

$(OUTPUT):$(SRC_FILE)
	g++ -g -std=c++11 -o $(OUTPUT) $(SRC_FILE)
