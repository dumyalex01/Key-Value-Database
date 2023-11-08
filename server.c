#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int establish_connection();
void run(int clientSocket);
void sendMessageToClient(int clientSocket, char* messageToSend);
void cleanup(int clientSocket);
char* execute_login(char*buffer);
char* execute_command(char*buffer);
bool verify_credentials(char*username,char*password,bool forLogin);
char* execute_autentificare(char* buffer);
int main() {
    
   //run(establish_connection());
    return 0;
}

char* execute_autentificare(char*buffer)
{
    char*word=strtok(buffer," ");
    char username[20];
    char password[20];
    int counter=0;
    while(word!=NULL)
    {
        if(counter==1)
            strcpy(username,word);
            else if(counter==2)
                strcpy(password,word);
        counter++;
        word=strtok(NULL," ");
    }
    if(verify_credentials(username,password,false))
        return "EXISTA";
     else {
    FILE*f =fopen("./serverUtils/credentials.txt","a");
    fprintf(f,"%s %s\n",username,password);
    fclose(f);
     return "OK";
     }
}
bool verify_credentials(char*username,char*password,bool forLogin)
{  
     FILE*f=fopen("./serverUtils/credentials.txt","rt");
     if(f==NULL)
     {
        perror("Eroare la deschiderea fisierului de credentiale!");
        exit(1);
     }
     
     char username_buff[20];
     char password_buff[20];
     while(!feof(f))
     {
        fscanf(f,"%s %s",username_buff,password_buff);
        if(forLogin)
        {
        if(strcmp(username_buff,username)==0 && strcmp(password_buff,password)==0)
            return true;
        }
        else if(strcmp(username_buff,username)==0)
            return true;

     }
     return false;
}
char* execute_command(char* buffer)
{
    char protocol=buffer[0];
    char* messageToSend=malloc(sizeof(char)*50);
    if(buffer[0]=='1')
        strcpy(messageToSend,execute_login(buffer));
    if(buffer[0]=='2')
        strcpy(messageToSend,execute_autentificare(buffer));
    return messageToSend;

}
char* execute_login(char* buffer)
{   
    char*word=strtok(buffer," ");
    char username[20];
    char password[20];
    int counter=0;
    while(word!=NULL)
    {
        if(counter==1)
            strcpy(username,word);
            else if(counter==2)
                    strcpy(password,word);
        counter++;
        word=strtok(NULL," ");
    }
    if(verify_credentials(username,password,true))
        return "DA";
        else return "NU";
    
}

int establish_connection() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la crearea socket-ului server");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Eroare la legarea socket-ului la port");
        exit(1);
    }

    if (listen(serverSocket, 5) == -1) {
        perror("Eroare la ascultarea conexiunilor");
        exit(1);
    }

    printf("Serverul așteaptă conexiuni...\n");

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == -1) {
        perror("Eroare la acceptarea conexiunii");
        exit(1);
    }
    close(serverSocket);
    return clientSocket;
}

void run(int clientSocket) {
   
    printf("Clientul s-a conectat.\n");
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int messageReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
       if(messageReceived>0)
       {
        //Aici functie execute_command care returneaza un char* de trimis,In ea vor fi functii pentru fiecare comanda(login,autentif,set,get,...);
        char message[250];
        strcpy(message,execute_command(buffer));
        sendMessageToClient(clientSocket, message);
        
        }
    }
    cleanup(clientSocket);
}

void sendMessageToClient(int clientSocket, char* messageToSend) {
    int messageToSendLength = strlen(messageToSend);
    int messageSent = send(clientSocket, messageToSend, messageToSendLength, 0);
    if (messageSent < messageToSendLength) {
        perror("Eroare la trimiterea mesajului către client!");
        exit(1);
    }
}

void cleanup(int clientSocket) {
    close(clientSocket);
}