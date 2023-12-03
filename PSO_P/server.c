#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#define PORT 12347
#define BUFFER_SIZE 1024
typedef struct listNode
{
    char username[30];
    char password[30];
    struct listNode *next;
} listNode;
typedef struct searchTree
{
	char* key;
	bool isList;
    bool isSet;
	int numberOfElements;
	char**values;
	struct	searchTree*leftNode;
	struct	searchTree*rightNode;


}searchTree;
listNode *loginList = NULL;
searchTree* BST=NULL;
bool compare(char*key1,char*key2)
{
	if(strlen(key1)>strlen(key2))
		return true;
	else if(strlen(key2)>strlen(key1))
		return false;                       //asta compara cheile ca sa le introduca in ordine
	else
	{
		for(int i=0;i<strlen(key1);i++)
			{if(key1[i]>key2[i])
				return true;
			 else if(key1[i]<key2[i])
			 		return false;
			}
			
	}
	return false;
}
searchTree* minValueNode(searchTree* node) {
    searchTree* current = node;
    while (current->leftNode != NULL) {
        current = current->leftNode;
    }
    return current;
}

searchTree* deleteNode(searchTree* root, char* key) {
    if (root == NULL) {
        return root;
    }

    if (compare(key, root->key)) {
        root->rightNode = deleteNode(root->rightNode, key);
    } else if (compare(root->key, key)) {
        root->leftNode = deleteNode(root->leftNode, key);
    } else {
        // Nodul cu cheia specificată trebuie șters
        if (root->leftNode == NULL) {
            searchTree* temp = root->rightNode;
            free(root->key);
            free(root->values[0]);
            free(root->values);
            free(root);
            return temp;
        } else if (root->rightNode == NULL) {
            searchTree* temp = root->leftNode;
            free(root->key);
            free(root->values[0]);
            free(root->values);
            free(root);
            return temp;
        }

        // Nodul are doi copii, găsim succesorul său în ordine
        searchTree* temp = minValueNode(root->rightNode);

        // Copiem valorile din succesor în nodul curent
        free(root->key);
        free(root->values[0]);
        free(root->values);
        root->key = strdup(temp->key);
        root->values = (char**)malloc(sizeof(char*));
        root->values[0] = strdup(temp->values[0]);

        // Ștergem succesorul (deja am copiat valorile sale)
        root->rightNode = deleteNode(root->rightNode, temp->key);
    }
    return root;
}
searchTree* insertIntoTree(searchTree* node, char* key, char** values, bool hasList,bool hasSet,int numberOfElements)
{
    if (node == NULL)
    {
        node = (searchTree*)malloc(sizeof(searchTree));
        node->key = strdup(key);
        node->isList = hasList;

        // Alocare memorie separată pentru values și copierea valorii           
        node->values = (char**)malloc(sizeof(char*));
        node->values[0] = strdup(values[0]);
        node->isSet=hasSet;
        node->numberOfElements=numberOfElements;

        node->leftNode = NULL;
        node->rightNode = NULL;
        return node;
    }

    if (compare(key, node->key))
        node->rightNode = insertIntoTree(node->rightNode, key, values, hasList,hasSet,numberOfElements);
    else if (compare(node->key, key))
        node->leftNode = insertIntoTree(node->leftNode, key, values, hasList,hasSet,numberOfElements);

    return node;
}

searchTree* findElementByKey(searchTree* node, char* key) {
    if (node == NULL || strcmp(node->key, key) == 0) {
        return node;
    }

    if (compare(key, node->key)) {
        return findElementByKey(node->rightNode, key);
    } else {                                                                    //returneaza nodul din BST care are cheia respectiva
        return findElementByKey(node->leftNode, key);
    }
}

