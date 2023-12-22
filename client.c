// server.c

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_NAME_LEN 20
#define MAX_MSG_LEN 200

typedef struct {
  int socket;
  char name[MAX_NAME_LEN];
} Client;

Client activeClients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *serviceClient(void *arg);
void broadcast(char *message, int senderSocket);
void sendError(int receiverSocket);

int serverSocket;

int main() {
  // Initialization code for the server socket
  struct sockaddr_in serverAddr;
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (serverSocket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(12345);

  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("Bind failed");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  if (listen(serverSocket, 10) == -1) {
    perror("Listen failed");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Loop to accept incoming connections
  while (1) {
    // Accept a new client connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

    if (clientSocket == -1) {
      perror("Accept failed");
      continue;
    }

    // Create a new thread to handle the client
    pthread_t tid;
    pthread_create(&tid, NULL, serviceClient, (void *)&clientSocket);
    pthread_detach(tid);
  }

  close(serverSocket);
  return 0;
}

void *serviceClient(void *arg) {
  int clientSocket = *((int *)arg);
  // Implementation of the ServiceClient thread

  // Receive and process messages from the client

  // Close the client connection when needed
  close(clientSocket);
  return NULL;
}

void broadcast(char *message, int senderSocket) {
  // Send a message to all clients except the sender
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (activeClients[i].socket != 0 &&
        activeClients[i].socket != senderSocket) {
      send(activeClients[i].socket, message, strlen(message), 0);
    }
  }
  pthread_mutex_unlock(&mutex);
}

void sendError(int receiverSocket) {
  // Send an error message to the specified client
  char errorMessage[] = "MERR|";
  send(receiverSocket, errorMessage, strlen(errorMessage), 0);
}
