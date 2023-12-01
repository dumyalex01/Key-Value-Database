#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct listNode
{
    char nume[25];
    char parola[25];
   struct listNode*next;
}listNode;
void insertNodeHeadOfTheList(listNode** A,char*nume,char*parola)
{
    if(*A==NULL)
    {
        *A=(listNode*)malloc(sizeof(listNode));
        strcpy((*A)->nume,nume);
        strcpy((*A)->parola,parola);
        (*A)->next=NULL;
    }
    else
    {
        listNode* newNode=(listNode*)malloc(sizeof(listNode));
        strcpy(newNode->nume,nume);
        strcpy(newNode->parola,parola);
        newNode->next=*A;
        (*A)=newNode;
    }
}
void printList(listNode*A)
{
  while(A!=NULL)
  {
    printf("%s %s\n",A->nume,A->parola);
    A=A->next;
  }
    
}

void main()
{
    listNode*A;
    insertNodeHeadOfTheList(&A,"hello","there");
    insertNodeHeadOfTheList(&A,"hi","man");
    insertNodeHeadOfTheList(&A,"Salut","Aici");
    printList(A);
}