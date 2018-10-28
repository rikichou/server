
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : listNode.h
Status       : Current
Description  : 

Author       : Liu Chuansen
Contact      : 179712066@qq.com

Revision     : 2014-04 ~ 2014-10
Description  : Primary released

## Please log your description here for your modication ##

Revision     : 
Modifier     : 
Description  : 

*/




#ifndef __LISTNODE_H__ 
#define __LISTNODE_H__

enum
{
    EMPTY = 0,
    VALID = 1,
    APPEND = 2,
    UPDATE = 3,
    DELETE = 4,    
};

#define validNode(_node) (((_node)->valid == APPEND) || ((_node)->valid == UPDATE) || ((_node)->valid == VALID))
#define emptyNode(_node) ((_node)->valid == EMPTY)

struct listNode
{
    struct listNode *next;
    int valid;
    void *data;
    union 
    {
        char *text;    /* for hash only */
        void *oldData; /* old data pointer */
    };
};

typedef struct listNode listNode_t;

/*
listNode_t *_listNodeNew(const void *data, int dataSize);

#define listNodeNew(_node, _data, _size) \
do { \
    _node = _listNodeNew(_data, _size); \
    if (_node == NULL) \
    { \
        debugf("malloc", "malloc(%d)", sizeof(struct listNode) + _size); \
    } \
}while(0)
*/

#define listNew(_node) \
do { \
    _node = (struct listNode *)malloc(sizeof(struct listNode)); \
    if (_node != NULL) \
    { \
        memset(_node, 0, sizeof(struct listNode)); \
    } \
    else { \
        debugf("malloc", "malloc(%d)", sizeof(struct listNode));\
    } \
}while(0)

#define listFree(_head) \
do { \
    listNode_t *_list, *_next; \
    _list = _head; \
    while(_list) \
    { \
        if (_list->data) \
        { \
            free(_list->data); \
        } \
        if (_list->oldData) \
        { \
            free(_list->oldData); \
        } \
        _next = _list->next; \
        free(_list); \
        _list = _next; \
    } \
    _head = NULL; \
}while(0)

listNode_t *listSort(listNode_t *head, int (*compare)(void *data1, void *data2));

listNode_t *hashListGet(listNode_t **head, int headNum, const char *text);
int hashListAdd(listNode_t **head, int headNum, const char *text, void *data, int dataSize);        
int hashListDelete(listNode_t **head, int headNum, const char *text);

#endif /* __LISTNODE_H__*/

