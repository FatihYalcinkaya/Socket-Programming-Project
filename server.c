// client.c

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_NAME_LEN 20
#define MAX_MSG_LEN 200

void connectToServer();
void *receiveChat(void *arg);
void sendConnCommand(int serverSocket, char *name);
void sendMessage(int serverSocket, char *receiver, char *message);

int serverSocket;

int main() {
  // Initialization code for the client socket
  struct sockaddr_in serverAddr;

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(12345);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(serverSocket, (struct sockaddr *)&serverAddr,
              sizeof(serverAddr)) == -1) {
    perror("Connection failed");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Connect to the server
  connectToServer();

  // Code for user input and sending messages
  char name[MAX_NAME_LEN];
  printf("Enter your chat name: ");
  fgets(name, MAX_NAME_LEN, stdin);
  name[strcspn(name, "\n")] = '\0'; // Remove the newline character

  sendConnCommand(serverSocket, name);

  // Create a thread to receive messages
  pthread_t receiveThread;
  pthread_create(&receiveThread, NULL, receiveChat, (void *)&serverSocket);
  pthread_detach(receiveThread);

  // Main loop for sending messages
  char receiver[MAX_NAME_LEN];
  char message[MAX_MSG_LEN];
  while (1) {
    printf("Enter recipient's name (or type 'exit' to quit): ");
    fgets(receiver, MAX_NAME_LEN, stdin);
    receiver[strcspn(receiver, "\n")] = '\0'; // Remove the newline character

    if (strcmp(receiver, "exit") == 0) {
      break;
    }

    printf("Enter your message: ");
    fgets(message, MAX_MSG_LEN, stdin);
    message[strcspn(message, "\n")] = '\0'; // Remove the newline character

    sendMessage(serverSocket, receiver, message);
  }

  close(serverSocket);
  return 0;
}

void connectToServer() {
  // Code for connecting to the server
  
}

void *receiveChat(void *arg) {
  int serverSocket = *((int *)arg);
  // Implementation of the ReceiveChat thread

  char buffer[MAX_MSG_LEN];
  while (1) {
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
      perror("Connection closed");
      break;
    }

    printf("Received: %s\n", buffer);
  }

  return NULL;
}

void sendConnCommand(int serverSocket, char *name) {
  // Send a connection command to the server
  char connCommand[MAX_NAME_LEN + 6]; // "CONN|" + name + "|"
  sprintf(connCommand, "CONN|%s|", name);
  send(serverSocket, connCommand, strlen(connCommand), 0);
}

void sendMessage(int serverSocket, char *receiver, char *message) {
  // Send a private message to the specified receiver
  char sendMessage[MAX_NAME_LEN + MAX_MSG_LEN +
                   6]; // "MESG|" + receiver + "|" + message + "|"
  sprintf(sendMessage, "MESG|%s|%s|", receiver, message);
  send(serverSocket, sendMessage, strlen(sendMessage), 0);
}
