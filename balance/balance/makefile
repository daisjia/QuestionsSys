INC += -I./
LIB +=  -ljson -levent -lpthread
OBJ_DIR = ./obj
OBJ = $(OBJ_DIR)/main.o $(OBJ_DIR)/balance.o 
OBJ_EXT = .o
CXX_EXT = .cpp


main : $(OBJ)
	@echo
	@echo "Linking..."
	g++ -o main -std=c++11  $(OBJ) $(LIB)
	@echo

$(OBJ_DIR)/%$(OBJ_EXT) : %$(CXX_EXT)
	@echo
	@echo "Compiling $< ==> $@..."
	@-mkdir -p $(OBJ_DIR)
	g++ -std=c++11 -c $< -o $@ -g

cc:
	@echo
	@echo "Removeing..."
	-rm -rf $(OBJ_DIR) ./bin main core*
	@echo
