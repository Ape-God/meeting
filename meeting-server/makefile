CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# ¿¿¿¿¿¿¿
PROJECT_NAME = meeting
VERSION = 0.1.0

# ¿¿¿¿¿¿¿¿¿
SRC_DIR = src
INCLUDE_DIR = include

# ¿¿¿
SRC_FILES = $(wildcard $(SRC_DIR)/*.cpp)

# ¿¿¿¿¿¿¿¿¿
BIN_DIR = bin

# ¿¿¿¿¿¿¿
TARGET = $(BIN_DIR)/$(PROJECT_NAME)

# ¿¿¿¿
LIBS = -pthread -lrt

# ¿¿¿¿
$(TARGET): $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $(SRC_FILES) -o $(TARGET) $(LIBS)

# ¿¿¿¿
all: $(TARGET)

# ¿¿¿¿¿¿¿
clean:
	rm -f $(TARGET)
