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

typedef struct {
  char sender[50];
  char recipient[50];
  char message[MAX_MESSAGE_SIZE];
} PrivateMessage;

Client activeClients[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void BroadcastMessage(const char *message, int senderSocket) {
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (activeClients[i].socket != -1 &&
        activeClients[i].socket != senderSocket) {
      send(activeClients[i].socket, message, strlen(message), 0);
    }
  }
  pthread_mutex_unlock(&mutex);
}

void *ServiceClient(void *arg) {
  int clientSocket = *((int *)arg);
  PrivateMessage privateMessage;

  // Receive the client's name
  if (recv(clientSocket, privateMessage.sender, sizeof(privateMessage.sender),
           0) <= 0) {
    perror("Error receiving client name");
    close(clientSocket);
    pthread_exit(NULL);
  }

  // Add the client to the list of active clients
  pthread_mutex_lock(&mutex);
  for (int i = 0; i < MAX_CLIENTS; ++i) {
    if (activeClients[i].socket == -1) {
      activeClients[i].socket = clientSocket;
      strncpy(activeClients[i].name, privateMessage.sender,
              sizeof(activeClients[i].name));
      printf("Client %s connected\n", privateMessage.sender);
      break;
    }
  }
  pthread_mutex_unlock(&mutex);

  // Implement message processing and forwarding here
  while (1) {
    // Receive private messages from the client
    if (recv(clientSocket, &privateMessage, sizeof(PrivateMessage), 0) <= 0) {
      perror("Error receiving private message");
      break;
    }

    // Find the recipient in the activeClients list
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
      if (activeClients[i].socket != -1 &&
          strcmp(activeClients[i].name, privateMessage.recipient) == 0) {
        // Send the private message to the recipient
        char displayMessage[MAX_MESSAGE_SIZE + 100]; // Increased size
        snprintf(displayMessage, sizeof(displayMessage), "[%s -> %s]: %s",
                 privateMessage.sender, privateMessage.recipient,
                 privateMessage.message);

        if (send(activeClients[i].socket, displayMessage,
                 strlen(displayMessage), 0) == -1) {
          perror("Error sending private message");
          break;
        }

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
      printf("Client %s disconnected\n", activeClients[i].name);
      activeClients[i].socket = -1;
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

  // Listen for incoming connections
  if (listen(serverSocket, MAX_CLIENTS) == -1) {
    perror("Error listening for connections");
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %d\n", PORT);

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
