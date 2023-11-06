#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 12345
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int connectToServer();
bool login(int clientSocket);
void sendMessageToServer(int clientSocket, char* messageToSend, char* messageToReceive);
void cleanup(int clientSocket);

int main() {
    int clientSocket = connectToServer();
    if (login(clientSocket)) {
        printf("Login reușit!\n");
    } else {
        printf("Login nereușit. Programul se închide.\n");
    }

    cleanup(clientSocket);

    return 0;
}

int connectToServer() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la crearea socket-ului client");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Eroare la conectarea la server");
        exit(1);
    }

    return clientSocket;
}

bool login(int clientSocket) {
    char username[20];
    char password[20];
    printf("Username:");
    scanf("%s", username);
    printf("Password:");
    scanf("%s", password);
    char messageToSend[50];
    char messageToReceive[50];
    strcpy(messageToSend,"1 ");
    strcat(messageToSend,username);
    strcat(messageToSend," ");
    strcat(messageToSend,password);
    
    sendMessageToServer(clientSocket, messageToSend, messageToReceive);

    return strcmp(messageToReceive, "DA") == 0;
}

void sendMessageToServer(int clientSocket, char* messageToSend, char* messageToReceive) {
    int sender = send(clientSocket, messageToSend, strlen(messageToSend), 0);
    if (sender <= 0) {
        perror("Eroare la trimiterea mesajului către Server!");
        exit(1);
    }
    while(listen(clientSocket,10))
    {
        int receiver = recv(clientSocket, messageToReceive, BUFFER_SIZE, 0);
        if (receiver <= 0) {
            perror("HELLO");
            
        }
        else
        {
            printf("%s",messageToReceive);
            break;
        }
    }
}

void cleanup(int clientSocket) {
    close(clientSocket);
}