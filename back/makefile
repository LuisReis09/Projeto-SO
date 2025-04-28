# Prerequisites:
INSTALL = sudo apt install -y

# OpenCV
OPENCV = libopencv-dev

# Nome do executável
TARGET = programa

# Fonte principal
SRC = server.cpp

# Flags via pkg-config (detecta paths automaticamente)
CXXFLAGS = `pkg-config --cflags opencv4`
LDFLAGS  = `pkg-config --libs opencv4`

# Compilador
CXX = g++

# Standard C++
CXXSTD = -std=c++17

# Regra padrão: compila e executa
all: prereq build run

# Instalação do OpenCV
prereq:
	$(INSTALL) $(OPENCV)

# Compilação
build:
	$(CXX) $(CXXSTD) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

run:
	./$(TARGET)

# Limpeza
clean:
	rm -f $(TARGET)
