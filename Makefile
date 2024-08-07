OUT_DIR = bin

SRC = src/*.cpp

CC = g++
FLAGS = -std=c++14 -Isrc
RELEASE_FLAGS = -Os -s -DPAULA_RELEASE
MINI_FLAGS = -Os -s -DPAULA_RELEASE -DPAULA_MINI
DEBUG_FLAGS = -fsanitize=leak -fsanitize=address -DPAULA_DEBUG

all:
	@echo Define target: \'release\', \'debug\', or \'mini\'

release: $(SRC) $(CMD_SRC)
	@echo --- Paula CLI RELEASE ---
	@mkdir -p $(OUT_DIR)   # Create the output directory if it doesn't exist
	$(CC) $(FLAGS) $(RELEASE_FLAGS) $(SRC) projects/paula-cli/paula-cli.cpp -o $(OUT_DIR)/paula -Wall
	
debug: $(SRC) $(CMD_SRC)
	@echo --- Paula DEBUG ---
	@mkdir -p $(OUT_DIR)   # Create the output directory if it doesn't exist
	$(CC) $(FLAGS) $(DEBUG_FLAGS) $(SRC) projects/paula-vs/paula-vs.cpp -o $(OUT_DIR)/pauladbg 
	
mini: $(SRC) $(CMD_SRC)
	@echo --- Paula MINI ---
	@mkdir -p $(OUT_DIR)   # Create the output directory if it doesn't exist
	$(CC) $(FLAGS) $(MINI_FLAGS) $(SRC) projects/paula-cli/paula-cli.cpp -o $(OUT_DIR)/paulamini -Wall
	
example: $(SRC) $(CMD_SRC)
	@echo --- Paula EXAMPLE ---
	@mkdir -p $(OUT_DIR)   # Create the output directory if it doesn't exist
	$(CC) $(FLAGS) $(RELEASE_FLAGS) $(SRC) projects/paula-example/paula-example.cpp -o $(OUT_DIR)/paulaexample -Wall