#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
typedef struct searchTree
{
	char* key;
	bool hasList;
	int numberOfElements;
	char**values;
	struct	searchTree*leftNode;
	struct	searchTree*rightNode;


}searchTree;

searchTree* insertIntoTree(searchTree* node,char* key,char**values,bool hasList);
void deleteFromTree(searchTree* tree,char* key);
searchTree* findElementByKey(searchTree*node,char*key);
bool compare(char*key1,char*key2);