void insertElement(listNode **A, char *username, char *password)
{
    if (*A == NULL)
    {
        (*A) = (listNode *)malloc(sizeof(listNode));
        strcpy((*A)->username, username);
        strcpy((*A)->password, password);
        (*A)->next = NULL;
    }                                                                       //pentru autentificare si login urmatoarele 3 de la insertElement inclusiv
    else                    
    {
        listNode *newnode = (listNode *)malloc(sizeof(listNode));
        strcpy(newnode->password, password);
        strcpy(newnode->username, username);
        newnode->next = (*A);
        (*A) = newnode;
    }
}
bool findElement(listNode *A, char username[30], char password[30])
{
    while (A != NULL)
    {
        if (strcmp(A->username, username) == 0 && strcmp(A->password, password) == 0)
            return true;
        A = A->next;
    }
    return false;
}
bool findUsername(listNode *A, char username[30])
{
    while (A != NULL)
    {
        if (strcmp(A->username, username) == 0)
            return true;
        A = A->next;
    }
    return false;
}
void populate_BST();
void populate_loginList(listNode **list);
int establish_connection();
void run(int clientSocket);
void sendMessageToClient(int clientSocket, char *messageToSend);
void cleanup(int clientSocket);
char *execute_login(char *buffer);
char *execute_command(char *buffer);
char* execute_get(char* buffer);
bool verify_credentials(char *username, char *password, bool forLogin);
char *execute_autentificare(char *buffer);
void add_logger(char* text);
char *execute_logout();


int main()
{   populate_BST();
    populate_loginList(&loginList);
    run(establish_connection());
    return 0;
}
void populateSimple()
{
     FILE* f=fopen("./serverUtils/simple.txt","r");
    if(f==NULL)
    {
        perror("Eroare la deschiderea fisierului!");            //populeaza arborele cu tot ce se afla in fisierul simple.
        exit(1);
    }
    char buffer[300];
    searchTree*nod;
    if(!feof(f))
    {
    while(!feof(f))
    {
       fscanf(f,"%s",buffer);
       if(strcmp(buffer,"")==0)
           break;
       char key[100];
       char**values=(char**)malloc(sizeof(char*));
       values[0]=(char*)malloc(sizeof(char)*100);
       char*word=strtok(buffer,"-");
       strcpy(key,word);
       word=strtok(NULL,"-");
       if(word!=NULL)
         strcpy(values[0],word);
        BST=insertIntoTree(BST,key,values,false,false,1);
        nod=BST;
       printf("S-au introdus in tree: %s %s\n",key,values[0]);
      

    }
    }
     fclose(f);
}
void populateList()
{
    char buffer[1000];
    char**values;
    int counter=0;
    values=(char**)malloc(sizeof(char*)*1000);
    for(int i=0;i<1000;i++)
        values[i]=(char*)malloc(sizeof(char)*500);
    bool isSet=false;
    bool isList=true;                                                   //populeaza arborele cu tot ce se gaseste in list.txt
    FILE*f=fopen("./serverUtils/list.txt","r");
    if(f==NULL)
    {
        perror("Eroare la deschiderea fisierului!");
        exit(1);
    }
    char key[100];
    while(!feof(f))
    {   counter=0;
        fscanf(f,"%s",buffer);
        if(strcmp(buffer,"")==0)
            break;
        char*word=strtok(buffer,"-");
        strcpy(key,word);
        word=strtok(NULL,"-");
        char*p=strtok(word,",");
        while(p!=NULL)
        {
            strcpy(values[counter++],p);
            p=strtok(NULL,",");
        }
        BST=insertIntoTree(BST,key,values,isList,isSet,counter);
        printf("S-a introdus in tree lista cu cheia %s\n",key);
    }
    fclose(f);
}
void populate_BST()
{
   populateSimple();
   populateList();
}

