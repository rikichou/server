
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : configFile.c
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




#include "configFile.h"

#ifdef STANDALONE

#ifdef DDEBUG
#define dbg(id, format, ...)  printf("(" id ")" format "\n", ##__VA_ARGS__)
#else
#define dbg(format, ...)
#endif 

#include <ctype.h>

static char *trim(char *s)
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

#else

#include "stringHelper.h"

#define dbg(id, format, ...)

#endif /* STANDALONE */

static inline uint32_t hash(const char *s)
{
    uint32_t hash = 0;

    while (*s)
    {
        hash = 31 * hash + *s++;
    }
    return hash;
}

static int __configItemSet(configHead_t head, const char *name, const char *value)
{
    struct configItem *item, *prev;
    int i, unSet;

    if (!head || !name)
    {
        return 0;
    }

    unSet = (value == NULL) ? 1 : 0;
   
    i = hash(name) % CONFIGFILE_HASH_ARRAY_SIZE;

    item = head[i];
    prev = NULL;

    while(item)
    {
        if (!strcmp(name, item->name))
        {
            if (unSet)
            {
                if (prev == NULL)
                {
                    head[i] = item->next;
                }
                else 
                {
                    prev->next = item->next;
                }

                if (item->value)
                {
                    free(item->value);
                }
                if (item->name)
                {
                    free(item->name);
                }
                free(item);                
            }
            else 
            {
                if (item->value)
                {
                    free(item->value);
                }
                
                item->value = strdup(value);
                dbg("cfgfile", "***%s: replace %s = %s\n", __FUNCTION__, name, value);                
            }
            return 1;
        }
        item = item->next;
    }

    if (!unSet)
    {
        item = (struct configItem *)malloc(sizeof(struct configItem));
        
        if (item)
        {
            memset(item, 0, sizeof(struct configItem));
        
            item->name = strdup(name);
            item->value = strdup(value);
            item->next = head[i];
            head[i] = item;

            dbg("cfgfile", "***%s: set %s = %s\n", __FUNCTION__, name, value);  

            return 1;
        }
    }

    return 0;
}


static int configLineSet(configHead_t head, char *line)
{
    char *eq, *name, *value;
    
    eq = strchr(line, '=');

    if (eq == NULL)
    {
        return 0;
    }
    *eq ++ = '\0';

    name = trim(line);

    /* skip comment line */
    if ((name[0] == '#') || (name[0] == ';'))
    {
        dbg("cfgfile", "***Comment line, skip");    
        return 0;
    }   

    value = trim(eq);

    return __configItemSet(head, name, value);
}


configHead_t *configFileParse(const char *path)
{
    configHead_t *head;
    FILE *fp;
    char line[CONFIGFILE_LINE_MAX_SIZE]; 
    int count = 0;
    
    fp = fopen(path, "r");

    if (fp == NULL)
    {
        printf("***%s: can not open file:%s\n", __FUNCTION__, path);    
        return NULL;
    }

    head = (configHead_t *)malloc(sizeof(configHead_t));

    if (head == NULL)
    {
        fclose(fp);
        return NULL;
    }

    memset(head, 0, sizeof(configHead_t));

    while(!feof(fp)) 
    {
        line[0] = '\0';
        if (fgets(line, sizeof(line) - 1, fp))                
        {
            line[sizeof(line) - 1 ]= '\0';
            dbg("cfgfile", "==> [%s]\n", line);
            count += configLineSet(*head, line);
        } 
    }        

    fclose(fp);

    return head;
}


configHead_t *configHeadNew(void)
{
    configHead_t *head;

    head = (configHead_t *)malloc(sizeof(configHead_t));

    if (head)
    {
        memset(head, 0, sizeof(configHead_t));
    }

    return head;
}

void configItemSet(configHead_t head, const char *name, const char *value)
{
    __configItemSet(head, name, value);
}

const char *configItemGet(configHead_t head, const char *name)
{
    struct configItem *item;
    int i;

    if (!head || !name)
    {
        return NULL;
    }
   
    i = hash(name) % CONFIGFILE_HASH_ARRAY_SIZE;

    item = head[i];

    while(item)
    {
        if (!strcmp(name, item->name))
        {
            return item->value;
        }
        item = item->next;
    }

    return NULL;
}


