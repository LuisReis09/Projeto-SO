# Nome do executável
TARGET = programa

# Fonte principal
SRC = server.cpp

# Flags via pkg-config (detecta paths automaticamente)
CXXFLAGS = `pkg-config --cflags opencv4`
LDFLAGS  = `pkg-config --libs opencv4`

# Compilador
CXX = g++

# Regra padrão: compila e executa
all: $(TARGET)
	./$(TARGET)

# Compilação
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

# Limpeza
clean:
	rm -f $(TARGET)
