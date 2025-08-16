CXX := g++ 
CXXFLAGS := -g -O0 -std=c++11

# Source directories
SRC_DIR := src
FILESYSTEM_DIR := $(SRC_DIR)/filesystem
SHELL_DIR := $(SRC_DIR)/shell
BASIC_DIR := $(SRC_DIR)/basic
DISK_DIR := $(SRC_DIR)/disk

# Source files
SRC := $(SRC_DIR)/main.cpp \
       $(FILESYSTEM_DIR)/FileSys.cpp \
       $(SHELL_DIR)/Shell.cpp \
       $(BASIC_DIR)/BasicFileSys.cpp \
       $(DISK_DIR)/Disk.cpp

# Header files
HDR := $(FILESYSTEM_DIR)/FileSys.h \
       $(FILESYSTEM_DIR)/Blocks.h \
       $(SHELL_DIR)/Shell.h \
       $(BASIC_DIR)/BasicFileSys.h \
       $(DISK_DIR)/Disk.h

# Object files
OBJ := $(patsubst %.cpp, %.o, $(SRC))
OBJ_DIR := build/obj
BIN_DIR := build/bin
DISK_DIR_BUILD := build/disk

# Create object files with proper paths
OBJ_FILES := $(addprefix $(OBJ_DIR)/, $(notdir $(OBJ)))

# Include directories
INCLUDES := -I$(SRC_DIR) -I$(FILESYSTEM_DIR) -I$(SHELL_DIR) -I$(BASIC_DIR) -I$(DISK_DIR)

all: $(BIN_DIR)/filesys

$(BIN_DIR)/filesys: $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) -o $@ $(OBJ_FILES)
	rm -f $(DISK_DIR_BUILD)/DISK

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(FILESYSTEM_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SHELL_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(BASIC_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: $(DISK_DIR)/%.cpp $(HDR) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# Create necessary directories
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(DISK_DIR_BUILD):
	mkdir -p $(DISK_DIR_BUILD)

clean:
	rm -rf build/obj/*.o
	rm -f build/bin/filesys
	rm -f build/disk/DISK

.PHONY: all clean
