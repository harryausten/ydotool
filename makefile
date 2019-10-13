# Compiler flags
WARN := -Wall -Wextra -Wpedantic
OPT += -pthread
CFLAGS := $(WARN) $(OPT)
CXXFLAGS := $(CFLAGS) -Weffc++

# Libraries
LIB += -levdevPlus

# Executables and dependencies
EXE := ydotool ydotoold
ydotool_DEP := ydotool.o click.o key.o mouse.o recorder.o type.o uinput.o
ydotoold_DEP := ydotoold.o uinput.o

# Default to building the executables
.PHONY: default
default: $(EXE)

# Compile a C source file
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile a C++ source file
%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main application
ydotool: $(ydotool_DEP)
	$(CXX) $(CXXFLAGS) $^ $(LIB) -o $@

# Daemon application
ydotoold: $(ydotoold_DEP)
	$(CC) $(CFLAGS) $^ -o $@

# Remove build files
.PHONY: clean
clean:
	$(RM) $(EXE) *.o
