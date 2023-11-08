#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#define PORT 12347
#define BUFFER_SIZE 1024

typedef struct listNode
{
    char username[30];
    char password[30];
    struct listNode*next;
}listNode;

void insertElement(listNode**A,char*username,char*password)
{
    if(*A==NULL)
    {
        (*A)=(listNode*)malloc(sizeof(listNode));
        strcpy((*A)->username,username);
        strcpy((*A)->password,password);
        (*A)->next=NULL;
    }
    else
    {
        listNode* newnode=(listNode*)malloc(sizeof(listNode));
        strcpy(newnode->password,password);
        strcpy(newnode->username,username);
        newnode->next=(*A);
        (*A)=newnode;
    }
}
bool findElement(listNode*A,char username[30],char password[30])
{
    while(A!=NULL)
    {
        if(strcmp(A->username,username)==0&&strcmp(A->password,password)==0)
            return true;
        A=A->next;
    }
    return false;
}
bool findUsername(listNode*A,char username[30])
{
    while(A!=NULL)
    {
        if(strcmp(A->username,username)==0)
        return true;
        A=A->next;
    }
    return false;
}

listNode*loginList=NULL;
void populate_loginList(listNode**list);
int establish_connection();
void run(int clientSocket);
void sendMessageToClient(int clientSocket, char* messageToSend);
void cleanup(int clientSocket);
char* execute_login(char*buffer);
char* execute_command(char*buffer);
bool verify_credentials(char*username,char*password,bool forLogin);
char* execute_autentificare(char* buffer);
int main() {
    populate_loginList(&loginList);
    run(establish_connection());
    return 0;
}


void populate_loginList(listNode**list)
{
    int fd=open("./serverUtils/credentials.txt",O_RDONLY);
    if(fd<0)
    {
        perror("Problema la deschiderea fisierului!");
        exit(1);
    }
    else
    {   char buffer[5000];
        int bufferSize=read(fd,buffer,5000);
        char*p =strtok(buffer,"\n ");
        int counter=0;
        char username[25];
        char password[25];
        while(p!=NULL)
        {   
            if(counter%2==0)
                strcpy(username,p);
                else
                {
                    strcpy(password,p);
                    insertElement(&*list,username,password);
                }
                counter++;
            p=strtok(NULL," \n");
        }
    }
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
     listNode*A=loginList;
        if(forLogin)
           {
             if(findElement(A,username,password));
                return true;
           }
        else
            if(findUsername(A,username))
                return true;
     return false;
}

char* execute_command(char* buffer)
{
    char*protocol=strtok(buffer," ");
    printf("%s",protocol);
    char* messageToSend=malloc(sizeof(char)*50);
    if(strcmp(protocol,"LOGIN")==0)
        strcpy(messageToSend,execute_login(buffer));
    if(strcmp(protocol,"AUTH")==0)
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
    int opt = 1;

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la crearea socket-ului server");
        exit(1);
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Eroare la legarea socket-ului la port");
        exit(1);
    }

    if (listen(serverSocket, 5) == -1) {
        perror("Eroare la ascultarea conexiunilor");
        exit(1);
    }

    printf("Serverul așteaptă conexiuni...\n");
    bool isClientConnected=false;
    // Așteptați conexiunea clientului doar dacă acesta nu este conectat
    if (!isClientConnected) {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == -1) {
            perror("Eroare la acceptarea conexiunii");
            exit(1);
        }
        isClientConnected = true; // Marcați starea conexiunii ca fiind adevărată
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