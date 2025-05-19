#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSZ 1024

// Comunica erro na inserção do IP e porta do servidor, exibe mensagem de uso correto e encerra o programa
void usage_error(int argc, char **argv)
{
    printf("usage: %s <v4/v6> <server port>\n", argv[0]);
    printf("example: %s v4 5151\n", argv[0]);
    exit(EXIT_FAILURE);
}

// Utilizado para transmitir os dados do cliente para a thread
struct client_data {
    int client_sockt;
    struct sockaddr_storage storage;
};

void *client_thread(void *data) {
    struct client_data *c_data = (struct client_data *)data;
    struct sockaddr *client_addr = (struct sockaddr *)(&c_data->storage);

    char caddrstr[BUFSZ];
    addr_to_str(client_addr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    size_t count = recv(c_data->client_sockt, buf, BUFSZ - 1, 0);
    printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

    sprintf(buf, "remote endpoint: %.1000s\n", caddrstr); // Limite de 1000 caracteres para evitar buffer overflow
    count = send(c_data->client_sockt, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1) {
        fatal_error("send");
    }

    close(c_data->client_sockt);

    pthread_exit(EXIT_SUCCESS);
}


int main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage_error(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage))
    {
        usage_error(argc, argv);
    }

    int sockt = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sockt == -1)
    {
        fatal_error("socket");
    }

    int enable = 1;
    if (0 != setsockopt(sockt, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) // permite reutilizar o endereço
    {
        fatal_error("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(sockt, addr, sizeof(storage)))
    {
        fatal_error("bind");
    }

    if (0 != listen(sockt, 10))
    {
        fatal_error("listen");
    }

    char addrstr[BUFSZ];
    addr_to_str(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

while (1) {
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int csock = accept(sockt, caddr, &caddrlen);
    if (csock == -1) {
        fatal_error("accept");
    }

    struct client_data *cdata = malloc(sizeof(*cdata));
    if (!cdata) {
        fatal_error("malloc");
    }

    cdata->client_sockt = csock;
    memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

    pthread_t tid;
    pthread_create(&tid, NULL, client_thread, cdata);
}

exit(EXIT_SUCCESS);

}
