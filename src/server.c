#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
void usage_error(int argc, char **argv)
{
  printf("usage: %s <v4/v6> <server port>\n", argv[0]);
  printf("example: %s v4 5151\n", argv[0]);
  exit(EXIT_FAILURE);
}

// Utilizado para transmitir os dados do cliente para a thread
struct client_data
{
  int client_sockt;
  struct sockaddr_storage storage;
};

int return_result(int client_atk, int server_atk)
{
  if (client_atk == server_atk)
  {
    return -1; // Empate
  }
  else if (client_atk == 0 && (server_atk == 2 || server_atk == 3))
  {
    return 1; // nuclear attack ganha de cyber attack e drone strike
  }
  else if (client_atk == 1 && (server_atk == 0 || server_atk == 4))
  {
    return 1; // intercept attack ganha de nuclear attack e bio attack
  }
  else if (client_atk == 2 && (server_atk == 1 || server_atk == 3))
  {
    return 1; // cyber attack ganha de intercept attack e drone strike
  }
  else if (client_atk == 3 && (server_atk == 1 || server_atk == 4))
  {
    return 1; // drone strike ganha de intercept attack e bio attack
  }
  else if (client_atk == 4 && (server_atk == 0 || server_atk == 2))
  {
    return 1; // bio attack ganha de nuclear attack e cyber attack
  }
  return 0; // Caso contrário, derrota do cliente
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
  // addr_to_str(addr, addrstr, BUFSZ);
  // printf(" %s \n", addrstr);
  connection_data server_data;
  server_data = return_connection_data(addr, addrstr, BUFSZ);
  printf("Servidor iniciado em modo IPv%d na porta %hu. Aguardando conexão...\n", server_data.version, server_data.port);
  while (1)
  {
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)&cstorage;
    socklen_t caddrlen = sizeof(cstorage);

    int client_sockt = accept(sockt, caddr, &caddrlen);
    if (client_sockt == -1)
    {
      fatal_error("accept");
    }

    char c_addrstr[BUFSZ];
    addr_to_str(caddr, c_addrstr, BUFSZ);
    printf("Cliente conectado. \n");

   char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    char *msg = "Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intercept Attack\n2 - Cyber Attack\n3 - Drone Strike\n4 - Bio Attack\n";
    size_t count = send(client_sockt, msg, strlen(msg) + 1, 0);
    printf("Apresentando as opções para o cliente.\n");

    count = recv(client_sockt, buf, BUFSZ - 1, 0);
    srand(time(NULL));
    int client_atk = atoi(buf); // Converte a string recebida para inteiro
    int server_atk = rand() % 5; // Escolha aleatória do servidor entre 0 e 4
    //printf("[msg] %s, %d bytes: %s\n", c_addrstr, (int)count, buf);
    printf("Cliente escolheu %d.", client_atk);
    printf("\nServidor escolheu aleatoriamente %d. \n", server_atk);

    sprintf(buf, "remote endpoint: %.1000s\n", c_addrstr); // Limite de 1000 caracteres para evitar buffer overflow
    count = send(client_sockt, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1) {
        fatal_error("send");
    }
     close(client_sockt);
  }
 
  exit(EXIT_SUCCESS);
}
