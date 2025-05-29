# Compiler
CXX = g++

# Get Python include and lib paths
PYTHON_INCLUDE = $(shell python3-config --includes)
NUMPY_INCLUDE = $(shell python3 -c "import numpy; print(numpy.get_include())")

# Compiler flags
CXXFLAGS = -I/usr/include/modbus -std=c++11 $(PYTHON_INCLUDE) -I$(NUMPY_INCLUDE) -I.
CXXFLAGS += -pthread
LDLIBS += -pthread
# Source files
SRCS = modbus_control.cpp main.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = modbus_control

# Get Python library path and name
PYTHON_VERSION = $(shell python3 -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
PYTHON_LIBDIR = $(shell python3-config --prefix)/lib
PYTHON_LIBS = -L$(PYTHON_LIBDIR) -lpython$(PYTHON_VERSION)

# Link libraries
LDLIBS = -lmodbus $(PYTHON_LIBS) -lutil

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDLIBS)

# Clean target
clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
modbus_control.o: modbus_control.cpp
main.o: main.cpp

# Pattern rule for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
