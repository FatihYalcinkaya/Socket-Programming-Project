// server.c

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_SIZE 1024
#define PORT 12345

typedef struct {
  int socket;
  char name[50];
} Client;

Client activeClients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *ServiceClient(void *arg) {
  int clientSocket = *((int *)arg);
  char buffer[MAX_MESSAGE_SIZE];

  // Receive the client's name
  if (recv(clientSocket, buffer, sizeof(buffer), 0) <= 0) {
    close(clientSocket);
    pthread_exit(NULL);
  }

  // Add the client to the list of active clients
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (activeClients[i].socket == -1) {
      activeClients[i].socket = clientSocket;
      strncpy(activeClients[i].name, buffer, sizeof(activeClients[i].name));
      break;
    }
  }
  pthread_mutex_unlock(&mutex);

  printf("Client %s connected\n", buffer);

  // Implement message processing and forwarding here
  while (1) {
    // Receive private messages from the client
    if (recv(clientSocket, buffer, sizeof(buffer), 0) <= 0) {
      break;
    }

    // Extract recipient name and message
    char recipient[50];
    char message[MAX_MESSAGE_SIZE];
    sscanf(buffer, "%s %[^\n]", recipient, message);

    // Find the recipient in the activeClients list
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      if (activeClients[i].socket != -1 &&
          strcmp(activeClients[i].name, recipient) == 0) {
        // Send the private message to the recipient
        send(activeClients[i].socket, message, strlen(message), 0);
        break;
      }
    }
    pthread_mutex_unlock(&mutex);
  }

  // Close the client socket and remove from the active clients list
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (activeClients[i].socket == clientSocket) {
      close(clientSocket);
      activeClients[i].socket = -1;
      printf("Client %s disconnected\n", activeClients[i].name);
      break;
    }
  }
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}

int main() {
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t addrSize = sizeof(struct sockaddr_in);

  // Initialize activeClients array
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    activeClients[i].socket = -1;
  }

  // Create socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  // Set up server address
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  // Bind socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("Error binding socket");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %d\n", PORT);

  // Listen for incoming connections
  if (listen(serverSocket, MAX_CLIENTS) == -1) {
    perror("Error listening for connections");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Accept a new connection
    clientSocket =
        accept(serverSocket, (struct sockaddr *)&clientAddr, &addrSize);
    if (clientSocket == -1) {
      perror("Error accepting connection");
      continue;
    }

    // Create a new thread for the client
    pthread_t thread;
    if (pthread_create(&thread, NULL, ServiceClient, &clientSocket) != 0) {
      perror("Error creating thread");
      close(clientSocket);
    }

    // Detach the thread to avoid memory leaks
    pthread_detach(thread);
  }

  close(serverSocket);
  return 0;
}
