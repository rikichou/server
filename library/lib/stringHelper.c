
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : stringHelper.c
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



/*
 * stringHelper.c
 * 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "stringHelper.h"

#ifdef CONFIG_BASE64

#include "base64.h"

#endif 

/*
    check if the string is include char and number only
    if NULL input, output 0
*/
int stringIsalnum(const char *string)
{
    int length = 0, i = 0;

    if (!string)
    {
        return 0;
    }

    length = strlen(string);

    for (i = 0; i < length; i ++)
    {
        if (!isalnum(string[i]))
        {
                return 0;
        }
    }

    return 1;
}

int stringSplit(char *s, char tok, char **ret,  int ret_num)
{
	char *cp;
	int n = 0;

	cp = s;
	if(!*cp) return n;
    
	n = 1;
	while(*cp) 
    {
		if (n -1 < ret_num)
        {
			ret[n - 1] = cp;
		}
        else 
        {
			return n -1;
		}
        
		while(*cp && *cp != tok)
        {
            cp ++;
        }
        
		if(*cp == tok)
        {
            n = n + 1;
        }
        
		if(*cp == 0)
        {
            return n;
        }
        
		*cp = 0;
		cp ++;
	}
	
	if (n -1 < ret_num)
    {
		ret[n - 1] = cp;
	}
    else 
    {
		return n - 1;
	}
    
	return n;
}

/*
 need to free
*/
char *strdupSplit(const char *s, char tok, char **vars, int *vars_num)
{
	int num;
	char *sdup = strdup(s);
    
	if (sdup == NULL) 
    {
        return NULL;
    }
    
	num = stringSplit(sdup, tok, vars, *vars_num);
    
	*vars_num = num;
    
	return sdup;
}

#ifdef CONFIG_BASE64

char *strdupBase64(const char *s)
{
    int size, len;
    char *base64;

    size = strlen(s);

    if (size <= 0)
    {
        return NULL;
    }

    /* for small size of string, it needs more for encoding */
    if (size < 8)
    {
        size = 8;
    }

    len = size + (size >> 1);

    base64 = malloc(len /* max size is about 1.4 times of origin */);

    if (base64 == NULL)
    {
        return NULL;
    }

    return base64Encode((unsigned char *)s, size, base64);
}

#endif 


char *stringToUpper(char *str)
{
	char *p = str;
	while(*p)
    {
		*p = toupper(*p);
		p ++;
	}
	return str;
}


char *stringToLower(char *str)
{
	char *p = str;
	while(*p)
    {
		*p = tolower(*p);
		p ++;
	}
	return str;
}


char *trim(char *s)
{
	size_t len = strlen(s);
	size_t lws;

	/* trim trailing whitespace */
	while (len && isspace(s[len-1]))
    {
        --len;
    }   
	
	/* trim leading whitespace */
	if (len)
    {
		lws = strspn(s, " \n\r\t\v");
		if (lws) 
        {
			len -= lws;
			memmove(s, s + lws, len);
		}
	}
    
	s[len] = '\0';

    return s;
}


char *trimRight(char *s)
{
	size_t len = strlen(s);

	/* trim trailing whitespace */
	while (len && isspace(s[len-1]))
    {
		--len;
    }   
    
	s[len] = '\0';
    
    return s;
}



char *stringAfter(const char *str, const char *needle)
{
    const char *p = needle;
    while(*str && *p)
    {
        if (*str != *p)
        {
            p = needle;
        }
        else 
        {
            p ++;
        }        
        str ++;
    }

    if (*p == '\0')
    {
        return (char *)str;
    }
    return NULL;
}

/* quotation means  range in special char "" */
char *stringTrimBetween(char *str, char quotMark)
{
    char *reBase, *p;

    reBase = strchr(str, quotMark);

    if (reBase == NULL)
    {
        reBase = str;
    }
    else 
    {
        reBase += 1;
    }

    /*
    if one of this, try to get the pair one         
    */
    if (quotMark == '[')
    {
        quotMark = ']';
    }    
    else if (quotMark == '<')
    {
        quotMark = '>';
    }
    else if (quotMark == '{')
    {
        quotMark = '}';
    }
    else if (quotMark == '(')
    {
        quotMark = ')';
    }    

    p = strrchr(reBase, quotMark);
    if (p)
    {
        *p = '\0';    
    }

    return reBase;
}

