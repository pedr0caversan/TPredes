# Diretórios
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

# Compilador e flags
CC = gcc
CFLAGS = -Wall -I$(INC_DIR)

# Arquivos fonte
CLIENT_SRC = $(SRC_DIR)/client.c $(SRC_DIR)/common.c
SERVER_SRC = $(SRC_DIR)/server.c $(SRC_DIR)/common.c
THREADED_SERVER_SRC = $(SRC_DIR)/threaded_server.c $(SRC_DIR)/common.c

# Executáveis
CLIENT_BIN = $(BIN_DIR)/client
SERVER_BIN = $(BIN_DIR)/server
THREADED_SERVER_BIN = $(BIN_DIR)/threaded_server

# Regra padrão
all: $(BIN_DIR) $(CLIENT_BIN) $(SERVER_BIN) $(THREADED_SERVER_BIN)

# Criação do diretório bin se não existir
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compilação
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) $^ -o $@

$(THREADED_SERVER_BIN): $(THREADED_SERVER_SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Limpeza
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean


