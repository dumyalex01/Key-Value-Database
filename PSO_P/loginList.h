#include <stdbool.h>
typedef struct listNode
{
    char username[30];
    char password[30];
    struct listNode *next;
} listNode;
void insertElement(listNode **A, char *username, char *password);
bool findElement(listNode *A, char username[30], char password[30]);
bool findUsername(listNode *A, char username[30]);