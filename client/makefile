INC += -I./
LIB += -lpthread -lprotobuf -ljson
OBJ_DIR = ./obj
OBJ =  $(OBJ_DIR)/IM.User.Msg.pb.o $(OBJ_DIR)/main.o  $(OBJ_DIR)/pdu.o $(OBJ_DIR)/view.o $(OBJ_DIR)/client.o $(OBJ_DIR)/control.o $(OBJ_DIR)/cli_socket.o 
OBJ_EXT = .o
CXX_EXT = .cpp
CC_EXT = .cc

main : $(OBJ)
	@echo
	@echo "Linking..."
	g++ -o main -std=c++11 $(OBJ) $(LIB)
	@echo

$(OBJ_DIR)/%$(OBJ_EXT) : %$(CXX_EXT)
	@echo
	@echo "Compiling $< ==> $@..."
	@-mkdir -p $(OBJ_DIR)
	g++ -c -g -std=c++11 $< -o $@

$(OBJ_DIR)/%$(OBJ_EXT) : %$(CC_EXT)
	@echo
	@echo "Compiling $< ==> $@..."
	@-mkdir -p $(OBJ_DIR)
	g++ -c -g -std=c++11 $< -o $@

pp:
	@echo
	@echo "Protoing..."
	protoc -I=./ --cpp_out=./ ./IM.User.Msg.proto
	@echo
cc:
	@echo
	@echo "Removeing..."
	-rm -rf $(OBJ_DIR) ./bin main *.pb.*
	make pp
	@echo
