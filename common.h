
#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>         // size_t
#include <stdint.h>         // uint16_t
#include <sys/socket.h>     // struct sockaddr, sockaddr_storage

// Comunica erro fatal e tipo de erro por meio de msg e encerra o programa
void fatal_error(const char *msg);

// Converte string de IP e porta para uma sockaddr_storage preenchida
int addr_parse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage);

// Inicializa um sockaddr_storage para uso em servidor (INADDR_ANY ou in6addr_any)
int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage);

// Converte sockaddr para string legível (IPv4/IPv6 + endereço + porta)
void addr_to_str(const struct sockaddr *addr, char *str, size_t strsize);

#endif // COMMON_H
