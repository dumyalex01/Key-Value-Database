#include "loginList.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void insertElement(listNode **A, char *username, char *password)
{
    if (*A == NULL)
    {
        (*A) = (listNode *)malloc(sizeof(listNode));
        strcpy((*A)->username, username);
        strcpy((*A)->password, password);
        (*A)->next = NULL;
    }
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