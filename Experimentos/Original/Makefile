# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -Isrc
LDFLAGS =

# Nome do executável
TARGET = manut

# Diretórios
SRC_DIR = src
OBJ_DIR = obj

# Encontra todos os arquivos .cpp no diretório src usando wildcard
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Gera os nomes dos arquivos de objeto no diretório obj
# Ex: Transforma "src/main.cpp" em "obj/main.o"
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Alvo padrão: compila o executável
all: $(TARGET)

# Regra para vincular os arquivos de objeto e criar o executável
$(TARGET): $(OBJS)
	@echo "Vinculando para criar o executável..."
	$(CXX) $(LDFLAGS) -o $(TARGET) $(OBJS)
	@echo "Executável '$(TARGET)' criado com sucesso."

# Regra para criar o diretório de objetos, se não existir
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

# A compilação dos objetos depende que o diretório obj exista
$(OBJS): | $(OBJ_DIR)

# Regra para compilar arquivos .cpp em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados (APENAS o executável e a pasta obj)
clean:
	@echo "Limpando arquivos gerados..."
	@rm -f $(TARGET)
	@rm -rf $(OBJ_DIR)
	@echo "Limpeza concluída."

# Regra para compilar com símbolos de depuração para profiling
profile: CXXFLAGS = -std=c++11 -Wall -O2 -g -fno-omit-frame-pointer -Isrc
profile: clean $(TARGET)

# Alvos que não representam arquivos
.PHONY: all clean profile
