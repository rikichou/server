

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "listNode.h"

/*
listNode_t *_listNodeNew(const void *data, int dataSize)
{
	listNode_t *node;
	int i;

    if (data == NULL)
    {
        dataSize = 0;
    }
    
	node = (listNode_t *)malloc(sizeof(*node) + dataSize);
    if (node == NULL)
    {  
        return NULL;
    }
    
    node->next = NULL;
	node->data = NULL;
    
    if (data && dataSize)
    {
        node->data = node + 1;
        memcpy(node->data, data, dataSize);
    }

    return node;
}
*/

listNode_t *listSort(listNode_t *head, int (*compare)(void *data1, void *data2))
{
//    int length = 0;
    listNode_t *node, *preNode, temp;
    
    if (head == NULL || compare == NULL)
    {
        return NULL;
    }

    for (preNode = head; preNode != NULL; preNode = preNode->next)
    {
        for (node = preNode->next; node != NULL; node = node->next)
        {   
            if (compare(preNode->data, node->data))
            {
                temp.valid = preNode->valid;
                temp.data  = preNode->data;

                preNode->valid = node->valid;
                preNode->data  = node->data;

                node->valid = temp.valid;
                node->data  = temp.data;
            }
        }
    }
    
    return head;
}

static unsigned long listHash(const char *s)
{
	unsigned long hash = 0;
	
	while (*s) 
		hash = 31 * hash + *s++;
	return hash;
}


listNode_t *hashListGet(listNode_t **head, int headNum, const char *text)
{
    listNode_t *item;     
	int i;
	i = listHash(text) % headNum;

	item = head[i];
    
	while(item)
    {
	    if (!strcmp(text, item->text))
        {
		    return item;
	    }
		item = item->next;
	} 
    
	return NULL;
}


/* just add ,do not check */
int hashListAdd(listNode_t **head, int headNum, const char *text, void *data, int dataSize)
{
	listNode_t *item;
	int i;

    if (data == NULL)
    {
        dataSize = 0;
    }
    
	item = (listNode_t *)malloc(sizeof(*item) + dataSize);
    if (item == NULL)
    {
        return -1;
    }
    
    item->next = NULL;    
	item->text = strdup(text);
	item->data = NULL;
    
    if (data)
    {
        if (dataSize)
        {
            item->data = item + 1;
            memcpy(item->data, data, dataSize);
        }
        else 
        {
            item->data = data;
        }
    }    
    
	i = listHash(item->text) % headNum;

    item->next = head[i];
    head[i] = item;
    
    /* add to tail 
    {
        listNode_t *prev = head[i];
        while(prev && prev->next)
        {
            prev = prev->next;
        }

        if (prev == NULL)
        {
            head[i] = item;
        }
        else 
        {
            prev->next = item;
        }
    }   
    */       

    return 1;
}

int hashListDelete(listNode_t **head, int headNum, const char *text)
{
    listNode_t *item, *prev = NULL, *next;     
	int i;
	i = listHash(text) % headNum;

	item = head[i];
    
	while(item)
    {
        next = item->next;
        
	    if (!strcmp(text, item->text))
        {
		    if (prev == NULL)
            {
                head[i] = next;
            }      
            else 
            {
                prev->next = next;
            }

            /* free item */
            free(item->text);
            free(item);
	    }

        prev = item;
		item = next;
	} 
    
	return 0;
}

