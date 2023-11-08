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
void autentificare(int clientSocket);

int main() {
    int optiune;
    int clientSocket=connectToServer();
    printf("1.Login\n");
    printf("2.Autentificare\n");
    printf("3.Exit\n");
    printf("Dati optiunea:");
    scanf("%d",&optiune);
    if(optiune==1)
    {
        if (login(clientSocket)) {
            printf("Login reușit!\n");
        } else {
            printf("Login nereușit. Programul se închide.\n");
        }
    }
    else if(optiune==2)
    {
        autentificare(clientSocket);
    }
    else if(optiune==3)
    {
        exit(1);
    }

    cleanup(clientSocket);

    return 0;
}
void autentificare(int clientSocket)
{
    char username[20];
    char password[20];
    printf("Username:");
    scanf("%s",username);
    printf("Password:");
    scanf("%s",password);
    char messageToSend[80];
    strcpy(messageToSend,"2 ");
    strcat(messageToSend,username);
    strcat(messageToSend," ");
    strcat(messageToSend,password);
    char messageReceived[80];
    sendMessageToServer(clientSocket,messageToSend,messageReceived);
    if(strcmp(messageReceived,"EXISTA")==0)
        {
            printf("User-ul exista deja! Try again later!");
            exit(1);
        }
    if(strstr("OK",messageReceived)==0)
        {
            printf("Autentificare reusita cu succes!");
            exit(1);
        }
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

    return strcmp(messageToReceive,"DA") == 0;
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
        {   printf("%s",messageToReceive);
            break;
        }
    }
}

void cleanup(int clientSocket) {
    close(clientSocket);
}