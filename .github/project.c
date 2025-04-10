#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char file_name[BUFFER_SIZE] = {0};

    // Receive file name from the client
    int bytes_read = read(client_socket, file_name, sizeof(file_name) - 1);
    if (bytes_read <= 0) {
        perror("Failed to read file name");
        close(client_socket);
        return;
    }
    file_name[bytes_read] = '\0'; // Null-terminate the file name

    printf("Client requested file: %s\n", file_name);

    // Open the requested file
    int file_fd = open(file_name, O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open file");
        write(client_socket, "File not found\n", 15);
        close(client_socket);
        return;
    }

    // Read the file and send its contents to the client
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    close(file_fd);
    close(client_socket);
    printf("File transfer complete and connection closed.\n");
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Bind socket to the specified port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("File server is running on port %d...\n", PORT);

    // Accept and handle incoming connections
    while ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0) {
        printf("New connection established.\n");
        handle_client(client_socket);
    }

    if (client_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return 0;
}