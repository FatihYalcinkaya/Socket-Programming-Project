#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_NAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 256

int server_socket;

// Utility function to calculate simple parity
int calculate_parity(char *message) {
  int parity = 0;
  for (int i = 0; i < strlen(message); i++) {
    parity ^= message[i];
  }
  return parity;
}

// Utility function to calculate checksum
int calculate_checksum(char *message) {
  int checksum = 0;
  for (int i = 0; i < strlen(message); i++) {
    checksum += message[i];
  }
  return checksum;
}

// Utility function to calculate CRC (Cyclic Redundancy Check)
int calculate_crc(char *message) {
  int crc = 0;
  for (int i = 0; i < strlen(message); i++) {
    crc ^= message[i];
  }
  return crc;
}

void *receive_chat(void *arg) {
  while (1) {
    char buffer[MAX_MESSAGE_LENGTH];
    memset(buffer, 0, sizeof(buffer));

    // Receive data from the server
    ssize_t bytes_received = recv(server_socket, buffer, sizeof(buffer), 0);

    // Check for errors in received data
    // Perform error checking based on the specified methods (simple parity,
    // checksum, CRC)
    int received_parity = buffer[0];
    int calculated_parity = calculate_parity(buffer + 1);

    int received_checksum = buffer[1];
    int calculated_checksum = calculate_checksum(buffer + 2);

    int received_crc = buffer[2];
    int calculated_crc = calculate_crc(buffer + 3);

    if (received_parity == calculated_parity &&
        received_checksum == calculated_checksum &&
        received_crc == calculated_crc) {
      // Data is correct, proceed with further processing
      // ...

      // Example: Display the received message
      printf("%s\n", buffer + 3);
    } else {
      // Send an error message back to the server
      char error_message[] = "MERR|";
      send(server_socket, error_message, sizeof(error_message), 0);
    }
  }
}

void send_message(char *message) {
  // Calculate error checking bits (simple parity, checksum, CRC)
  int parity = calculate_parity(message);
  int checksum = calculate_checksum(message);
  int crc = calculate_crc(message);

  // Construct the final message to send
  char final_message[MAX];
}