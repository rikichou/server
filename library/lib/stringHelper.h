
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : stringHelper.h
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




#ifndef __STRINGHELPER_H__
#define __STRINGHELPER_H__

#include <stdint.h>
#include <string.h>

#define safeStrncpy(dest, src, dest_size) \
do { \
    strncpy(dest, src, dest_size - 1);  \
    dest[dest_size - 1] = '\0'; \
}while(0)


#define free_s(ptr) \
do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
}while(0)

#ifndef memdup
#define memdup(dst, src) \
do { \
    dst = (typeof(dst)) malloc(sizeof(*(src))); \
    if (dst == NULL) {debugf("malloc", "malloc(%d)", sizeof(*(src)));} \
    if (dst) {memcpy(dst, src, sizeof(*(src)));} \
}while(0)
#endif
    
#define memndup(dst, src, n) \
    do { \
        dst = (typeof(dst)) malloc(n); \
        if (dst == NULL) {debugf("malloc", "malloc(%d)", n);} \
        if (dst) {memcpy(dst, src, n);} \
    }while(0)


/* string */

int stringSplit(char *s, char tok, char **ret,  int ret_num);
char *strdupSplit(const char *s, char tok, char **vars, int *vars_num);

#ifdef CONFIG_BASE64
char *strdupBase64(const char *s);
#endif 

char *stringToUpper(char *str);
char *stringToLower(char *str);

char *trim(char *s);
char *trimRight(char *s);

char *stringAfter(const char *str, const char *needle);
/* quotation means  range in special char "" */
char *stringTrimBetween(char *str, char quotMark);
/* endBefore means  find the next special char, if zero, act a normal trim */
char *stringTrim(char *str);
char *stringTrimTo(char *str, char spec);

const char *stringAt(const char *stringArray, int index, int offset);
int indexOf(const char *stringArray, const char *subString, int offset);

int stringIsalnum(const char *string);

#if 0

typedef struct 
{
    int index;
    const char *string;
}indexStringMap_t;


const char *indexToString(int index, const indexStringMap_t *map);
int stringToIndex(const char *string, const indexStringMap_t *map);    


int stringIndex(const char *str, const char *stringArray, int indexBase, int defaultValue);
const char *indexString(int index, const char *stringArray, int indexBase, int defaultValue, char *buffer, int size);

#define indexOf(str, str_prefix) stringIndex(str, str_prefix##_STRING, str_prefix##_BASE, str_prefix##_DEFAULT) 
#define stringAt(index, str_prefix, buffer, size) indexString(index, str_prefix##_STRING, str_prefix##_BASE, str_prefix##_DEFAULT, buffer, size)

#endif 

#endif /* __STRINGHELPER_H__ */

