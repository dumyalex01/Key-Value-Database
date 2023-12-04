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
        node->isSet=hasSet;

        // Alocare memorie separată pentru values și copierea valorii           
        if(node->isList || node->isSet)
            {
                node->values = (char**)malloc(sizeof(char*)*50);
            for(int i=0;i<50;i++)
                {node->values[i]=(char*)malloc(sizeof(char)*100);
                    if(values[i]!=NULL)
                    strcpy(node->values[i],values[i]);
                }
            

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
void run(int clientSocket);
void sendMessageToClient(int clientSocket, char *messageToSend);
void cleanup(int clientSocket);
char *execute_login(char *buffer);
char *execute_command(char *buffer);
char* execute_get(char* buffer);
bool verify_credentials(char *username, char *password, bool forLogin);
char *execute_autentificare(char *buffer);
void add_logger(char*text);
char*execute_logout();
char* user=NULL;
char *execute_help();


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
       printf("S-au introdus in tree: %s %s\n",key,values[0]);
    }
    }
    fclose(f);
}
void populateListandSet(bool isList1)
{   const char*message;
    const char*filename;
    bool isSet;
    bool isList;
    if(isList1)
    {
        filename="./serverUtils/list.txt";
        isSet=false;
        isList=true;
        message="S-a adaugat in tree lista ";
    }
    else
    {
        filename="./serverUtils/set.txt";
        isSet=true;
        isList=false;
        message="S-a adaugat in tree setul ";
    }
    char buffer[1000];
    char**values;
    int counter=0;
    values=(char**)malloc(sizeof(char*)*100);
    for(int i=0;i<100;i++)
        values[i]=(char*)malloc(sizeof(char)*100);                                                  //populeaza arborele cu tot ce se gaseste in list.txt
    FILE*f=fopen(filename,"r");
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
        printf("%s %s ",message,key);
        for(int i=0;i<counter;i++)
            printf("%s,",values[i]);
        printf("\n");
    }
    fclose(f);
}
void populate_BST()
{
   populateSimple();
   populateListandSet(true);
   populateListandSet(false);
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
        BST=insertIntoTree(BST,key,values,false,true,0);
        FILE*f=fopen("./serverUtils/set.txt","a");
        fprintf(f,"\n%s-",key);
        fclose(f);
        return "SET creat cu succes!";
    }
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
            bufferToReturn[strlen(bufferToReturn-2)]='>';
            bufferToReturn[strlen(bufferToReturn-1)]='\0';
        }
        else return "SET VID";
        return bufferToReturn;

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
void updateFile(char*key,bool isList1)
{
    const char*filename;
    if(isList1)
        filename="./serverUtils/list.txt";
    else filename="./serverUtils/set.txt";
    char**vectorLinii=(char**)malloc(sizeof(char*)*100);    
    int counter=0;
    for(int i=0;i<100;i++)
        vectorLinii[i]=(char*)malloc(sizeof(char)*200);
    FILE*f=fopen(filename,"r");
    char buffer[250];
    char copie[250];
    while(!feof(f))
    {
        fscanf(f,"%s",buffer);
        strcpy(copie,buffer);
        char*key_f=strtok(copie,"-");
        if(strcmp(key_f,key)==0)
        {
            searchTree*node=findElementByKey(BST,key);
            strcpy(buffer,key);
            strcat(buffer,"-");
            for(int i=0;i<node->numberOfElements;i++)
            {
                strcat(buffer,node->values[i]);
                strcat(buffer,",");
            }
        }
        strcpy(vectorLinii[counter],buffer);
        counter++;
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
            updateFile(key,true);
            char buf[100];
            buf[0]='\0';
            strcat(buf,user);
            strcat(buf," a folosit comanda LPOP pe lista: ");
            strcat(buf,word);
            strcat(buf, "!");
            add_logger(buf);
            return bufferToReturn;
        }
    }
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
            updateFile(key,true);
            char buf[100];
            buf[0]='\0';
            strcat(buf,user);
            strcat(buf," a folosit comanda RPOP pe lista: ");
            strcat(buf,word);
            strcat(buf, "!");
            add_logger(buf);
            return bufferToReturn;
        }
    }
}
char* execute_lpush(char*buffer)
{
    char key[100];
    char value[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    addValueToLeft(BST,key,value);
    updateFile(key,true);
    if(BST==NULL)
        return "NOTOK";
    else return "OK";
}
char* execute_rpush(char*buffer)
{
    char key[100];
    char value[100];
    char*word=strtok(buffer," ");
    word=strtok(NULL," ");
    strcpy(key,word);
    word=strtok(NULL," ");
    strcpy(value,word);
    addValueToRight(BST,key,value);
    updateFile(key,true);
    if(BST==NULL)
        return "NOTOK";
    else return "OK";

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
        fprintf(f,"\n%s-",key);
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
    updateFile(key,false);
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
        updateFile(key,false);
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
        BST=insertIntoTree(BST,key,value,false,false,1);
        FILE*f=fopen("./serverUtils/simple.txt","a");
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

char *execute_command(char *buffer)
{   
    char *copie=(char*)malloc(sizeof(char)*70);
    strcpy(copie, buffer);
    char *protocol = strtok(copie, " ");
    char *messageToSend = malloc(sizeof(char) * 1500);
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
    if(strcmp(protocol,"LOGOUT")==0)
        strcpy(messageToSend,execute_logout());
    if(strcmp(protocol,"HELP")==0)
        strcpy(messageToSend,execute_help());
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
         {
        strcpy(bufferToReturn, "DA");
        user=(char*)malloc(sizeof(char)*strlen(username));
        strcpy(user,username);
        char buf[100];
        buf[0]='\0';
        strcat(buf,user);
        strcat(buf," s-a autentificat cu succes!");
        add_logger(buf);
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
        if (messageReceived > 0)
        {
            // Aici functie execute_command care returneaza un char* de trimis,In ea vor fi functii pentru fiecare comanda(login,autentif,set,get,...);
            char* message=(char*)malloc(sizeof(char)*1500);
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
    char buf[100];
    buf[0]='\0';
    strcat(buf,user);
    strcat(buf," s-a delogat cu succes!");
    add_logger(buf);
    user=NULL;
    return aux;
}

char *execute_help()
{
    FILE* fisier=fopen("./serverUtils/help.txt","r");
    if(fisier==NULL)
    {
        perror("Eroare la deschidere fisier HELP!");
        exit(-1);
    }
    fseek(fisier, 0, SEEK_END);
    int lungime = ftell(fisier);
    fseek(fisier, 0, SEEK_SET);

    char *continut = (char *)malloc(sizeof(char)*(lungime+1));
    if (continut == NULL) {
        perror("Eroare la alocare memorie!");
        exit(-1);
    }

    fread(continut, sizeof(char), lungime, fisier);
   
    fclose(fisier);
    return continut;
}