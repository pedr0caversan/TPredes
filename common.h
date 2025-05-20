
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>         // size_t
#include <stdint.h>         // uint16_t
#include <sys/socket.h>  
#include <arpa/inet.h>

#define MSG_SIZE 256
#define BUFSZ 1024


typedef enum {
MSG_REQUEST,
MSG_RESPONSE,
MSG_RESULT,
MSG_PLAY_AGAIN_REQUEST,
MSG_PLAY_AGAIN_RESPONSE,
MSG_ERROR,
MSG_END
} MessageType;

typedef struct {
int type; // Tipo da mensagem
int client_action;
int server_action;
int result;
int client_wins;
int server_wins;
char message[MSG_SIZE];
} GameMessage;


// Armazena dados de conexão: versão do IP, endereço e porta
typedef struct
{
    int version;
    char addr_str[INET6_ADDRSTRLEN + 1];
    uint16_t port;
} connection_data;

// Comunica erro fatal e tipo de erro por meio de msg e encerra o programa
void fatal_error(const char *msg);

// Converte string de IP e porta para uma sockaddr_storage preenchida
int addr_parse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

// Inicializa um sockaddr_storage para uso em servidor (INADDR_ANY ou in6addr_any)
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

// devolve um struct contendo informações sobre a conexão: versão do IP, endereço e porta.
connection_data return_connection_data(const struct sockaddr *addr, char *str, size_t strsize);

// Converte sockaddr para string legível (IPv4/IPv6 + endereço + porta)
void addr_to_str(const struct sockaddr *addr, char *str, size_t strsize);

#endif // COMMON_H