/* endBefore means  find the next special char */
static char *__stringTrim(char *str, char endChar)
{
    char *reBase;

    /* trim left */
    while (*str)
    {
        if ((*str == ' ') || (*str == '\n') || (*str == '\a') || (*str == '\t'))
        {
            str ++;
        }
        else
        {
            break;
        }
    }

    reBase = str;

    /* trim right */

    str = NULL;
    
    if (endChar)
    {
        str = strchr(reBase, endChar);
        if (str)
        {
            *str = '\0';
            str --;
        }  
    }
    
    if (str == NULL)
    {
        str = reBase + strlen(reBase) - 1;
    }
    
    while(str > reBase)
    {
        if ((*str == ' ') || (*str == '\n') || (*str == '\a') || (*str == '\t'))
        {
            *str = '\0';
            str --;
        }
        else
        {
            break;
        }
    }    
   
	return reBase;
}

char *stringTrim(char *str)
{
    return __stringTrim(str, 0);
}

char *stringTrimTo(char *str, char spec)
{
    return __stringTrim(str, spec);
}

const char *stringAt(const char *stringArray, int index, int offset)
{
    int i;
    const char *p;
    
    /* the first one */
    if (index <= offset)
    {
        return stringArray;
    }

    p = stringArray;
    i = 0;
    
    while(i >= 0)
    {       
        if (*p == '\0')
        {
            /* end condition ? */
            if (*(p  + 1) == '\0')
            {
                return p;
            }

            i ++;

            if (i == (index - offset))
            {
                return (p + 1);
            }
        }
        p ++;
    }

    return "";
}



int indexOf(const char *stringArray, const char *subString, int offset)
{

    int i;
    const char *p;

    p = stringArray;
    i = 0;
    
    while(*p)
    {
        if (!strcmp(p, subString))
        {
            return i + offset;
        }

        /* to next string */

        while(*p)
        {
            p ++;
        }

        i ++;
        p ++;
    }

    return -1;
}


#if 0
/* indexToString */

const char *indexToString(int index, const indexStringMap_t *map)
{
    int i;
    for (i = 0; map[i].string; i ++)
    {
        if (map[i].index == index)
        {
            return map[i].string;
        }
    }
    return "";
}

int stringToIndex(const char *string, const indexStringMap_t *map)
{
    int i;
    for (i = 0; map[i].string; i ++)
    {
        if (!strcmp(map[i].string, string))
        {
            return map[i].index;
        }
    }
    return -1;
}

/* string */
int stringIndex(const char *str, const char *stringArray, int indexBase, int defaultValue)
{
    int len = strlen(str);
    const char *p = stringArray;
    
    while(p && *p)
    {
        if (*p == ',')
        {
            p ++;
            indexBase ++;            
            continue;
        }
    
        if (!strncmp(str, p, len))
        {
            /* the first len bytes match, the next byte should be ',' or '\0' */
            if ((p[len] == ',') || (p[len] == '\0'))
            {
                return indexBase;
            }
        }

        /* not match, try next */
        p = strchr(p, ',');

        if (p)
        {
            p ++;
            indexBase ++;
        }		
    }

    return defaultValue;
}


const char *indexString(int index, const char *stringArray, int indexBase, int defaultValue, char *buffer, int size)
{
    int len;
    int tryAgain = 0;
    
    const char *p = stringArray;

    if (!buffer || (size < 1))
    {
        /* if not available, return default value */
        buffer = NULL;
        index = defaultValue;
        tryAgain = 2;
    }

    do
    {
        if (p && *p)
        {
            if (indexBase == index)
            {
                /* copy to buffer */
                if (buffer)
                {   
                    len = 0;
                    while((*p != ',') && (*p != '\0') && (len < size - 1))
                    {
                        buffer[len ++] = *p ++;
                    }
                    buffer[len] = '\0';  
                    return buffer;
                }

                if (tryAgain == 0)
                {
                    /* try again to get the default one */
                    tryAgain = 1;
                }
                else 
                {                       
                    return "notFound";
                }
            }
            
            if (*p == ',')
            {
                p ++;
                indexBase ++;              
                continue;
            }   
            /* not match, try next */
            p = strchr(p, ',');

            if (p)
            {
                p ++;
                indexBase ++;
            }
            else if (tryAgain == 0)
            {
                tryAgain = 1;
            }            
        }	

        if (tryAgain == 1)
        {
            p = stringArray;
            index = defaultValue;
            tryAgain ++;
            continue;
        }
        
    }while(p && *p);

    return "notFound";
}

#endif /* */


