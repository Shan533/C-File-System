CXX := g++ 
CXXFLAGS := -g -O0 -std=c++11

SRC	:= BasicFileSys.cpp Disk.cpp FileSys.cpp  main.cpp Shell.cpp
HDR	:= BasicFileSys.h  Blocks.h  Disk.h  FileSys.h  Shell.h
OBJ	:= $(patsubst %.cpp, %.o, $(SRC))

all: filesys

filesys: $(OBJ)
	$(CXX) -o $@ $(OBJ)
	rm -f DISK

%.o:	%.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f filesys *.o DISK
