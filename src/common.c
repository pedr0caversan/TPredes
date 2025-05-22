// funções comuns ao server e cliente
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

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

typedef struct
{
    int version;
    char addr_str[INET6_ADDRSTRLEN + 1];
    uint16_t port;
} connection_data;

// Comunica erro fatal e tipo de erro por meio de msg e encerra o programa
void fatal_error(const char *msg)
{
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

// Responsável por converter um endereço IP (string) e uma porta (string) em uma estrutura sockaddr_storage preenchida com os dados apropriados.
int addr_parse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage)
{
    if (addrstr == NULL || portstr == NULL)
    {
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr); // string to integer to big-endian
    // uint16_t garante tamanho de 16 bits
    if (port == 0)
    {
        return -1;
    }

    port = htons(port); // host to network short; host pode ser little-endian ou big-endian, enquanto network é sempre big-endian. htons garante essa conversão

    struct in_addr inaddr4; // 32-bit IP address
    if (inet_pton(AF_INET, addrstr, &inaddr4))
    {                                                              // interpreting as IPv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage; // addr4 é um ponteiro que aponta para o mesmo lugar que storage,
        // lugar que agora é interpretado como sendo do tipo sockaddr_in (IPv4)
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6))
    { // interpreting as IPv6
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

// Responsável por inicializar uma estrutura sockaddr_storage com um endereço IP e porta fornecidos como strings. O endereço IP é definido como INADDR_ANY (IPv4) ou in6addr_any (IPv6)
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage)
{
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0)
    {
        return -1;
    }

    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4"))
    {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    }
    else if (0 == strcmp(proto, "v6"))
    {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    }
    else
    {
        return -1;
    }
}


// responsável por converter um endereço IP em uma string.
void addr_to_str(const struct sockaddr *addr, char *str, size_t strsize)
{
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET)
    {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr; // addr4 é um ponteiro que aponta para o mesmo lugar que addr, lugar este que é interpretado comm sendo do tipo sockaddr_in (IPv4)
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1))
        {
            fatal_error("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    }
    else if (addr->sa_family == AF_INET6)
    {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
        {
            fatal_error("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    }
    else
    {
        fatal_error("unknown protocol family.");
    }

    if (str)
    {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

// Explicação para os 4 tipos de struct sockaddr:
//  sockaddr: é uma estrutura genérica que pode ser usada para armazenar endereços de diferentes protocolos. Não é instanciada diretamente, apenas usada como um ponteiro. (cast)
//  sockaddr_in: é uma estrutura específica para endereços IPv4. Ela contém campos como sin_family (família do endereço), sin_port (porta) e sin_addr (endereço IP).
//  sockaddr_in6: é uma estrutura específica para endereços IPv6. Same: sin_family, sin6_port e sin6_addr
//  sockaddr_storage: é uma estrutura que pode armazenar endereços de diferentes tamanhos, como IPv4 e IPv6. Ela é útil para garantir que o espaço necessário para o endereço seja alocado corretamente, independentemente do tipo de endereço usado.

// devolve um struct contendo informações sobre a conexão: versão do IP, endereço e porta.


connection_data return_connection_data(const struct sockaddr *addr, char *str, size_t strsize)
{
    connection_data data = {0};
    char addrstr[INET6_ADDRSTRLEN + 1] = "";

    if (addr->sa_family == AF_INET)
    {
        data.version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr; // addr4 é um ponteiro que aponta para o mesmo lugar que addr, lugar este que é interpretado comm sendo do tipo sockaddr_in (IPv4)
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1))
        {
            fatal_error("ntop");
        }
        data.port = ntohs(addr4->sin_port); // network to host short
    }
    else if (addr->sa_family == AF_INET6)
    {
        data.version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1))
        {
            fatal_error("ntop");
        }
        data.port = ntohs(addr6->sin6_port); // network to host short
    }
    else
    {
        fatal_error("unknown protocol family.");
    }

    snprintf(data.addr_str, sizeof(data.addr_str), "%s", addrstr);

    return data;
}