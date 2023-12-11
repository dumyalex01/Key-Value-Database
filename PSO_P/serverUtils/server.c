#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#define PORT 12347
#define BUFFER_SIZE 2500
#define TYPE_SIMPLE 1
#define TYPE_LIST 2
#define TYPE_SET 3
bool empty_file(FILE*f)
{
    fseek(f,0,SEEK_END);
    int length=ftell(f);
    if(length>0)
        return false;
    return true;
}
typedef struct client_info
{
    int clientSocket;
    
}client_info;
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
    bool isOrdonated; //
    int priorites[50]; //
	int numberOfElements;
	char**values;
	struct	searchTree*leftNode;
	struct	searchTree*rightNode;


}searchTree;
listNode *loginList = NULL;
searchTree* BST=NULL;
char user[50];
void updateFile(char*key,int type);
bool compare(char*key1,char*key2)
{
	if(strlen(key1)>strlen(key2))
		return true;
	else if(strlen(key2)>strlen(key1))
		return false;                       //asta compara cheile ca sa le introduca in ordine
	else
	{
		for(int i=0;i<strlen(key1);i++)
			{
                if(key1[i]>key2[i])
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
            //TODO verifcia aici
            if(root->isOrdonated==true)
            {
                for(int i=0;i<root->numberOfElements;i++)
                    free(root->values[i]);
            }
            else
                free(root->values[0]);
            free(root->values);
            free(root);
            return temp;
        } else if (root->rightNode == NULL) {
            searchTree* temp = root->leftNode;
            free(root->key);
             if(root->isOrdonated==true)
            {
                for(int i=0;i<root->numberOfElements;i++)
                    free(root->values[i]);
            }
            else
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

void swapSet(searchTree* node) //functie pentru swap la set ordonat
{
     int swapped;

do {
    swapped = 0;

    for (int i = 0; i < node->numberOfElements - 1; i++) {
        if (node->priorites[i] > node->priorites[i + 1]) {
            // Schimbăm prioritățile
            int tempPriority = node->priorites[i];
            node->priorites[i] = node->priorites[i + 1];
            node->priorites[i + 1] = tempPriority;

            // Schimbăm valorile corespunzătoare
            char tempValue[100];
            strcpy(tempValue, node->values[i]);
            strcpy(node->values[i], node->values[i + 1]);
            strcpy(node->values[i + 1], tempValue);
            swapped = 1;
        }
    }
} while (swapped);
}

bool findElementSort(searchTree* node,char**values,int *priorites)
{
    for(int i=0;i<node->numberOfElements;i++)
    {
        if(strcmp(node->values[i],values[0])==0)
        {
            node->priorites[i]=priorites[0];
            return true;
        }
    }
    return false;
}

searchTree* insertIntoTree(searchTree* node, char* key, char** values, bool hasList,bool hasSet,int numberOfElements,bool isOrdonated,int* priorites)
{
    if (node == NULL)
    {
        if(isOrdonated==true && hasSet==false && hasList==false)
        {
        node = (searchTree*)malloc(sizeof(searchTree));
        node->key = strdup(key);
       
        node->values = (char**)malloc(sizeof(char*)*50);
        for(int i=0;i<numberOfElements;i++)
        {
            node->values[i]=(char*)malloc(sizeof(char)*100);
                    if(values[i]!=NULL)
                    strcpy(node->values[i],values[i]);
        }
        for(int i=0;i<10;i++)
            node->priorites[i]=priorites[i];
        node->numberOfElements=numberOfElements;
        node->isList=false;
        node->isOrdonated=true;
        node->isSet=false;
        node->leftNode = NULL;
        node->rightNode = NULL;
        return node;
        }
        else
        {
            node = (searchTree*)malloc(sizeof(searchTree));
        node->key = strdup(key);
        node->isList = hasList;
        node->isSet=hasSet;
        node->isOrdonated=isOrdonated;
        // Alocare memorie separată pentru values și copierea valorii           
        if(node->isList || node->isSet)
            {
              node->values = (char**)malloc(sizeof(char*)*100);
                for(int i=0;i<100;i++)
                    node->values[i]=(char*)malloc(sizeof(char)*50);
                for(int i=0;i<numberOfElements;i++)
                    strcpy(node->values[i],values[i]);
            }

        else 
        {
        node->values=(char**)malloc(sizeof(char*));
        node->values[0] = strdup(values[0]);
        }
        node->numberOfElements=numberOfElements;

        node->leftNode = NULL;
        node->rightNode = NULL;
        return node;    
        }
    }
    else if(isOrdonated==true)
    {
        if(strcmp(node->key,key)==0)
    {
            if(findElementSort(node,values,priorites)==true)
            {
                swapSet(node);
            }
            else
            {
            node->values[node->numberOfElements]=(char*)malloc(sizeof(char)*100);
            strcpy(node->values[node->numberOfElements],values[0]);
            node->priorites[node->numberOfElements]=priorites[0];
            node->numberOfElements++;
            swapSet(node);
            }
            
        return node;
    }
    }

    if (compare(key, node->key))
        node->rightNode = insertIntoTree(node->rightNode, key, values, hasList,hasSet,numberOfElements,isOrdonated,priorites);
    else if (compare(node->key, key))
        node->leftNode = insertIntoTree(node->leftNode, key, values, hasList,hasSet,numberOfElements,isOrdonated,priorites);

    return node;
}

searchTree* findElementByKey(searchTree* node, char* key) {  //TODO acelasi cod cu cel find element Ord
    if (node == NULL || strcmp(node->key, key) == 0) {
        return node;
    }

    if (compare(key, node->key)) {
        return findElementByKey(node->rightNode, key);
    } else {                                                                    //returneaza nodul din BST care are cheia respectiva
        return findElementByKey(node->leftNode, key);
    }
}
searchTree* addValueToRight(searchTree* node,char*key,char* value)
{   
    searchTree*currentNode=findElementByKey(node,key);
    if(currentNode->isList)
    {
    if(currentNode==NULL)
    {
        return NULL;
    }
    else
    {
        if(currentNode->numberOfElements==0)
        {
            currentNode->numberOfElements++;
            strcpy(currentNode->values[0],value);
        }
        else
        {
            currentNode->numberOfElements=currentNode->numberOfElements+1;
            strcpy(currentNode->values[currentNode->numberOfElements-1],value);
        }
        return currentNode;
    }
    }
    else return NULL;
}
searchTree* addValueToLeft(searchTree*node,char*key,char*value)
{
    searchTree*nod=findElementByKey(node,key);
    if(nod==NULL)
        return NULL;
    else
    {
        if(nod->numberOfElements==0)
        {
            nod->numberOfElements++;
            strcpy(nod->values[nod->numberOfElements-1],value);
        }
        else
        {
            nod->numberOfElements++;
            for(int i=nod->numberOfElements-1;i>=1;i--)
                strcpy(nod->values[i],nod->values[i-1]);
            strcpy(nod->values[0],value);
        }
        return nod;
    }
}
searchTree* addValueToSet(searchTree*node,char*key,char*value)
{   
    searchTree*nod=findElementByKey(node,key);
    if(nod==NULL)
        return NULL;
    else
    {
        if(!nod->isSet)
            return NULL;
        else
        {
            if(nod->numberOfElements==0)
            {nod->numberOfElements++;
            strcpy(nod->values[0],value);
            }
            else
            {
            nod->numberOfElements++;
            strcpy(nod->values[nod->numberOfElements-1],value);
            }
            return nod;
        }
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
void runApp(int clientSocket);
void sendMessageToClient(int clientSocket, char *messageToSend);
void cleanup(int clientSocket);
char *execute_login(char *buffer);
char *execute_command(char *buffer);
char* execute_get(char* buffer);
bool verify_credentials(char *username, char *password, bool forLogin);
char *execute_autentificare(char *buffer);
void add_logger(char*text);
char*execute_logout();
char* execute_zadd(char* buffer);
char* execute_zcard(char* buffer);
char* execute_zscore(char* buffer);
char* execute_zcount(char* buffer);
char* execute_zincrby(char* buffer);
char* execute_zrank(char* buffer);
char * execute_zrange(char* buffer);
char* execute_zrem(char* buffer);
char* execute_zunion(char* buffer);
char* execute_zunionstore(char* buffer);
char* execute_help(char* buffer);
char* execute_zdel(char* buffer);
void handle_sigint(int signum) {
    printf("CTRL+C detected. Cleaning file...\n");

    FILE *file = fopen("./serverUtils/online", "w");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului");
        exit(EXIT_FAILURE);
    }

    if (truncate("./serverUtils/online", 0) == -1) {
        perror("Eroare la trunchierea fișierului");
        exit(EXIT_FAILURE);
    }

    fclose(file);
    exit(1);
}

int main()
{   populate_BST();
    populate_loginList(&loginList);
    establish_connection();
    return 0;
}
void populateSimple()
{
     FILE* f=fopen("./serverUtils/simple.txt","r");
    if(f==NULL)
    {
        perror("Eroare la deschiderea fisierului! simple.txt");            //populeaza arborele cu tot ce se afla in fisierul simple.
        exit(1);
    }
    char buffer[300];
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
       BST=insertIntoTree(BST,key,values,false,false,1,false,NULL);
       printf("S-au introdus in tree: %s %s\n",key,values[0]);
      

    }
    }
     fclose(f);
}
void populateListandSet(bool isList1,bool isOrd1)
{   const char*message;
    const char*filename;
    bool isSet;
    bool isList;
    bool isOrd;
    if(isOrd1==true && isList1==false)
    {
        isSet=false;
        isOrd=true;
        isList=false;
         FILE* file = fopen("./serverUtils/setOrd.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char buffer[1024]={'\0'};
    char** values;
    int counter=0;
    values=(char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
        values[i]=(char*)malloc(sizeof(char)*100);   

    char setname[100];
    int priorities[100]={0};

    while(!feof(file))
    {   
        counter=0;
        fscanf(file,"%s",buffer);
        char copy_buffer[1024];
        strcpy(copy_buffer,buffer);
        if(strcmp(buffer,"")==0)
            break;
        char*word=strtok(copy_buffer,"-");
        strcpy(setname,word);
        char* element=strchr(buffer,'-')+1;
        while(sscanf(element,"%[^,],%d;",values[counter],priorities+counter))
        {
            counter++;
            element = strchr(element, ';');
            if (element != NULL && *(element + 1) != '\0')
            {
                element++; 
            }
            else
            {
                break;   
            }
        
        }
        BST=insertIntoTree(BST,setname,values,false,false,counter,true,priorities);
    }
    for (int i = 0; i < 100; i++) {
            free(values[i]);
    }
    free(values);
    fclose(file);
    }
    else if(isList1==true && isOrd1==false)
    {
        filename="./serverUtils/list.txt";
        isSet=false;
        isOrd=false;
        isList=true;
        message="S-a adaugat in tree lista ";
        char buffer[1000];
    char**values;
    int counter=0;
    values=(char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
        values[i]=(char*)malloc(sizeof(char)*100);                                                  //populeaza arborele cu tot ce se gaseste in list.txt
    FILE*f=fopen(filename,"r");
    if(f==NULL)
    {
        perror("Eroare la deschiderea fisierului aici!");
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
        BST=insertIntoTree(BST,key,values,isList,isSet,counter,isOrd,NULL);
        printf("%s %s ",message,key);
        for(int i=0;i<counter;i++)
            printf("%s,",values[i]);
        printf("\n");

    }
    fclose(f);
    }
    else if(isList1==false && isOrd1==false)
    {
        filename="./serverUtils/set.txt";
        isSet=true;
        isList=false;
        isOrd=false;
        message="S-a adaugat in tree setul ";
        char buffer[1000];
    char**values;
    int counter=0;
    values=(char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
        values[i]=(char*)malloc(sizeof(char)*100);                                                  //populeaza arborele cu tot ce se gaseste in list.txt
    FILE*f=fopen(filename,"r");
    if(f==NULL)
    {
        perror("Eroare la deschiderea fisierului aici!");
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
        BST=insertIntoTree(BST,key,values,isList,isSet,counter,isOrd,NULL);
        printf("%s %s ",message,key);
        for(int i=0;i<counter;i++)
            printf("%s,",values[i]);
        printf("\n");

    }
    fclose(f);
    }
}
void populate_BST()
{
   populateSimple();
   populateListandSet(true,false);
   populateListandSet(false,false);
   populateListandSet(false,true);
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
        fscanf(f,"%s",buffer);
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
        if(i==counter-1)
            fprintf(of,"%s",vector[i]);
        else
        fprintf(of,"%s\n",vector[i]);
    }
    fclose(of);
}
void populate_loginList(listNode **list)
{
    int fd = open("./serverUtils/credentials.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("Problema la deschiderea fisierului credentials!");
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
char*execute_setc(char*buffer)
{
    char key[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(node!=NULL)
        return "Cheia deja exista in arbore!";
    else
    {   char**values=(char**)malloc(sizeof(char*)*100);
        for(int i=0;i<100;i++)
            values[i]=(char*)malloc(sizeof(char)*100);
        BST=insertIntoTree(BST,key,values,false,true,0,false,NULL);
        FILE*f=fopen("./serverUtils/set.txt","a");
        if(empty_file(f))
            fprintf(f,"%s-",key);
        else
            fprintf(f,"\n%s-",key); 
        fclose(f);
        return "SET creat cu succes!";
    }
}
char*execute_lset(char*buffer)
{
    char key[50];
    char value[50];
    int index;
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    word=strtok(NULL," ");
    index=atoi(word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "CHEIA NU EXISTA!";
    if(!node->isList)
        return "CHEIA DATA NU CORESPUNDE UNUI SET!";
    if(index>node->numberOfElements+1)
        return "INDEXUL ESTE INVALID!";
    node->numberOfElements++;
    for(int i=node->numberOfElements;i>index;i--)
        strcpy(node->values[i],node->values[i-1]);
    strcpy(node->values[index],value);
    updateFile(key,TYPE_LIST);
    return "S-a realizat cu succes introducerea!";

}
char*execute_sinter(char*buffer)
{
    char key1[50];
    char key2[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key1,word);
    word=strtok(NULL," ");
    strcpy(key2,word);
    searchTree* nod1=findElementByKey(BST,key1);
    if(nod1==NULL)
        return "CHEIA DATA CA PRIM PARAMETRU NU EXISTA!";
    if(!nod1->isSet)
        return "CHEIA DATA CA PRIM PARAMETRU NU CORESPUNDE UNUI SET!";
    searchTree*nod2=findElementByKey(BST,key2);
    if(nod2==NULL)
        return "CHEIA DATA CA PARAMETRU SECUND NU EXISTA!";
    if(!nod2->isSet)
        return "CHEIA DATA CA PARAMETRU SECUND NU CORESPUNDE UNUI SET!";
    char*bufferToReturn=(char*)malloc(sizeof(char)*150);
    bool find=false;
    strcpy(bufferToReturn,"<");
    for(int i=0;i<nod1->numberOfElements;i++)
        for(int j=0;j<nod2->numberOfElements;j++)
            if(strcmp(nod1->values[i],nod2->values[j])==0)
              { 
                find=true;
                strcat(bufferToReturn,nod1->values[i]);
                strcat(bufferToReturn,",");
              }
        strcat(bufferToReturn,">");
        if(find)
        {
            bufferToReturn[strlen(bufferToReturn)-2]='>';
            bufferToReturn[strlen(bufferToReturn)-1]='\0';
        }
        else return "SET VID";
        return bufferToReturn;

}
char*execute_sunion(char*buffer)
{
    char key1[50];
    char key2[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key1,word);
    word=strtok(NULL," ");
    strcpy(key2,word);
    searchTree* node1=findElementByKey(BST,key1);
    searchTree* node2=findElementByKey(BST,key2);
    if(node1==NULL)
        return "NU EXISTA O INTRARE PENTRU PRIMA CHEIE!";
    if(!node1->isSet)
        return "NU EXISTA O CORESPONDENTA INTRE VREUN SET SI PRIMA CHEIE!";
    if(node2==NULL)
        return "NU EXISTA O INTRARE PENTRU A DOUA CHEIE!";
    if(!node2->isSet)
        return "NU EXISTA O CORESPONDENTA INTRE VREUN SET SI A DOUA CHEIE!";
    char**result=(char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
        result[i]=(char*)malloc(sizeof(char)*50);
    int counter=0;
    bool ok=1;
    for(int i=0;i<node1->numberOfElements;i++)
        strcpy(result[counter++],node1->values[i]);
    for(int j=0;j<node2->numberOfElements;j++)
    {   ok=1;
        for(int k=0;k<node1->numberOfElements;k++)
            if(strcmp(result[k],node2->values[j])==0)
                {ok=0;
                break;}
        if(ok)
            strcpy(result[counter++],node2->values[j]);

    }
    char*bufferToReturn=(char*)malloc(sizeof(char)*250);
    strcpy(bufferToReturn,"<");
    for(int i=0;i<counter;i++)
    {
        strcat(bufferToReturn,result[i]);
        strcat(bufferToReturn,",");
    }
    strcat(bufferToReturn,">");
    bufferToReturn[strlen(bufferToReturn)-2]='>';
    bufferToReturn[strlen(bufferToReturn)-1]='\0';
    return bufferToReturn;
}

char* execute_getset(char*buffer)
{
    char key[50];
    char value[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "CHEIA NU EXISTA!";
    if(node->isSet || node->isList)
        return "NU ESTE O CHEIE SIMPLA!";
    strcpy(node->values[0],value);
    updateFile(key,TYPE_SIMPLE);
    return "VALOARE SCHIMBATA CU SUCCES!";

}
char*execute_scard(char*buffer)
{
    char key[50];
    char*buff=(char*)malloc(sizeof(char)*5);
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NU EXISTA CORESPONDENTA PENTRU CHEIA DATA!";
    if(!node->isSet)
        return "CHEIA NU CORESPUNDE CU UN SET!";
    snprintf(buff,5,"%d",node->numberOfElements);
    return buff;
}
char*execute_lrange(char*buffer)
{
    char key[150];
    int index1;
    int index2;
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    index1=atoi(word);
    word=strtok(NULL," ");
    index2=atoi(word);
     searchTree*node=findElementByKey(BST,key);
     if(node->isList)
     {
    if(index1>node->numberOfElements || index2>node->numberOfElements || index1<0 || index2<0)
        return "INDEX OUT OF RANGE";
    else
    if(index1>index2)
        return "INDEX1 SHOULD BE BIGGER THAN INDEX2";
    else
    {
        char* bufferToReturn=(char*)malloc(sizeof(char)*200);
        strcpy(bufferToReturn,"[");
        for(int i=index1;i<=index2;i++)
        { strcat(bufferToReturn,node->values[i]);
            if(i!=index2)
                strcat(bufferToReturn,",");
        }
        strcat(bufferToReturn,"]");
        return bufferToReturn;
    }
     }
     else return "NOT A LIST!";

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
        if(!node->isList && !node->isSet)
           strcpy(bufferToReturn,node->values[0]);
        if(node->isList && !node->isSet)
        {   strcpy(bufferToReturn,"list:");
            strcat(bufferToReturn,"[");
            searchTree*node=findElementByKey(BST,word);
            for(int i=0;i<node->numberOfElements;i++)
            {
                strcat(bufferToReturn,node->values[i]);
                if(i!=node->numberOfElements-1)
                    strcat(bufferToReturn,",");
            }
            strcat(bufferToReturn,"]");
        }
        bufferToReturn[strlen(bufferToReturn)]='\0';
        return bufferToReturn;
    }
}

void updateFile(char*key,int type)
{
    const char*filename;
    if(type==TYPE_LIST)
        filename="./serverUtils/list.txt";
    else if(type==TYPE_SET) 
        filename="./serverUtils/set.txt";
    else if(type==TYPE_SIMPLE)
        filename="./serverUtils/simple.txt";
    char**vectorLinii=(char**)malloc(sizeof(char*)*100);    
    int counter=0;
    for(int i=0;i<100;i++)
        vectorLinii[i]=(char*)malloc(sizeof(char)*200);
    FILE*f=fopen(filename,"r");
    char buffer[250];
    char content[250];
    char copie[250];

    while(!feof(f))
    {
        fscanf(f,"%s",buffer);
        strcpy(copie,buffer);
        char*key_f=strtok(copie,"-");
        if(strcmp(key_f,key)==0)
        {   char content[250];
            searchTree*node=findElementByKey(BST,key);
            strcpy(content,key);
            strcat(content,"-");
            for(int i=0;i<node->numberOfElements;i++)
            {
                strcat(content,node->values[i]);
                if(type!=TYPE_SIMPLE)
                    strcat(content,",");

            }
            strcpy(vectorLinii[counter],content);
            counter++;
        }
        else
        {
        strcpy(vectorLinii[counter],buffer);
        counter++;
        }
    }
    fclose(f);
    FILE*of=fopen(filename,"w");
    for(int i=0;i<counter;i++)
    {
        if(i==counter-1)
            fprintf(of,"%s",vectorLinii[i]);
        else fprintf(of,"%s\n",vectorLinii[i]);
        fflush(of);
    }
    fclose(of);
    

}
bool firstLine=0;
void updateSimple(searchTree*node,FILE*f)
{
    if(node==NULL || node->isList || node->isSet)
        return;
    if(!firstLine)
    {
        fprintf(f,"%s-%s",node->key,node->values[0]);
        firstLine=true;
    }
    else 
    {
        if(empty_file(f))
            fprintf(f,"%s-%s",node->key,node->values[0]);
    else
        fprintf(f,"\n%s-%s",node->key,node->values[0]);
    }
    fflush(f);

    updateSimple(node->leftNode,f);
    updateSimple(node->rightNode,f);
    
}
char* execute_changev(char*buffer)
{
    char oldkey[50];
    char newkey[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(oldkey,word);
    word=strtok(NULL," ");
    strcpy(newkey,word);
    searchTree*node=findElementByKey(BST,oldkey);
    if(node==NULL)
        return "CHEIA DATA NU EXISTA!";
    char**values=node->values;
    bool isList=node->isList;
    bool isSet=node->isSet;
    bool isOrd=node->isOrdonated;
    int numberOfElements=node->numberOfElements;
    insertIntoTree(BST,newkey,values,isList,isSet,numberOfElements,isOrd,NULL);
    deleteNode(BST,oldkey);
    FILE*f=fopen("./serverUtils/simple.txt","w");
    updateSimple(BST,f);
    fclose(f);


    //TODO functie de update aici pentru a inlocui cheia in fisier

    return "OK";



}
char*execute_lpop(char*buffer)
{   
    char key[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(!node->isList)
        return "NO LIST FOUND";
    else
    {
        if(node->numberOfElements==0)
            return "NU EXISTA ELEMENTE IN LISTA!";
        else
        {   
            char*bufferToReturn=(char*)malloc(sizeof(char)*100);
            strcpy(bufferToReturn,node->values[0]);
            for(int i=0;i<node->numberOfElements-1;i++)
                strcpy(node->values[i],node->values[i+1]);
            node->numberOfElements--;
            updateFile(key,TYPE_LIST);
            return bufferToReturn;

        }
    }
}
void keys(searchTree*node,char**buffer);
char*execute_keys()
{
    char*buffer=(char*)malloc(sizeof(char)*500);
    keys(BST,&buffer);
    return buffer;
}
void keys(searchTree*node,char**buffer)
{
    if(node==NULL)
        return;
    
    strcat(*buffer,node->key);
    if(node->isList)
        strcat(*buffer,"-lista");
    else if(node->isSet)
            strcat(*buffer,"-set");
    else if(node->isOrdonated)
            strcat(*buffer,"-set ordonat");
        else strcat(*buffer,"-simplu");
    strcat(*buffer,"\n");

    keys(node->leftNode,buffer);
    keys(node->rightNode,buffer);
    
}
char* execute_rpop(char*buffer)
{
    char key[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(!node->isList)
        return "NO LIST FOUND";
    else
    {
        if(node->numberOfElements==0)
            return "NU EXISTA ELEMENTE IN LISTA!";
        else
        {
            char*bufferToReturn=(char*)malloc(sizeof(char)*100);
            strcpy(bufferToReturn,node->values[node->numberOfElements-1]);
            node->numberOfElements--;
            updateFile(key,TYPE_LIST);
            return bufferToReturn;
        }
    }
}
char* execute_getrange(char*buffer)
{
    char key[50];
    char*bufferToReturn=(char*)malloc(sizeof(char)*70);
    int start;
    int end;
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    start=atoi(word);
    word=strtok(NULL," ");
    end=atoi(word);
    if(start>end)
        return "INDECSII NU SUNT VALIZI!!";
    int counter=0;
    searchTree* node=findElementByKey(BST,key);
    if(end>strlen(node->values[0]))
        return "INDEXUL DE FINAL ESTE MAI MARE DEECAT SIRRUL!";
    if(node==NULL)
        return "CHEIA NU EXISTA!";
    if(node->isList || node->isSet)
        return "CHEIA ESTE PENTRU SETURI SAU LISTE!";
    for(int i=start;i<end;i++)
        bufferToReturn[counter++]=node->values[0][i];
        bufferToReturn[counter]='\0';
    return bufferToReturn;



}
char* execute_lpush(char*buffer)
{
    char key[100];
    char value[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NOTOK";
    word=strtok(NULL," ");
    strcpy(value,word);
    addValueToLeft(BST,key,value);
    updateFile(key,TYPE_LIST);
    return "OK";
}
char* execute_rpush(char*buffer)
{
    char key[100];
    char value[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NOTOK";
    word=strtok(NULL," ");
    strcpy(value,word);
    addValueToRight(BST,key,value);
    updateFile(key,TYPE_LIST);
    return "OK";

}
char *execute_autentificare(char *buffer)
{
    char *word = strtok(buffer, " ");
    char *username = (char *)malloc(sizeof(char) * 30);
    char *password = (char *)malloc(sizeof(char) * 30);
    char *bufferToReturn = (char *)malloc(sizeof(char) * 10);
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
char* execute_sismember(char*buffer)
{
    char key[50];
    char value[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NU EXISTA O CORESPONDENTA PENTRU CHEIA DATA!";
    if(!node->isSet)
        return "CHEIA DATA NU CORESPUNDE CU UN SET!";
    for(int i=0;i<node->numberOfElements;i++)
        if(strcmp(node->values[i],value)==0)
            return "VALOAREA DATA EXISTA IN SET!";
    return "VALOAREA DATA NU EXISTA IN SET!";
    
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
        if(empty_file(f))
            fprintf(f,"%s-",key);
        else
            fprintf(f,"\n%s-",key);
        BST=insertIntoTree(BST,key,values,true,false,0,false,NULL);
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
            deleteFromFile("./serverUtils/set.txt",key);
            return "OK";
        }

}
bool findValue(char*key,char*value)
{
    searchTree*nod=findElementByKey(BST,key);
    if(!nod->isSet)
        return true;
    else
    {   char hello[250];
        for(int i=0;i<nod->numberOfElements;i++)
            if(strcmp(value,nod->values[i])==0)
                return true;
        return false;
    }
}
char* execute_sadd(char*buffer)
{
    char key[50];
    char value[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    if(!findValue(key,word))
    {searchTree*nod= addValueToSet(BST,key,value);
    updateFile(key,TYPE_SET);
    if(nod==NULL)
        return "Nu exista un set cu aceasta cheie...";
    else 
        return "Valoare adaugata cu succes...";
    }
    else return "Eroare.Elementul se afla deja in set...";
}
char* execute_srem(char*buffer)
{
    char key[50];
    char value[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    searchTree* nod=findElementByKey(BST,key);
    if(nod==NULL)
        return "NU EXISTA ACEASTA CHEIE";
    else
    {
        if(!nod->isSet)
            return "CHEIA DATA NU CORESPUNDE UNUI SET";
        int poz=-1;
        for(int i=0;i<nod->numberOfElements;i++)
            if(strcmp(nod->values[i],value)==0)
                poz=i;
        if(poz==-1)
            return "ELEMENTUL NU EXISTA IN SETUL RESPECTIV";
        for(int i=poz;i<nod->numberOfElements-1;i++)
            strcpy(nod->values[i],nod->values[i+1]);
        nod->numberOfElements--;
        updateFile(key,TYPE_SET);
        return "ELEMENTUL A FOST STERS!";
    }
}
char* execute_smembers(char*buffer)
{
    char key[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NU EXISTA ACEASTA CHEIE";
    else
    {
        if(!node->isSet)
            return "CHEIA DATA NU CORESPUNDE UNUI SET";
        char* bufferToReturn=(char*)malloc(sizeof(char)*250);
        strcpy(bufferToReturn,"(");
        for(int i=0;i<node->numberOfElements;i++)
        {
            strcat(bufferToReturn,node->values[i]);
            if(i!=node->numberOfElements-1)
                strcat(bufferToReturn,",");
        }
        strcat(bufferToReturn,")");
        return bufferToReturn;
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
        BST=insertIntoTree(BST,key,value,false,false,1,false,NULL);
        FILE*f=fopen("./serverUtils/simple.txt","a");
        if(empty_file(f))
            fprintf(f,"%s-%s",key,value[0]);
        else
            fprintf(f,"\n%s-%s",key,value[0]);
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
char* execute_strlen(char*buffer)
{
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    searchTree*node=findElementByKey(BST,word);
    if(node==NULL)
        return "NU EXISTA O INTRARE PENTRU CHEIA DATA!";
    if(node->isList || node->isSet)
        return "CHEIA DATA ESTE PENTRU UN SET SAU O LISTA!"; 
    char* bufferToReturn=malloc(sizeof(char)*10);
    int n=strlen(node->values[0]);
    snprintf(bufferToReturn,10,"%d",n);
    return bufferToReturn;
}

char* execute_lrem(char*buffer)
{
    char key[50];
    int index;
    char value[50];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    index=atoi(word);
    searchTree*node=findElementByKey(BST,key);
    if(node==NULL)
        return "NU EXISTA O INTRARE PENTRU ACEASTA CHEIE!";
    if(!node->isList)
        return "CHEIA DATA NU ESTE PENTRU O LISTA!";
    if(index>node->numberOfElements || index<0)
        return "INDEXUL ESTE PREA MARE SAU NEGATIV!";
    node->numberOfElements--;
    for(int i=index;i<node->numberOfElements;i++)
        strcpy(node->values[i],node->values[i+1]);
    updateFile(key,TYPE_LIST);
    return "VALOARE ELIMINATA CU SUCCES!";
}

char *execute_command(char *buffer)
{   
    char *copie=(char*)malloc(sizeof(char)*70);
    strcpy(copie, buffer);
    char *protocol = strtok(copie, " ");
    char *messageToSend = malloc(sizeof(char) * 2500);
    strcpy(messageToSend,"");
    if (strcmp(protocol,"LOGIN") == 0)
        strcpy(messageToSend, execute_login(buffer));
    else
    if (strcmp(protocol,"AUTH") == 0)
        strcpy(messageToSend, execute_autentificare(buffer));
    else
    if(strcmp(protocol,"GET")==0)
        strcpy(messageToSend,execute_get(buffer));
    if(strcmp(protocol,"SET")==0)
        strcpy(messageToSend,execute_set(buffer));
    if(strcmp(protocol,"DEL")==0)
        strcpy(messageToSend,execute_del(buffer));
    if(strcmp(protocol,"LIST")==0)
        strcpy(messageToSend,execute_listCreation(buffer));
    if(strcmp(protocol,"RPUSH")==0)
        strcpy(messageToSend,execute_rpush(buffer));
    if(strcmp(protocol,"LPUSH")==0)
        strcpy(messageToSend,execute_lpush(buffer));
    if(strcmp(protocol,"LRANGE")==0)
        strcpy(messageToSend,execute_lrange(buffer));
    if(strcmp(protocol,"RPOP")==0)
        strcpy(messageToSend,execute_rpop(buffer));
    if(strcmp(protocol,"LPOP")==0)
        strcpy(messageToSend,execute_lpop(buffer));
    if(strcmp(protocol,"SETC")==0)
        strcpy(messageToSend,execute_setc(buffer));
    if(strcmp(protocol,"SADD")==0)
        strcpy(messageToSend,execute_sadd(buffer));
    if(strcmp(protocol,"SREM")==0)
        strcpy(messageToSend,execute_srem(buffer));
    if(strcmp(protocol,"SMEMBERS")==0)
        strcpy(messageToSend,execute_smembers(buffer));
    if(strcmp(protocol,"SISMEMBER")==0)
        strcpy(messageToSend,execute_sismember(buffer));
    if(strcmp(protocol,"SCARD")==0)
        strcpy(messageToSend,execute_scard(buffer));
    if(strcmp(protocol,"SINTER")==0)
        strcpy(messageToSend,execute_sinter(buffer));
    if(strcmp(protocol,"SUNION")==0)
        strcpy(messageToSend,execute_sunion(buffer));
    if(strcmp(protocol,"LOGOUT")==0)
        strcpy(messageToSend,execute_logout());
    if(strcmp(protocol,"FINISH")==0)
        exit(1);
    if(strcmp(protocol,"STRLEN")==0)
        strcpy(messageToSend,execute_strlen(buffer));
    if(strcmp(protocol,"GETRANGE")==0)
        strcpy(messageToSend,execute_getrange(buffer));
    if(strcmp(protocol,"LSET")==0)
        strcpy(messageToSend,execute_lset(buffer));
    if(strcmp(protocol,"LREM")==0)
        strcpy(messageToSend,execute_lrem(buffer));
    if(strcmp(protocol,"GETSET")==0)
        strcpy(messageToSend,execute_getset(buffer));
    if(strcmp(protocol,"KEYS")==0)
        strcpy(messageToSend,execute_keys());
    if(strcmp(protocol,"CHANGEV")==0)
        strcpy(messageToSend,execute_changev(buffer));
    if(strcmp(protocol,"ZADD")==0)
        strcpy(messageToSend,execute_zadd(buffer));
    if(strcmp(protocol,"ZCARD")==0)
        strcpy(messageToSend,execute_zcard(buffer));
    if(strcmp(protocol,"ZSCORE")==0)
        strcpy(messageToSend,execute_zscore(buffer));
    if(strcmp(protocol,"ZCOUNT")==0)
        strcpy(messageToSend,execute_zcount(buffer));
    if(strcmp(protocol,"ZINCRBY")==0)
        strcpy(messageToSend,execute_zincrby(buffer));
    if(strcmp(protocol,"ZRANK")==0)
        strcpy(messageToSend,execute_zrank(buffer));
    if(strcmp(protocol,"ZRANGE")==0)
        strcpy(messageToSend,execute_zrange(buffer));
    if(strcmp(protocol,"ZREM")==0)
        strcpy(messageToSend,execute_zrem(buffer));
    if(strcmp(protocol,"ZUNION")==0)
        strcpy(messageToSend,execute_zunion(buffer));
    if(strcmp(protocol,"ZUNIONSTORE")==0)
        strcpy(messageToSend,execute_zunionstore(buffer));
    if(strcmp(protocol,"HELP")==0)
        strcpy(messageToSend,execute_help(buffer));
    if(strcmp(protocol,"ZDEL")==0)
        strcpy(messageToSend,execute_zdel(buffer));
    return messageToSend;
}

char *execute_login(char *buffer)
{
    char *word = strtok(buffer, " ");
    char *username = (char *)malloc(sizeof(char) * 30);
    char *password = (char *)malloc(sizeof(char) * 30);
    char *bufferToReturn = (char *)malloc(sizeof(char) * 10);
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
        { FILE*f=fopen("./serverUtils/online","r");
        strcpy(bufferToReturn,"DA");
        char username_F[100];
        while(!feof(f))
        {
            fscanf(f,"%s",username_F);
            if(strcmp(username_F,username)==0)
            {
                strcpy(bufferToReturn,"ONLINE");
                break;
            } 


        }
        fclose(f);
        if(strcmp(bufferToReturn,"ONLINE")!=0)
            {
            add_logger("Utilizatorul s-a autentificat cu succes!");
            FILE*of=fopen("./serverUtils/online","a");
            fprintf(of,"%s\n",username);
            strcpy(user,username);
            fclose(of);
            }
        }
    else
        strcpy(bufferToReturn, "NU");
    bufferToReturn[strlen(bufferToReturn)] = '\0';
    return bufferToReturn;
}
void runApp(int clientSocket)
{
    signal(SIGINT,handle_sigint);
    char buffer[BUFFER_SIZE];
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int messageReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (messageReceived > 0)
        {
            // Aici functie execute_command care returneaza un char* de trimis,In ea vor fi functii pentru fiecare comanda(login,autentif,set,get,...);
            char* message=(char*)malloc(sizeof(char)*100);
            if (strcmp(buffer, "LOGOUT") == 0)
            {
            strcpy(message, execute_command(buffer));
            char* buffer=(char*)malloc(sizeof(char)*50);
            strcat(buffer,user);
            strcat(buffer," s-a delogat cu succes!");
            add_logger(buffer);
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
void *run(void *arg) {
        client_info *client = (client_info *)arg;
    char buffer[BUFFER_SIZE];

 
    runApp(client->clientSocket);

    // Închideți socket-ul clientului și eliberați memoria
    close(client->clientSocket);
    free(client);

    pthread_exit(NULL);
}
int establish_connection()
{
    signal(SIGINT,handle_sigint);
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
    {   client_info*newClient=(client_info*)malloc(sizeof(client_info));
        newClient->clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (clientSocket == -1)
        {
            perror("Eroare la acceptarea conexiunii");
            exit(1);
        }

        printf("Clientul %d s-a conectat.\n",newClient->clientSocket);
        pthread_t client_thread;
        pthread_create(&client_thread,NULL,run,(void*)newClient);
    }

    return 0;
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
    printf("Clientul s-a deconectat!");
    add_logger("Utilizatorul s-a delogat!");
    FILE*f=fopen("./serverUtils/online","r");
    char buff[50];
    char users[10][50];
    int counter=0;
    while(!feof(f))
    {
        fscanf(f,"%s",buff);
        if(strcmp(buff,user)!=0)
            strcpy(users[counter++],buff);
    }
    fclose(f);
    FILE*of=fopen("./serverUtils/online","w");
    for(int i=0;i<counter;i++)
        fprintf(of,"%s\n",users[i]);
    fclose(of);
    return aux;
}



void saveSetToFile(FILE* file, searchTree* root) {
    if (root != NULL) {
        if(root->isOrdonated==true)
        {
        fprintf(file, "%s-", root->key);
        for (int i = 0; i < root->numberOfElements; i++) {
            fprintf(file, "%s,%d;", root->values[i], root->priorites[i]);
        }
        fprintf(file, "\n");
        }
        saveSetToFile(file, root->leftNode);
        saveSetToFile(file, root->rightNode);
    }
}


void removeLastLine(const char* filename) {
    FILE* file = fopen(filename, "r+");
    if (file == NULL) {
        perror("Error opening file for reading and writing");
        return;
    }

    // Mergem la sfârșitul fișierului
    if (fseek(file, -1, SEEK_END) != 0) {
        perror("Error seeking to the end of the file");
        fclose(file);
        return;
    }

    // Cautăm înapoi până la găsirea caracterului '\n' sau la începutul fișierului
    while (ftell(file) > 0) {
        char ch = fgetc(file);
        if (ch == '\n') {
            break;
        }
        fseek(file, -2, SEEK_CUR);  // Deplasare înapoi cu 2 caractere
    }

    // Salvăm poziția curentă (începutul ultimei linii)
    int currentPos = ftell(file);

    // Setăm dimensiunea fișierului la poziția curentă
    if (truncate("./serverUtils/setOrd.txt", currentPos-1) != 0) {
        perror("Eroare la modificarea dimensiunii fisierului");
        fclose(file);
        exit(-1);
    }

    fclose(file);
}

// Funcție pentru a salva arborele în fișier
void saveTreeToFile(FILE* file, searchTree* root) {
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    saveSetToFile(file, root);
    fclose(file);
    removeLastLine("./serverUtils/setOrd.txt");
}



bool searchSetByName(searchTree* root, char* setName) {
    // Parcurge arborele pentru a căuta setul cu numele dat
    while (root != NULL) {
        int compareResult = strcmp(setName,root->key);
      
        if (compareResult == 0) {
            // Numele setului a fost găsit în arbore
            return true;
        } else if (compare(setName,root->key)) {
            // Numele setului este mai mic, continuă căutarea în subarborele stâng
            root = root->rightNode;
        } else  if(compare(root->key,setName)){
            // Numele setului este mai mare, continuă căutarea în subarborele drept
            root = root->leftNode;
        }
    }
    return false;
}


char* execute_zadd(char* buffer)
{
    char* msg_logger=(char*)malloc(sizeof(char)*100);
    strcpy(msg_logger,user);
    strcat(msg_logger," a folost comanda ");
    strcat(msg_logger,buffer);
    char* ptr;
    char **setname=(char**)malloc(sizeof(char*)*2);
    
    char** values=(char**)malloc(sizeof(char*)*2);
    values[0]=(char*)malloc(sizeof(char)*40);
    int priorites[1];
    ptr=strtok(buffer," ");
    printf("\n\n%s\n",ptr);

    ptr=strtok(NULL," ");
    setname[0]=(char*)malloc(sizeof(char)*(strlen(ptr)+1));
    strcpy(setname[0],ptr);
    setname[0][strlen(setname[0])]='\0';
    printf("Codurile ASCII ale caracterelor din setname[0]: ");
for (int i = 0; i < strlen(setname[0]); i++) {
    printf("%d ", setname[0][i]);
}
printf("\n");


    int compare=strcasecmp("cheie1",setname[0]);
    printf("\n\n%s %d\n",setname[0],compare);

    ptr=strtok(NULL," ");
    strcpy(values[0],ptr);
    printf("\n\n%s\n",values[0]);


    ptr=strtok(NULL," ");
    priorites[0]=atoi(ptr);
    printf("\n\n%d\n",priorites[0]);


    if(searchSetByName(BST,setname[0])==false)
    {   
        FILE* file = fopen("./serverUtils/setOrd.txt", "a");
        
    if (file == NULL) {
        perror("Error opening file");
        exit(-1);
    }

    fseek(file, 0, SEEK_END);  // Muta indicatorul de poziție la sfârșitul fișierului
    long size = ftell(file);

    if (size == 0) {
        fprintf(file,"%s-%s,%d;",setname[0],values[0],priorites[0]);
        fclose(file);
        BST = insertIntoTree(BST, setname[0], values, false,false,1,true, priorites);
    } else {
        fprintf(file,"\n%s-%s,%d;",setname[0],values[0],priorites[0]);
        fclose(file);
        BST = insertIntoTree(BST, setname[0], values, false,false,1,true, priorites);}
    }
    else
    {
    BST = insertIntoTree(BST, setname[0], values, false,false,1,true, priorites);

       // Deschide fișierul în modul de citire
    FILE* file = fopen("./serverUtils/setOrd.txt", "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Salvează întregul arbore în fișier
    saveTreeToFile(file, BST);
}
    char* aux="OK";
    strcat(msg_logger," cu SUCCES!");
    add_logger(msg_logger);
    return aux;
}



char* execute_zcard(char* buffer)
{
    char* msg_logger=(char*)malloc(sizeof(char)*100);
    strcpy(msg_logger,user);
    strcat(msg_logger," a folost comanda ");
    strcat(msg_logger,buffer);

    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL,"\0\n ");

    searchTree* node=findElementByKey(BST,ptr);
    char* messages=(char*)malloc(sizeof(char)*50);
    if(node==NULL)
        {strcpy(messages,"Setul ordonat nu exista!");
        strcat(msg_logger," cu FAILED!");
        add_logger(msg_logger);
        return messages;
        }
    else if(node->isOrdonated!=true)
    {
        strcat(msg_logger," cu FAILED!");
        add_logger(msg_logger);
        return "Set-ul nu este un set ordonat!";
    }
    else
    {
            strcat(messages,"Setul ordonat ");
            strcat(messages,node->key);
            strcat(messages," are ");
            char buffer[10];
            snprintf(buffer,sizeof(buffer),"%d",node->numberOfElements);
            strcat(messages,buffer);
            strcat(messages," elemente!");
    }
    strcat(msg_logger," cu SUCCES!");
    add_logger(msg_logger);
    return messages;
}



char* execute_zscore(char* buffer)
{
    char* msg_logger=(char*)malloc(sizeof(char)*100);
    strcpy(msg_logger,user);
    strcat(msg_logger," a folost comanda ");
    strcat(msg_logger,buffer);

    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    char* messages=(char*)malloc(sizeof(char)*60);
    char score[10];
    searchTree* node=findElementByKey(BST,ptr);
    int ok=0;
    if(node==NULL)
        {
            strcpy(messages,"Setul introdus nu exista!");
            strcat(msg_logger," cu FAILED!");
            add_logger(msg_logger);
            return messages;
        }
    else if(node->isOrdonated!=true)
    {
        strcat(msg_logger," cu FAILED!");
        add_logger(msg_logger);
        return "Set-ul nu este un set ordonat!";
    }
    else
    {
        ptr=strtok(NULL,"\0\n");
        for(int i=0;i<node->numberOfElements;i++)
        {
            if(strcmp(ptr,node->values[i])==0)
            {
                printf("%d",node->priorites[i]);
                snprintf(score,sizeof(score),"%d",node->priorites[i]);
                ok=1;
            }
        }
        if(ok==1)
        {
            strcpy(messages,"Membrul setului introdus are scorul de ");
           
            strcat(messages,score);
        }
        else
        {
            strcpy(messages,"Membrul din setul introdus nu exista!");
        }

    }
    strcat(msg_logger," cu SUCCES!");
    add_logger(msg_logger);
    return messages;
}


char* execute_zcount(char* buffer)
{
    char* msg_logger=(char*)malloc(sizeof(char)*100);
    strcpy(msg_logger,user);
    strcat(msg_logger," a folost comanda ");
    strcat(msg_logger,buffer);

    char* messages=(char*)malloc(sizeof(char)*50);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node=findElementByKey(BST,ptr);
    if(node ==NULL)
    {
            strcpy(messages,"Setul introdus nu exista!");
            strcat(msg_logger," cu FAILED!");
            add_logger(msg_logger);
            return messages;
    }else if(node->isOrdonated!=true)
    {
        strcat(msg_logger," cu FAILED!");
        add_logger(msg_logger);
        return "Set-ul nu este un set ordonat!";
    }
    ptr=strtok(NULL," ");
    int min=atoi(ptr);
    ptr=strtok(NULL,"\0\n");
    int max=atoi(ptr);
    int counter=0;
    for(int i=0;i<node->numberOfElements;i++)
    {
        if(node->priorites[i]>=min && node->priorites[i]<=max)
            counter++;
    }
    snprintf(messages,sizeof(messages),"%d",counter);
    strcat(msg_logger," cu SUCCES!");
    add_logger(msg_logger);
    return messages;
}


searchTree* modify_tree(searchTree* node, searchTree* modify_node)
{
    if (strcmp(node->key,modify_node->key)==0 && node->isList==modify_node->isList && node->isOrdonated==modify_node->isOrdonated && node->isSet==modify_node->isSet)
    {
        node=modify_node;
        swapSet(node);  
        return node;
    }
    

    if (compare(modify_node->key, node->key))
        node->rightNode = modify_tree(node->rightNode, modify_node);
    else if (compare(node->key, modify_node->key))
        node->leftNode = modify_tree(node->leftNode, modify_node);

    return node;
}




char* execute_zincrby(char* buffer)
{
    char* copy_buffer=strdup(buffer);
    char* messages=(char*)malloc(sizeof(char)*50);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node=findElementByKey(BST,ptr);
    if(node ==NULL)
    {
        char* a=execute_zadd(copy_buffer);
        strcpy(messages,"Elementul a fost creat impreuna cu setul!");
        return messages;
    } else if(node->isOrdonated!=true)
    {
        return "Set-ul nu este un set ordonat!";
    }
    else
    {
        ptr=strtok(NULL," ");
        int ok=0;
        for(int i=0;i<node->numberOfElements;i++)
        {
            if(strcmp(node->values[i],ptr)==0)
            {
                ptr=strtok(NULL,"\0\n");
                int aux=atoi(ptr);
                node->priorites[i]+=aux;
                ok=1;
            }
        }
        if(ok==0)
        {
            char* a=execute_zadd(copy_buffer);
            strcpy(messages,"Elemntul a fost creat!");
            return messages;
        }
        BST=modify_tree(BST,node);
        FILE* file = fopen("./serverUtils/setOrd.txt", "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Salvează întregul arbore în fișier
    saveTreeToFile(file, BST);
    strcpy(messages,"ELemntul din set a fost incremneta cu succes!");
    }
    return messages;
}


char* execute_zrank(char* buffer)
{
    char* messages=(char*)malloc(sizeof(char)*50);
    strcpy(messages,"ZRANK!");
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node=findElementByKey(BST,ptr);
    if(node==NULL)
    {
        strcpy(messages,"Setul introdus nu exista!");
        return messages;
    }else if(node->isOrdonated!=true)
    {
        return "Set-ul nu este un set ordonat!";
    }
    ptr=strtok(NULL,"\0\n");

    for(int i=0;i<node->numberOfElements;i++)
    {
        if(strcmp(node->values[i],ptr)==0)
        {
            char buffer[5];
            snprintf(buffer,sizeof(buffer),"%d",i);
            strcpy(messages,buffer);
            return messages;
        }
    }
    strcpy(messages,"Elemntul introdus nu exista in set!");
    return messages;
}



char * execute_zrange(char* buffer)
{
    char* message=(char*)malloc(sizeof(char)*100);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node=findElementByKey(BST,ptr);
    if(node==NULL)
    {
        strcpy(message,"Setul introdus nu exista!");
        return message;
    }else if(node->isOrdonated!=true)
    {
        return "Set-ul nu este un set ordonat!";
    }
    ptr=strtok(NULL," ");
    int min=atoi(ptr);
    if(min>node->numberOfElements)
    {
            strcpy(message,"Range-ul dat este incorect!(RANGE START > nr de elemente)");
            return message;
    }
   
    ptr=strtok(NULL,"\0\n");
    int max=atoi(ptr);
    if(max>node->numberOfElements)
    {
        strcpy(message,"Range-ul STOP este mai mare decat nr de elemente: ");
        max=node->numberOfElements-1;
    }
    for(int i=min;i<=max;i++)
    {
        strcat(message,node->values[i]);
        strcat(message," ");
    }
    //TODO mai multe if
    return message;
}


char *execute_zrem(char* buffer)
{
    char* message=(char*)malloc(sizeof(char)*50);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node=findElementByKey(BST,ptr);
    if(node==NULL)
    {
        strcpy(message,"Setul introdus nu exista!");
        return message;
    } else if(node->isOrdonated!=true)
    {
        return "Set-ul nu este un set ordonat!";
    }
    ptr=strtok(NULL,"\0\n");
    int ok=-1;
    for(int i=0;i<node->numberOfElements;i++)
    {
        if(strcmp(ptr,node->values[i])==0)
            ok=i;
    }
    if(ok==-1)
    {
        strcpy(message,"Elementul nu exista in set!");
        return message;
    }
    if(ok==node->numberOfElements-1)  //cazul in care ultimul element trebuie eliminat!
    {
        node->values[ok]="\0";
        node->priorites[ok]=0;
        node->numberOfElements--;
        BST=modify_tree(BST,node);
    }
    else
    {
        char* aux;
        strcpy(aux,node->values[node->numberOfElements-1]);
        strcpy(node->values[node->numberOfElements-1],node->values[ok]);
        strcpy(node->values[ok],aux);
        node->values[node->numberOfElements-1]="\0";
        int aux2;
        aux2=node->priorites[node->numberOfElements-1];
        node->priorites[node->numberOfElements-1]=node->priorites[ok];
        node->priorites[ok]=aux2;
        node->priorites[node->numberOfElements-1]=0;
        node->numberOfElements--;

        BST=modify_tree(BST,node);
    }
     FILE* file = fopen("./serverUtils/setOrd.txt", "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Salvează întregul arbore în fișier
    saveTreeToFile(file, BST);
    strcpy(message,"Elemntul a fost sters cu succes!");
    return message;
}



char* execute_zunion(char* buffer)
{
    char* message=(char*)malloc(sizeof(char)*100);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    searchTree* node1=findElementByKey(BST,ptr);
    if(node1==NULL)
    {
        strcpy(message,"Set-ul 1 introdus nu exista!");
        return message;
    } else if(node1->isOrdonated!=true)
    {
        return "Set-ul 1 nu este un set ordonat!";
    }
    ptr=strtok(NULL,"\0\n");
    searchTree* node2=findElementByKey(BST,ptr);
    if(node2==NULL)
    {
        strcpy(message,"Set-ul 2 introdus nu exista!");
        return message;
    }else if(node2->isOrdonated!=true)
    {
        return "Set-ul 2 nu este un set ordonat!";
    }
    char** copy_value=(char**)malloc(sizeof(char*)*70);
    int prioritati[70];
    int nr_elem=0;
    for(int i=0;i<70;i++)
        copy_value[i]=(char*)malloc(sizeof(char)*70);
    for(int i=0;i<node1->numberOfElements;i++)
    {
        strcpy(copy_value[i],node1->values[i]);
        prioritati[i]=node1->priorites[i];
        nr_elem++;
    }
    for(int i=0;i<node2->numberOfElements;i++)
    {
        int ok=0;
        for(int j=0;j<nr_elem;j++)
        {
            if(strcmp(node2->values[i],copy_value[j])==0)
             {   
                prioritati[j]+=node2->priorites[i];
                ok=1;
             }
        }
        if(ok==0)
        {
            strcpy(copy_value[nr_elem],node2->values[i]);
            prioritati[nr_elem]=node2->priorites[i];
            nr_elem++;
        }
    }
    int swapped;

    do {
    swapped = 0;
    for (int i = 0; i < nr_elem - 1; i++) {
        if (prioritati[i] > prioritati[i + 1]) {
            int tempPriority = prioritati[i];
            prioritati[i] = prioritati[i + 1];
            prioritati[i + 1] = tempPriority;
            char tempValue[100];
            strcpy(tempValue, copy_value[i]);
            strcpy(copy_value[i],copy_value[i + 1]);
            strcpy(copy_value[i + 1], tempValue);
            swapped = 1;
        }
    }
    } while (swapped);

    for(int i=0;i<nr_elem;i++)
    {
        printf("%s ",copy_value[i]);
        strcat(message,copy_value[i]);
        char buffer[5];
        snprintf(buffer,sizeof(buffer),"%d",prioritati[i]);
        strcat(message,"(");
        strcat(message,buffer);
        strcat(message,") ");
    }
    return message;

}



char* execute_zunionstore(char* buffer)
{
    char* message=(char*)malloc(sizeof(char)*100);
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL," ");
    char* setname;
    strcpy(setname,ptr);
    ptr=strtok(NULL," ");
    searchTree* node1=findElementByKey(BST,ptr);
    if(node1==NULL)
    {
        strcpy(message,"Set-ul 1 introdus nu exista!");
        return message;
    } else if(node1->isOrdonated!=true)
    {
        return "Set-ul 1 nu este un set ordonat!";
    }

    ptr=strtok(NULL,"\0\n");
    searchTree* node2=findElementByKey(BST,ptr);
    if(node2==NULL)
    {
        strcpy(message,"Set-ul 2 introdus nu exista!");
        return message;
    }else if(node2->isOrdonated!=true)
    {
        return "Set-ul 2 nu este un set ordonat!";
    }
    char** copy_value=(char**)malloc(sizeof(char*)*70);
    int prioritati[70];
    int nr_elem=0;
    for(int i=0;i<70;i++)
        copy_value[i]=(char*)malloc(sizeof(char)*70);
    for(int i=0;i<node1->numberOfElements;i++)
    {
        strcpy(copy_value[i],node1->values[i]);
        prioritati[i]=node1->priorites[i];
        nr_elem++;
    }
    for(int i=0;i<node2->numberOfElements;i++)
    {
        int ok=0;
        for(int j=0;j<nr_elem;j++)
        {
            if(strcmp(node2->values[i],copy_value[j])==0)
             {   
                prioritati[j]+=node2->priorites[i];
                ok=1;
             }
        }
        if(ok==0)
        {
            strcpy(copy_value[nr_elem],node2->values[i]);
            prioritati[nr_elem]=node2->priorites[i];
            nr_elem++;
        }
    }
    int swapped;

    do {
    swapped = 0;
    for (int i = 0; i < nr_elem - 1; i++) {
        if (prioritati[i] > prioritati[i + 1]) {
            int tempPriority = prioritati[i];
            prioritati[i] = prioritati[i + 1];
            prioritati[i + 1] = tempPriority;
            char tempValue[100];
            strcpy(tempValue, copy_value[i]);
            strcpy(copy_value[i],copy_value[i + 1]);
            strcpy(copy_value[i + 1], tempValue);
            swapped = 1;
        }
    }
    } while (swapped);
    BST=insertIntoTree(BST,setname,copy_value,false,false,nr_elem,true,prioritati);
    FILE* file = fopen("./serverUtils/setOrd.txt", "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Salvează întregul arbore în fișier
    saveTreeToFile(file, BST);
    strcpy(message,"Reuniunea a fost salvata in fisier!");
    return message;
}


char* execute_help(char* buffer)
{
    char* message=(char*)malloc(sizeof(char)*2500);
    FILE* file=fopen("./serverUtils/help.txt","r");
    if(file==NULL)
    {
        perror("eroare la deshiderea fisierului help!");
        return "eroare la deschiderea fisierului help!";
    }
    fseek(file, 0, SEEK_END);
    int lungime = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(message, sizeof(char), lungime, file);
    return message;
}

char* execute_zdel(char* buffer)
{
    char* ptr=strtok(buffer," ");
    ptr=strtok(NULL,"\0\n");
    searchTree* node=findElementByKey(BST,ptr);
    if(node==NULL)
        return "Set-ul introdus nu exista!";
    else if(node->isOrdonated!=true)
        return "Set-ul introdus nu este un set ordonat!";
    deleteNode(BST,ptr);
    FILE* file = fopen("./serverUtils/setOrd.txt", "w+");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // Salvează întregul arbore în fișier
    saveTreeToFile(file, BST);
    return "Set-ul a fost sters cu succes!";
}