#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#define PORT 12347
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
int numarCaracterePrimite;
int connectToServer();
bool login(int clientSocket);
void sendMessageToServer(int clientSocket, char* messageToSend, char* messageToReceive);
void cleanup(int clientSocket);
void autentificare(int clientSocket);
void run_commander();
void run();
char*user=NULL;
int main() {
   
    run();
    return 0;
}
void run()
{
    int optiune;
    int clientSocket;
    do{
    clientSocket=connectToServer();
    printf("1.Login\n");
    printf("2.Autentificare\n");
    printf("3.Exit\n");
    printf("Dati optiunea:");
    scanf("%d",&optiune);
    if(optiune==1)
    {
        if (login(clientSocket)) {

            printf("Login reușit!\n");
            run_commander(clientSocket);
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
    }while(1);
    cleanup(clientSocket);
}
char* getType(char*command)
{   char aux[250];
    strcpy(aux,command);
    char*type=strtok(aux," ");
    return type;
}
bool commandWith3Words(char*type)
{
    if(strcmp(type,"SET")==0 || strcmp(type,"RPUSH")==0 || strcmp(type,"LPUSH")==0 || strcmp(type,"SADD")==0
    || strcmp(type,"SREM")==0 || strcmp(type,"SISMEMBER")==0 || strcmp(type,"SUNION")==0 || strcmp(type,"SINTER")==0 || strcmp(type,"CHANGEV")==0
    || strcmp(type,"GETSET")==0 ||  strcmp(type,"LREM")==0)
        return true;
    return false;
}
bool commandWith2Words(char*type)
{
    if(strcmp(type,"GET")==0 || strcmp(type,"DEL")==0 || strcmp(type,"LPOP")==0 || strcmp(type,"RPOP")==0 ||
    strcmp(type,"SMEMBERS")==0 || strcmp(type,"SCARD") ==0 || strcmp(type,"LIST")==0 || strcmp(type,"SETC")==0 || strcmp(type,"STRLEN")==0)
        return true;
    return false;
}
bool commandWith4Words(char*type)
{
    if(strcmp(type,"LRANGE")==0 || strcmp(type,"GETRANGE")==0 || strcmp(type,"LSET")==0)
        return true;
    return false;
}
bool commandWith1Word(char*type)
{
    if(strcmp(type,"HELP")==0 || strcmp(type,"LOGOUT")==0 ||  strcmp(type,"LOGGER")==0 || strcmp(type,"FINISH")==0 || strcmp(type,"EXIT")==0 ||
    strcmp(type,"KEYS")==0)
        return true;
    return false;
}
int countWords(char* command)
{
    int numberOfWords=0;
    char*p=strtok(command," ");
    while(p!=NULL)
    {
        numberOfWords++;
        p=strtok(NULL," ");
    }
    return numberOfWords;
}
bool verify_numberOfWords(char* command)
{

    if(commandWith1Word(getType(command)))
    {
       if(countWords(command)==1)
            return true;
        return false;
    }
    else if(commandWith2Words(getType(command)))
    {
        if(countWords(command)==2)
            return true;
        return false;
    }
    else if(commandWith3Words(getType(command)))
    {
        if(countWords(command)==3)
            return true;
        return false;
    }
    else 
    {if(countWords(command)==4)
        return true;
    return false;
    }

    

}
bool verify_command(char* command)
{   
    char*type=getType(command);
    int fd=open("comenzi.txt",O_RDONLY);
    if(fd<0)
    {
        perror("Eroare la deschiderea fisierului de comenzi!");
        exit(1);
    }
    else
    {
        char buffer[250];
        int bytesNumber=read(fd,buffer,250);
        char*line=strtok(buffer,"\n");
        while(line!=NULL)
        {  
            if(strcmp(line,type)==0)
            {
                if(verify_numberOfWords(command))
                    return true;
                else
                {
                    printf("Comanda nu are numarul prevazut de cuvinte!\n");
                    return false;
                }
            }
            line=strtok(NULL,"\n ");
        }
    }
    return false;
}
void run_commander(int clientSocket)
{   
    char*command=(char*)malloc(sizeof(char)*50);
    char*command_copy=(char*)malloc(sizeof(char)*50);
    char*receiver=(char*)malloc(sizeof(char)*40);
    char*messageToReceive;
    bool firstTry=true;
    fgets(command,50,stdin);
    do
    {   
          printf("\033[1;31m%s>\033[0m ",user);
          fflush(stdout);
          fgets(command,50,stdin);
          command[strlen(command)-1]='\0';
          strcpy(command_copy,command);
        if(!verify_command(command_copy))
            {printf("Comanda gresita!Incercati din nou!\n");
             free(command_copy);
            }
        else
        {   char copie[50];
            strcpy(copie,command);
            char*protocol=strtok(copie," ");
            if(strcmp(command,"EXIT")==0)
            {   cleanup(clientSocket);
                free(command);
                free(command_copy);
                exit(1);
            }
            messageToReceive=(char*)malloc(sizeof(char)*256);
            sendMessageToServer(clientSocket,command,messageToReceive);
            if(strcmp(protocol,"GET")==0)
            {
            if(strncmp(messageToReceive,"EROARE GET",10)==0)
                printf("EROARE - CHEIA NU EXISTA!\n");
            else
            {   char*listIdentifier=strtok(messageToReceive,":");
                if(strcmp(listIdentifier,"list")!=0)
                {
                for(int i=0;i<numarCaracterePrimite;i++)
                    printf("%c",messageToReceive[i]);
                printf("\n");
                }
                else
                {
                    listIdentifier=strtok(NULL,":");
                    printf("%s\n",listIdentifier);
                }
            }
            }
            else
            if(strcmp(protocol,"SET")==0)
            {
                if(strncmp(messageToReceive,"DEJA_EXISTA",11)==0)
                    printf("Eroare - Cheia deja exista!\n");
                else printf("Pereche adaugata!\n");
            }
            else
            if(strcmp(protocol,"DEL")==0)
            {
                if(strncmp(messageToReceive,"OK",2)==0)
                    printf("Stergere cu succes\n");
                else printf("Cheie inexistenta...\n");
            }
            else
            if(strcmp(protocol,"LIST")==0)
            {
                if(strncmp(messageToReceive,"OK",2)==0)
                    printf("Lista creata cu succes!\n");
                else printf("Cheia exista deja...");
            }
            else
              if(strcmp(protocol,"LOGOUT")==0)
            {
                if(strncmp(messageToReceive,"OK",2)==0)
                   { 
                    printf("Deconectare cu succes!\n");
                    cleanup(clientSocket);
                    free(user);
                    break;

                    }
                else
                    printf("NU s-a putut deconecta!");
            }
            else
            if(strcmp(protocol,"RPUSH")==0)
            {
                if(strncmp(messageToReceive,"OK",2)==0)
                    printf("Element adaugat cu succes!\n");
                else printf("Eroare la adaugare element!Cheia nu exista...\n");
            }
            else
            if(strcmp(protocol,"LPUSH")==0)
            {
                if(strncmp(messageToReceive,"OK",2)==0)
                    printf("Element adaugat cu succes!\n");
                else printf("Eroare la adugare element!Cheia nu exista...\n");
            }
            else printf("%s\n",messageToReceive);


            free(messageToReceive);
        }
        
    }while(1);
    cleanup(clientSocket);
    free(receiver);
    free(command);
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
    strcpy(messageToSend,"AUTH ");
    strcat(messageToSend,username);
    strcat(messageToSend," ");
    strcat(messageToSend,password);
    char messageReceived[80];
    sendMessageToServer(clientSocket,messageToSend,messageReceived);
    if(strcmp(messageReceived,"EXISTA")==0)
        {
            printf("User-ul exista deja! Try again later!");
            cleanup(clientSocket);
            exit(1);
        }
    if(strstr("OK",messageReceived)==0)
        {
            printf("Autentificare reusita cu succes!");
            cleanup(clientSocket);
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
    strcpy(messageToSend,"LOGIN ");
    strcat(messageToSend,username);
    strcat(messageToSend," ");
    strcat(messageToSend,password);
    
    
    sendMessageToServer(clientSocket, messageToSend, messageToReceive);
    if(strncmp(messageToReceive,"DA",2) == 0)
    {
        user=(char*)malloc(sizeof(char)*strlen(username));
        strcpy(user,username);
        return true;
    }
    if(strncmp(messageToReceive,"ONLINE",6)==0)
    {
        printf("Eroare! USER DEJA CONECTAT!\n");
        return false;
    }
    if(strncmp(messageToReceive,"NO",2)==0)
    {
        printf("Eroare! Credentiale invalide!");
        return false;
    }
    
}

void sendMessageToServer(int clientSocket, char* messageToSend, char* messageToReceive) {
    int sender = send(clientSocket, messageToSend, strlen(messageToSend), 0);
    if (sender <= 0) {
        perror("Eroare la trimiterea mesajului către Server!");
        exit(1);
    }
    int caract;
    numarCaracterePrimite = recv(clientSocket, messageToReceive, BUFFER_SIZE, 0);
    caract=numarCaracterePrimite;
    if (numarCaracterePrimite <= 0) {
        printf("Aplicatia se va inchide in cateva momente...");
        exit(1);
    }
    messageToReceive[numarCaracterePrimite]='\0';
}

void cleanup(int clientSocket) {
    close(clientSocket);
}