INC += -I./
LIB +=  -ljson -levent -lpthread -lhiredis -lmysqlclient
OBJ_DIR = ./obj
OBJ =  $(OBJ_DIR)/socket.o  $(OBJ_DIR)/sys.o $(OBJ_DIR)/main.o $(OBJ_DIR)/mytime.o $(OBJ_DIR)/mysql.o $(OBJ_DIR)/redis.o $(OBJ_DIR)/threadpoll.o 
OBJ_EXT = .o
CXX_EXT = .cpp

main : $(OBJ)
	@echo
	@echo "Linking..."
	g++ -o main -std=c++11 $(OBJ) $(LIB)
	@echo

$(OBJ_DIR)/%$(OBJ_EXT) : %$(CXX_EXT)
	@echo
	@echo "Compiling $< ==> $@..."
	@-mkdir -p $(OBJ_DIR)
	g++ -c -std=c++11 $< -o $@

cc:
	@echo
	@echo "Removeing..."
	-rm -rf $(OBJ_DIR) ./bin main
	@echo