#include "arbore.h"
#include <stdlib.h>
#include <string.h>

searchTree* insertIntoTree(searchTree* node,char* key,char**values,bool hasList)
{
	if(node==NULL)
	{
		node=(searchTree*)malloc(sizeof(searchTree));
		node->key=key;
		node->hasList=hasList;
		node->values=values;
		node->numberOfElements=1;
		node->leftNode=NULL;
		node->rightNode=NULL;
		return node;
	}
	if(compare(key,node->key))
		node->rightNode=insertIntoTree(node->rightNode,key,values,hasList);
	if(!compare(key,node->key))
		node->leftNode=insertIntoTree(node->leftNode,key,values,hasList);

		return node;
}

searchTree* findElementByKey(searchTree* node, char* key) {
    if (node == NULL || strcmp(node->key, key) == 0) {
        return node;
    }

    if (compare(key, node->key)) {
        return findElementByKey(node->rightNode, key);
    } else {
        return findElementByKey(node->leftNode, key);
    }
}


bool compare(char*key1,char*key2)
{
	if(strlen(key1)>strlen(key2))
		return true;
	else if(strlen(key2)>strlen(key1))
		return false;
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
