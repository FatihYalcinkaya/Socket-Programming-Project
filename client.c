// client.c

#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define MAX_MESSAGE_SIZE 1024

typedef struct {
  char sender[50];
  char recipient[50];
  char message[MAX_MESSAGE_SIZE];
} PrivateMessage;

void *HandleUserInput(void *arg) {
  int clientSocket = *((int *)arg);
  char recipient[50];
  char message[MAX_MESSAGE_SIZE];
  PrivateMessage privateMessage;

  // Get the user's name
  printf("Enter your name: ");
  if (fgets(privateMessage.sender, sizeof(privateMessage.sender), stdin) ==
      NULL) {
    perror("Error reading user input");
    close(clientSocket);
    pthread_exit(NULL);
  }

  privateMessage.sender[strcspn(privateMessage.sender, "\n")] =
      '\0'; // Remove the newline character

  // Send the user's name to the server
  if (send(clientSocket, privateMessage.sender, strlen(privateMessage.sender),
           0) == -1) {
    perror("Error sending user name to the server");
    close(clientSocket);
    pthread_exit(NULL);
  }

  while (1) {
    // Get recipient and message from user input
    printf("Enter recipient's name (or 'exit' to quit): ");
    if (fgets(recipient, sizeof(recipient), stdin) == NULL) {
      perror("Error reading user input");
      break; // Break the loop on input error
    }
    recipient[strcspn(recipient, "\n")] = '\0'; // Remove the newline character

    // Check if the user wants to exit
    if (strcmp(recipient, "exit") == 0) {
      break;
    }

    printf("Enter message: ");
    if (fgets(message, sizeof(message), stdin) == NULL) {
      perror("Error reading user input");
      break; // Break the loop on input error
    }
    message[strcspn(message, "\n")] = '\0'; // Remove the newline character

    // Prepare the private message
    strncpy(privateMessage.recipient, recipient,
            sizeof(privateMessage.recipient));
    strncpy(privateMessage.message, message, sizeof(privateMessage.message));

    // Send the private message to the server
    if (send(clientSocket, &privateMessage, sizeof(PrivateMessage), 0) == -1) {
      perror("Error sending message to the server");
      break; // Break the loop on send error
    }
  }

  close(clientSocket);
  pthread_exit(NULL);
}

int main() {
  int clientSocket;
  struct sockaddr_in serverAddr;

  // Create socket
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  // Set up server address
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  serverAddr.sin_port = htons(PORT);

  // Connect to the server
  if (connect(clientSocket, (struct sockaddr *)&serverAddr,
              sizeof(serverAddr)) == -1) {
    perror("Error connecting to the server");
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Create a thread to handle user input
  pthread_t thread;
  if (pthread_create(&thread, NULL, HandleUserInput, &clientSocket) != 0) {
    perror("Error creating thread");
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Wait for the thread to finish
  pthread_join(thread, NULL);

  close(clientSocket);
  return 0;
}
