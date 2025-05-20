#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>


// Comunica erro na inserção do IP e porta do servidor, exibe mensagem de uso correto e encerra o programa
void usage_error(char argc, char *argv[])
{
    printf("uso: %s <server IP> <server port>\n", argv[0]);
    printf("exemplo: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    
    // AF_INET determina IPv4 (AF_INET6 para IPv6), SOCK_STREAM indica que o socket é do tipo TCP e 0 indica que o protocolo padrão deve ser usado (TCP para SOCK_STREAM e UDP para SOCK_DGRAM)
    if (argc < 3) // port e IP
    {
        usage_error(argc, argv);
    }

    struct sockaddr_storage storage; // storage para IPv4 e IPv6
    if (0 != addr_parse(argv[1], argv[2], &storage)) {
        usage_error(argc, argv);
    }

    int sockt = socket(storage.ss_family, SOCK_STREAM, 0); 

    if (sockt == -1)
    {
        fatal_error("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)( &storage );
    if (0 != connect(sockt, addr, sizeof(storage))) {
        fatal_error("connect");
    }


    // Começo da conexão a seguir
    printf("Conectado ao servidor.\n");

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intercept Attack\n2 - Cyber Attack\n3 - Drone Strike\n4 - Bio Attack\n");
    fgets(buf, BUFSZ - 1, stdin);
    size_t count = send(sockt, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1)
    {
        fatal_error("send");
    }

    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    while (1)
    {
        count = recv(sockt, buf + total, BUFSZ - total, 0);
        if (count == 0)
        {
            // Connection terminated.
            break;
        }
        total += count;
    }
    close(sockt);

    printf("received %u bytes\n", total);
    puts(buf);

    exit(EXIT_SUCCESS);
}

// typedef enum {
// MSG_REQUEST,
// MSG_RESPONSE,
// MSG_RESULT,
// MSG_PLAY_AGAIN_REQUEST,
// MSG_PLAY_AGAIN_RESPONSE,
// MSG_ERROR,
// MSG_END
// } MessageType;