int configItemPrint(configHead_t head, const char *name, const char *format, ...)
{
    int ret;
    char buffer[CONFIGFILE_PRINT_BUFFER_SIZE];
    va_list args;

    va_start (args, format);
    ret = vsnprintf (buffer, sizeof(buffer) - 1, format, args);
    va_end (args);

    __configItemSet(head, name, buffer);
    
    return ret;
} 

/* quote function, it will add or remove the double quotes to or from the values */
void configItemSetQuote(configHead_t head, const char *name, const char *value)
{
    const char *p;
    int quoteNum = 0;
    char *newValue, *q;    

    p = value;
    
    while(*p)
    {
        if (*p == '"')
        {
            quoteNum ++;
        }
        p ++;
    }

    newValue = malloc((p - value) + quoteNum + 2 + 1); /* main string size, encoded '\' num + begining and ending double quotes + ending char */

    if (newValue == NULL)
    {
        printf("***%s: malloc(%d) failed\n", __FUNCTION__, (p - value) + quoteNum + 2 + 1);    
        return;
    }
    
    p = value;
    q = newValue;
    
    *q = '"'; q ++;

    if (quoteNum)
    {
        while(*p)
        {
            if (*p == '"')
            {
                *q = '\\'; q ++;
                *q = '"'; q ++;
            }
            else 
            {
                *q = *p; q ++;                
            }
            p ++;
        }        
    }
    else 
    {
        while(*p)
        {
            *q = *p; 
            q ++;          
            p ++;
        }        
    }
    
    *q = '"'; q ++;    
    *q = '\0';
    
    __configItemSet(head, name, newValue);

    free(newValue);
}