void deleteFromFile(const char* file,char*key)
{
    char**vector=(char**)malloc(sizeof(char*)*1000);
    char buffer[100];
    int counter=0;                                                          //la apelarea DEl se actualizeaza fisierul si se sterge linia                                                            //de la cheia data ca arguments
    for(int i=0;i<10;i++)
        vector[i]=(char*)malloc(sizeof(char)*100);
    char copie[100];
    FILE*f=fopen(file,"r");
    while(!feof(f))
    {   
        fgets(buffer,100,f);
        if(strcmp(buffer,"")==0)
            break;
        strcpy(copie,buffer);
        char*key_F=strtok(copie,"-");
        if(strcmp(key_F,key)!=0)
            strcpy(vector[counter++],buffer);
    }
    fclose(f);
    FILE*of=fopen(file,"w");
    for(int i=0;i<counter;i++)
    {   
        fprintf(of,"%s\n",vector[i]);
    }
    fclose(of);
}
void populate_loginList(listNode **list)
{
    int fd = open("./serverUtils/credentials.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Problema la deschiderea fisierului!");
        exit(1);
    }
    else
    {
        char buffer[5000];
        int bufferSize = read(fd, buffer, 5000);
        char *p = strtok(buffer, "\n ");
        int counter = 0;
        char username[25];
        char password[25];
        while (p != NULL)
        {
            if (counter % 2 == 0)
                strcpy(username, p);
            else
            {
                strcpy(password, p);                                    //populeaza lista pt login din RAM
                insertElement(&*list, username, password);
            }
            counter++;
            p = strtok(NULL, " \n");
        }
    }
}
char* execute_get(char*buffer)
{
    char copie[250];
    char*bufferToReturn=(char*)malloc(sizeof(char)*100);
    strcpy(copie,buffer);
    char*word=strtok(copie," ");
    word=strtok(NULL," ");
    searchTree*node=findElementByKey(BST,word);                 //functii de executie
    if(node==NULL)
    {
        strcpy(bufferToReturn,"EROARE GET");
        bufferToReturn[strlen(bufferToReturn)]='\0';
        return bufferToReturn;
    }
    else
    {
        if(!node->isList)
           strcpy(bufferToReturn,node->values[0]);
        bufferToReturn[strlen(bufferToReturn)]='\0';
        return bufferToReturn;
    }
        

}
char *execute_autentificare(char *buffer)
{
    char *word = strtok(buffer, " ");
    char *username = (char *)malloc(sizeof(char) * 30);
    char *password = (char *)malloc(sizeof(char) * 30);
    int counter = 0;
    while (word != NULL)
    {
        if (counter == 1)
            strcpy(username, word);
        else if (counter == 2)
            strcpy(password, word);
        counter++;
        word = strtok(NULL, " ");
    }
    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';
    if (verify_credentials(username, password, false))
        return "EXISTA";
    else
    {
        FILE *f = fopen("./serverUtils/credentials.txt", "a");
        fclose(f);
        return "OK";
    }
}
char* execute_listCreation(char*buffer)
{   char copie[100];
    char**values=(char**)malloc(sizeof(char*)*1000);
    for(int i=0;i<1000;i++)
        values[i]=(char*)malloc(sizeof(char)*500);
    strcpy(copie,buffer);
    char*key=strtok(copie," ");
    key=strtok(NULL," ");
    if(findElementByKey(BST,key))
        return "EXISTA_DEJA";
    else
    {
        FILE*f=fopen("./serverUtils/list.txt","a");
        fprintf(f,"%s-\n",key);
        BST=insertIntoTree(BST,key,values,true,false,0);
        fclose(f);
        return "OK";
    }

}
char* execute_del(char*buffer)
{
    char copie[100];
    char key[50];
    strcpy(copie,buffer);
    char*word=strtok(copie," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    if(!findElementByKey(BST,key))
        return "CHEIE_INEXISTENTA";
    else
        {   
            BST=deleteNode(BST,key);
            deleteFromFile("./serverUtils/simple.txt",key);
            deleteFromFile("./serverUtils/list.txt",key);
            return "OK";
        }

}
char* execute_set(char*buffer)
{
    char copie[250];
    char key[100];
    char**value=(char**)malloc(sizeof(char*));
    value[0]=(char*)malloc(sizeof(char)*100);
    strcpy(copie,buffer);
    char*word=strtok(copie," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value[0],word);
    if(!findElementByKey(BST,key))
    {
        BST=insertIntoTree(BST,key,value,false,false,1);
        FILE*f=fopen("./serverUtils/simple.txt","a");
        fprintf(f,"%s-%s\n",key,value[0]);
        fflush(f);
        fclose(f);
        return "OK";
    }
    else return "DEJA_EXISTA";
    
}
bool verify_credentials(char *username, char *password, bool forLogin)
{
    listNode *A = loginList;
    if (forLogin)
    {
        if (findElement(A, username, password))
            return true;
    }
    else if (findUsername(A, username))
        return true;
    return false;
}

char *execute_command(char *buffer)
{   
    char *copie=(char*)malloc(sizeof(char)*40);
    strcpy(copie, buffer);
    char *protocol = strtok(copie, " ");
    char *messageToSend = malloc(sizeof(char) * 50);
    strcpy(messageToSend,"");
    if (strcmp(protocol, "LOGIN") == 0)
        strcpy(messageToSend, execute_login(buffer));
    else
    if (strcmp(protocol, "AUTH") == 0)
        strcpy(messageToSend, execute_autentificare(buffer));
    else
    if(strcmp(protocol, "GET")==0)
        strcpy(messageToSend,execute_get(buffer));
    if(strcmp(protocol,"SET")==0)
        strcpy(messageToSend,execute_set(buffer));
    if(strcmp(protocol,"DEL")==0)
        strcpy(messageToSend,execute_del(buffer));
    if(strcmp(protocol,"LIST")==0)
        strcpy(messageToSend,execute_listCreation(buffer));
    if(strcmp(protocol,"LOGOUT")==0)
        strcpy(messageToSend,execute_logout());
    return messageToSend;
}
char *execute_login(char *buffer)
{
    char *word = strtok(buffer, " ");
    char *username = (char *)malloc(sizeof(char) * 30);
    char *password = (char *)malloc(sizeof(char) * 30);
    char *bufferToReturn = (char *)malloc(sizeof(char) * 3);
    int counter = 0;
    while (word != NULL)
    {
        if (counter == 1)
            strcpy(username, word);
        else if (counter == 2)
            strcpy(password, word);
        counter++;
        word = strtok(NULL, " ");
    }
    username[strlen(username)] = '\0';
    password[strlen(password)] = '\0';
    if (verify_credentials(username, password, true))
        {strcpy(bufferToReturn, "DA");
        add_logger("Utilizatorul s-a autentificat cu succes!");
        }
    else
        strcpy(bufferToReturn, "NU");
    bufferToReturn[strlen(bufferToReturn)] = '\0';
    return bufferToReturn;
}

int establish_connection()
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int opt = 1;

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare la crearea socket-ului server");
        exit(1);
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Eroare la legarea socket-ului la port");
        exit(1);
    }

    if (listen(serverSocket, 5) == -1)
    {
        perror("Eroare la ascultarea conexiunilor");
        exit(1);
    }

    printf("Serverul așteaptă conexiuni...\n");

    while (1)
    {
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == -1)
        {
            perror("Eroare la acceptarea conexiunii");
            exit(1);
        }

        printf("Clientul s-a conectat.\n");
        run(clientSocket);
        close(clientSocket);
    }

    return 0;
}
void run(int clientSocket)
{

    printf("Clientul s-a conectat.\n");
    char buffer[BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int messageReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if(messageReceived==0)
        {
            printf("clientul s-a deconectat!");
        }
        if (messageReceived > 0)
        {
            // Aici functie execute_command care returneaza un char* de trimis,In ea vor fi functii pentru fiecare comanda(login,autentif,set,get,...);
            char* message=(char*)malloc(sizeof(char)*100);
            if (strcmp(buffer, "LOGOUT") == 0)
            {
                strcpy(message, execute_command(buffer));
            message[strlen(message)]='\0';
            sendMessageToClient(clientSocket, message);
                break;
            }
            strcpy(message, execute_command(buffer));
            message[strlen(message)]='\0';
            sendMessageToClient(clientSocket, message);
            free(message);
        }
    }

    cleanup(clientSocket);
}

void sendMessageToClient(int clientSocket, char *messageToSend)
{
    int messageToSendLength = strlen(messageToSend);
    int messageSent = send(clientSocket, messageToSend, messageToSendLength, 0);
    if (messageSent < messageToSendLength)
    {
        perror("Eroare la trimiterea mesajului către client!");

        cleanup(clientSocket);
        exit(1);
    }
}

void cleanup(int clientSocket)
{
    close(clientSocket);
}

void add_logger(char* text)
{
    time_t timp_actual = time(NULL);
    struct tm *info_timp = localtime(&timp_actual);
    char timp_str[50];
    strftime(timp_str, sizeof(timp_str), "%H:%M:%S %x", info_timp);
    FILE *fisier=fopen("./serverUtils/logger.txt","a");
    if(fisier==NULL)
    {
        perror("Erorare la deschidere fisier!");
        exit(-1);
    }
    fprintf(fisier,"%s %s\n",text,timp_str);
    fclose(fisier);
}

char* execute_logout()
{
    char* aux="OK";
    add_logger("Utilizatorul s-a delogat!");
    return aux;
}