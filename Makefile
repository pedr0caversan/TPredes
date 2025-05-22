CC = gcc
CFLAGS = -Wall -Iinclude
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Fontes
SRCS = $(SRC_DIR)/client.c \
       $(SRC_DIR)/server.c \
       $(SRC_DIR)/threaded_server.c \
       $(SRC_DIR)/common.c

# Objetos
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Executáveis
TARGETS = $(BIN_DIR)/client $(BIN_DIR)/server $(BIN_DIR)/threaded_server

# Regra padrão
all: $(TARGETS)

# Compilar arquivos objeto
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c include/common.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# client
$(BIN_DIR)/client: $(OBJ_DIR)/client.o $(OBJ_DIR)/common.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# server
$(BIN_DIR)/server: $(OBJ_DIR)/server.o $(OBJ_DIR)/common.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# threaded_server
$(BIN_DIR)/threaded_server: $(OBJ_DIR)/threaded_server.o $(OBJ_DIR)/common.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Criar diretórios se não existirem
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Limpeza
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