int configItemPrintQuote(configHead_t head, const char *name, const char *format, ...)
{
    int ret;
    char buffer[CONFIGFILE_PRINT_BUFFER_SIZE];
    va_list args;

    va_start (args, format);
    ret = vsnprintf (buffer, sizeof(buffer) - 1, format, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end (args);

    configItemSetQuote(head, name, buffer);
    
    return ret;
} 


/* 
 if duplicate is set , you need to free the return string if it is not empty
 if duplicate is not set , the trim action will affect the values saved in list

 " sdfjsdrjwewe\" kdjsdkfjsd "
 sdfsasdrwer"dsdfsdf\"sdfsdf
     "  jkhjhkj"  jkk
*/
#if 0
const char *configItemGetQuote(configHead_t head, const char *name, int duplicate)
{
    char *value;

    value = configItemGet(head, name);

    if (duplicate)
    {
        value = strdup(value);
    }
    
    if (value)
    {
        int quoteString = 0;
        
        char *p = value;    

        /* check if it is a string in quotation */
        while(*p)
        {
            if ((*p == ' ') || (*p == '\t') || (*p == '\r') || (*p == '\n'))
            {
                p ++;
            }
            else if (*p == '"')
            {                
                quoteString = 1;
                break;
            }
            else 
            {
                break;
            }
        }

        if (!quoteString)
        {
            return value;
        }

        /* */

        
        p = strchr(value, '"');

        if (p == )
        
        while(*p)
        {
            
        }
    }

    return value;
}

#endif 

void configItemUnset(configHead_t head, const char *name)
{
    __configItemSet(head, name, NULL);
}

int configFileOutput(const char *path, configHead_t head, int export)
{
    int i, count;
    FILE *fp = stdout;
    struct configItem *item;
    
    if (!head)
    {
        return 0;
    }

    if (path)
    {
        fp = fopen(path, "w+");
        
        if (fp == NULL)
        {
            printf("***%s: can not open file:%s\n", __FUNCTION__, path);
            return 0;
        }
    }

    count = 0;
    
    for (i = 0; i < CONFIGFILE_HASH_ARRAY_SIZE; i ++)
    {
        item = head[i];
        while(item)
        {
            fprintf(fp, "%s%s=%s\n", export ? "export " : "", item->name, item->value ? item->value : "");            
            count ++;
            item = item->next;
        }
    }

    if (path)
    {
        fclose(fp);
    }
    
    return count;
}


int configFileWrite(const char *path, configHead_t head)
{
  return  configFileOutput(path, head, 0);
}
 

void configHeadFree(configHead_t head)
{
    int i;
    struct configItem *item, *next;

    if (!head)
    {
        return ;
    }
        
    for (i = 0; i < CONFIGFILE_HASH_ARRAY_SIZE; i ++)
    {
        item = head[i];
        while(item)
        {
            next = item->next;
            
            if (item->value)
            {
                free(item->value);
            }
            if (item->name)
            {
                free(item->name);
            }
            free(item);
            
            item = next;
        }
    }

    free(head);
}


/*
    return value:
        1 - set ok
        0 - set failed
*/

int configFileSet(const char *path, const char *name, const char *value)
{
    int ret;
    configHead_t *head;

    if (!path || !name)
    {
        return 0;
    }
    
    head = configFileParse(path);

    if (head == NULL)
    {
        return 0;
    }

    configItemSet(*head, name, value ? value : "");

    ret = configFileWrite(path, *head);

    configHeadFree(*head);

    return ret; 
}


int configFileUnset(const char *path, const char *name)
{
    int ret;
    configHead_t *head;

    if (!path || !name)
    {
        return 0;
    }
    
    head = configFileParse(path);

    if (head == NULL)
    {
        return 0;
    }

    configItemUnset(*head, name);

    ret = configFileWrite(path, *head);

    configHeadFree(*head);

    return ret; 
}


#ifdef STANDALONE

/*
    gcc -Wall -o cfgfile configFile.c -DSTANDALONE -I ../../include     
*/
/* 
 cfgfile CONFIG set NAME=VALUE [NAME=VALUE] [-o OUTPUT]
 cfgfile CONFIG export [-o OUTPUT]
 cfgfile CONFIG unset NAME [NAME] [-o OUTPUT]
*/

const char *s_usage = "Usage:\n"
    " cfgfile SOURCE [-o OUTPUT] set NAME=VALUE [NAME=VALUE] ...\n"
    " cfgfile SOURCE [-o OUTPUT] unset NAME [NAME] ...\n"
    " cfgfile SOURCE [-o OUTPUT] export\n"    
    "    SOURCE -- source config file\n"
    "    OUTPUT -- if OUTPUT is not sepcified, it will print to stdand output\n"
    ;

int main(int argc, char **argv)
{
    const char *pSource = NULL, *pOutput = NULL;
    int i, command = 0, done = 0, invalid = 0;
    int cmdIndex;
    FILE *fp;
    configHead_t *head = NULL;

    if (argc > 1)
    {
        pSource = argv[1];

        fp = fopen(pSource, "r");
        if (fp == NULL)
        {
            fprintf(stderr, "***Open %s failed!\n", pSource);
            pSource = NULL;
        }        
        else 
        {
            fclose(fp);
        }
    }

    if ((argc > 2) && !strcmp(argv[2], "-o"))
    {
        if (argc >= 4)
        {
            pOutput = argv[3];
                       
        }
        else 
        {
            invalid = 1;
        }
    }    

    cmdIndex = pOutput ? 4 : 2;

    if (argc > cmdIndex)
    {
        if (!strcmp(argv[cmdIndex], "set"))
        {
            /* need at lease one name=value */
            if (argc <= cmdIndex + 1)
            {
                invalid = 1;
            }
            command = 1;
        }
        else if (!strcmp(argv[cmdIndex], "unset"))
        {
            if (argc <= cmdIndex + 1)
            {
                invalid = 1;
            }        
            command = 2;
        }        
        else if (!strcmp(argv[cmdIndex], "export"))
        {
            command = 3;
        }                
    }


    if (!invalid && pSource && command)
    {
        if ((command == 1))
        {
            head = configFileParse(pSource);
            if (head)
            {
                for (i = cmdIndex + 1; i < argc; i ++)
                {
                    configLineSet(*head, argv[i]);
                }

                configFileWrite(pOutput, *head);

                configHeadFree(*head);

                done = 1;
            }
        }
        else if (command == 2)
        {
            head = configFileParse(pSource);
            if (head)
            {
                for (i = cmdIndex + 1; i < argc; i ++)
                {
                    configItemUnset(*head, argv[i]);
                }

                configFileWrite(pOutput, *head);

                configHeadFree(*head);

                done = 1;
            }       
        }
        else if (command == 3)
        {
            head = configFileParse(pSource);
            if (head)
            {
                configFileOutput(pOutput, *head, 1);

                configHeadFree(*head);

                done = 1;
            }                     
        }
    }


    if ((argc > cmdIndex) && !command)
    {
        printf("***Unknown Command:%s\n", argv[cmdIndex]);        
    }
    
    if (!done)
    {
        printf("%s", s_usage);
        return 1;
    }

    return 0;
}

#endif 

