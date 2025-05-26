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
    if (0 != addr_parse(argv[1], argv[2], &storage))
    {
        usage_error(argc, argv);
    }

    int sockt = socket(storage.ss_family, SOCK_STREAM, 0);

    if (sockt == -1)
    {
        fatal_error("Erro ao criar o socket.");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != connect(sockt, addr, sizeof(storage)))
    {
        fatal_error("Erro ao conectar ao servidor.");
    }

    printf("Conectado ao servidor.\n");

    GameMessage msg_to_server = {0};
    GameMessage msg_from_server = {0};
    msg_to_server.type = MSG_RESPONSE;
    // loop de recebimento e envio de mensagens
    while (1)
    {
        // recebe msg de qualquer tipo do servidor
        // printf("primeiro recv\n");
        size_t byte_count = recv(sockt, &msg_from_server, BUFSZ - 1, 0);
        if (byte_count == -1)
        {
            fatal_error("Erro ao receber mensagem do servidor.");
        }
        else if (byte_count == 0)
        {
            printf("Servidor desconectado.\n");
            break;
        }

        printf("%s", msg_from_server.message);

        if (msg_from_server.type == MSG_PLAY_AGAIN_REQUEST || msg_from_server.type == MSG_REQUEST)
        {
            char buf[BUFSZ];
            fgets(buf, BUFSZ - 1, stdin);
            msg_to_server.client_action = atoi(buf);

            if (msg_from_server.type == MSG_REQUEST)
            {
                msg_to_server.type = MSG_RESPONSE;
            }
            else if (msg_from_server.type == MSG_PLAY_AGAIN_REQUEST)
            {
                msg_to_server.type = MSG_PLAY_AGAIN_RESPONSE;
            }
        }
        else
        {
            msg_to_server.type = MSG_RESPONSE;
        }
        // envia msg para o servidor
        byte_count = send(sockt, &msg_to_server, sizeof(msg_to_server), 0);
        if (byte_count == -1)
        {
            fatal_error("Erro ao enviar mensagem para o servidor.");
        }
        msg_to_server.client_action = -1; // reset da ação do cliente
    }
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
