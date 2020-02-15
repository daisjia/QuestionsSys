INC += -I./
LIB +=  -ljson -levent -lpthread -lhiredis -lmysqlclient -lprotobuf -pthread
OBJ_DIR = ./obj
OBJ =  $(OBJ_DIR)/socket.o  $(OBJ_DIR)/sys.o $(OBJ_DIR)/main.o $(OBJ_DIR)/mytime.o $(OBJ_DIR)/mysql.o $(OBJ_DIR)/redis.o \
	$(OBJ_DIR)/threadpoll.o $(OBJ_DIR)/IM.User.Msg.pb.o $(OBJ_DIR)/control.o $(OBJ_DIR)/pdu.o 
OBJ_EXT = .o
CXX_EXT = .cpp
CC_EXT = .cc

main : $(OBJ)
	@echo
	@echo "Linking..."
	g++ -o main -std=c++11 $(OBJ) $(LIB) -g
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
	-rm -rf $(OBJ_DIR) ./bin main *.pb.* core.*
	make pp
	@echo
