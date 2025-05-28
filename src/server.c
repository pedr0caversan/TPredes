#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

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

//retorna verdadeiro se a mensagem do cliente for inválida
bool test_input_error(int msg_type, char *client_message)
{
  bool error = false;

  if (msg_type == MSG_RESPONSE)
  {
    if (
        strncmp(client_message, "0", 1) != 0 &&
        strncmp(client_message, "1", 1) != 0 &&
        strncmp(client_message, "2", 1) != 0 &&
        strncmp(client_message, "3", 1) != 0 &&
        strncmp(client_message, "4", 1) != 0)
    {
      //printf("não é 0-4...\n");
      error = true;
    }
  }
  else if (msg_type == MSG_PLAY_AGAIN_RESPONSE)
  {
    if (
        strncmp(client_message, "0", 1) != 0 &&
        strncmp(client_message, "1", 1) != 0)
    {
      error = true;
    }
  }
  //error = false;
  return error;
}

// Utilizado para transmitir os dados do cliente para a thread
struct client_data
{
  int client_sockt;
  struct sockaddr_storage storage;
};

// retorna o resultado da partida
// -1 para empate, 0 para derrota do cliente, 1 para vitória do cliente
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
    fatal_error("Erro ao criar o socket.");
  }

  int enable = 1;
  if (0 != setsockopt(sockt, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) // permite reutilizar o endereço
  {
    fatal_error("Erro ao definir o socket como reutilizável.");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (0 != bind(sockt, addr, sizeof(storage)))
  {
    fatal_error("Erro ao associar o socket.");
  }

  if (0 != listen(sockt, 10))
  {
    fatal_error("Erro ao escutar o socket.");
  }

  char addrstr[BUFSZ];
  connection_data server_data;
  server_data = return_connection_data(addr, addrstr, BUFSZ);
  printf("Servidor iniciado em modo IPv%d na porta %hu. Aguardando conexão...\n", server_data.version, server_data.port);

  // loop para atendimento de um cliente de cada vez
  while (1)
  {
    struct sockaddr_storage client_storage;
    struct sockaddr *caddr = (struct sockaddr *)&client_storage;
    socklen_t client_addrlen = sizeof(client_storage);

    int client_sockt = accept(sockt, caddr, &client_addrlen);
    if (client_sockt == -1)
    {
      fatal_error("accept");
    }

    printf("Cliente conectado. \n");

    GameMessage msg_to_client = {0};
    GameMessage msg_from_client = {0};
    msg_from_client.type = MSG_RESPONSE;
    msg_to_client.type = MSG_REQUEST;
    int client_atk = 0;
    int server_atk = 0;
    char *attacks[] = {"Nuclear Attack", "Intercept Attack", "Cyber Attack", "Drone Strike", "Bio Attack"};
    char *result_str[] = {"Empate", "Derrota", "Vitória"};
    bool tie = false;
    bool tie_exeption = false;
    bool error = false;

    // loop de envios e recebimentos de mensagens
    while (1)
    {
      char buf[BUFSZ];
      memset(buf, 0, BUFSZ);

      if (tie_exeption)
      {
        msg_to_client.type = MSG_REQUEST;
        tie_exeption = false;
      }

      if (tie)
      {
        tie_exeption = true;
        tie = false;
      }

      // printf("Tipo da mensagem enviada pro cliente: %d\n", msg_to_client.type);
      switch (msg_to_client.type)
      {
      case (MSG_REQUEST):
        snprintf(msg_to_client.message, MSG_SIZE, "Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intercept Attack\n2 - Cyber Attack\n3 - Drone Strike\n4 - Bio Attack\n");
        printf("Apresentando as opções para o cliente.\n");
        if (-1 == send(client_sockt, &msg_to_client, sizeof(msg_to_client), 0))
        {
          fatal_error("Erro ao enviar mensagem para o cliente.");
        }
        break;

      case (MSG_RESULT):
        snprintf(msg_to_client.message, MSG_SIZE, "Você escolheu: %s \nServidor escolheu: %s\nResultado: %s!\n", attacks[client_atk], attacks[server_atk], result_str[msg_to_client.result + 1]);
        if (-1 == send(client_sockt, &msg_to_client, sizeof(msg_to_client), 0))
        {
          fatal_error("Erro ao enviar mensagem de resultado para o cliente.");
        }
        break;

      case (MSG_PLAY_AGAIN_REQUEST):
        snprintf(msg_to_client.message, MSG_SIZE, "Deseja jogar novamente?\n1 - Sim\n0 - Não\n");
        if (-1 == send(client_sockt, &msg_to_client, sizeof(msg_to_client), 0))
        {
          fatal_error("Erro ao enviar mensagem para o cliente.\n");
        }
        printf("Perguntando se o cliente deseja jogar novamente.\n");
        break;

      case (MSG_ERROR):
        error = true;
        if (msg_from_client.type == MSG_RESPONSE)
        {
          snprintf(msg_to_client.message, MSG_SIZE, "Por favor, selecione um valor de 0 a 4.\n");
        } else if (msg_from_client.type == MSG_PLAY_AGAIN_RESPONSE)
        {
          snprintf(msg_to_client.message, MSG_SIZE, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n");
        }
        
        if (-1 == send(client_sockt, &msg_to_client, sizeof(msg_to_client), 0))
        {
          fatal_error("Erro ao enviar mensagem de erro para o cliente.");
        }
        if (msg_from_client.type == MSG_RESPONSE)
        {
          msg_to_client.type = MSG_REQUEST;
        } else if (msg_from_client.type == MSG_PLAY_AGAIN_RESPONSE)
        {
          msg_to_client.type = MSG_PLAY_AGAIN_REQUEST;
        }
        break;
      case (MSG_END):
        snprintf(msg_to_client.message, MSG_SIZE, "Fim de jogo!\nPlacar final: Cliente %d x %d Servidor\n", msg_to_client.client_wins, msg_to_client.server_wins);
        printf("Enviando placar final.\n");
        if (-1 == send(client_sockt, &msg_to_client, sizeof(msg_to_client), 0))
        {
          fatal_error("Erro ao enviar mensagem para o cliente.");
        }
        break;
      }

      if (-1 == recv(client_sockt, &msg_from_client, sizeof(msg_from_client), 0))
      {
        fatal_error("Erro ao receber mensagem do cliente.");
      }

      if (msg_to_client.type == MSG_END)
      {
        break;
      }
      if (error)
      {
        error = false;
        continue;
      }

      // printf("TIPO CLIENTE: %d\n", msg_from_client.type);
      switch (msg_from_client.type)
      {
      case (MSG_RESPONSE):
        // msg_to_client.type = MSG_PLAY_AGAIN_REQUEST;
        char test_msg[MSG_SIZE];
        snprintf(test_msg, MSG_SIZE, "5");
        //printf("MENSAGEM: %s\n", msg_from_client.message);
        if (test_input_error(MSG_RESPONSE, msg_from_client.message))
        {
          printf("Erro: opção inválida de jogada.\n");
          msg_to_client.type = MSG_ERROR;
          continue;
        }
        srand(time(NULL));
        client_atk = msg_from_client.client_action;
        server_atk = rand() % 5;
        msg_to_client.server_action = server_atk;
        if (client_atk != INVALID)
        {
          printf("Cliente escolheu %d.", client_atk);
          printf("\nServidor escolheu aleatoriamente %d. \n", server_atk);
        }

        msg_to_client.result = return_result(client_atk, server_atk);

        if (msg_to_client.type == MSG_REQUEST)
        {
          msg_to_client.type = MSG_RESULT;
          if (msg_to_client.result == -1)
          {
            printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
            // msg_to_client.type = MSG_REQUEST;
            tie = true;
          }
          else
          {
            if (msg_to_client.result == 0)
            {
              msg_to_client.server_wins++;
            }
            else if (msg_to_client.result == 1)
            {
              msg_to_client.client_wins++;
            }
            // break;
            printf("Placar atualizado: Cliente %d x %d Servidor\n", msg_to_client.client_wins, msg_to_client.server_wins);
          }
        }
        else if (msg_to_client.type == MSG_RESULT)
        {
          msg_to_client.type = MSG_PLAY_AGAIN_REQUEST;
        }
        break;
        // msg_to_client.type = MSG_RESULT;
      case (MSG_PLAY_AGAIN_RESPONSE):
        if (test_input_error(MSG_RESPONSE, msg_from_client.message))
        {
          printf("Erro: opção inválida de jogada.\n");
          msg_to_client.type = MSG_ERROR;
          continue;
        }
        if (msg_from_client.client_action == 0)
        {
          printf("Cliente não deseja jogar novamente.\n");
          msg_to_client.type = MSG_END;
          break;
        }
        else
        {
          printf("Cliente deseja jogar novamente.\n");
          msg_to_client.type = MSG_REQUEST;
        }
        break;
      }
    }
    printf("Encerrando conexão.\n");
    close(client_sockt);
    break;
  }
  printf("Cliente desconectado.\n");
  exit(EXIT_SUCCESS);
}
