INC += -I./
LIB += -lhiredis
OBJ_DIR = ./obj
OBJ =   $(OBJ_DIR)/main.o $(OBJ_DIR)/redis.o
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


cc:
	@echo
	@echo "Removeing..."
	-rm -rf $(OBJ_DIR) ./main
	@echo