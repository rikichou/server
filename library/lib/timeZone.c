
/*
Copyright (c) 2014,  Shenzhen Hexicom Technologies Co., Ltd.
All rights reserved.

File         : timeZone.c
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

#include "timeZone.h"

typedef struct
{
    int index;
    char *timeDiff;
    char *zoneInfo;
}timeZone_t;


static int s_timeZoneSize = 0;
static const timeZone_t s_timeZones[] = 
{
    {1,"-12:00","Eniwetok, Kwajalein"},
    {2,"-11:00","Midway Island, Samoa"},
    {3,"-10:00","Hawaii"},
    {4,"-9:00","Alaska"},
    {5,"-8:00","Pacific Time (US & Canada)"},
    {6,"-7:00","Mountain Time (US & Canada)"},
    {7,"-6:00","Central Time (US & Canada), Mexico City"},
    {8,"-5:00","Eastern Time (US & Canada), Bogota, Lima"},
    {9,"-4:00","Atlantic Time (Canada), Caracas, La Paz"},
    {10,"-3:30","Newfoundland"},
    {11,"-3:00","Brazil, Buenos Aires, Georgetown"},
    {12,"-2:00","Mid-Atlantic"},
    {13,"-1:00","Azores, Cape Verde Islands"},
    {14,"+0:00","Western Europe Time, London, Lisbon, Casablanca"},
    {15,"+1:00","Brussels, Copenhagen, Madrid, Paris"},
    {16,"+2:00","Kaliningrad, South Africa"},
    {17,"+3:00","Baghdad, Riyadh, Moscow, St. Petersburg"},
    {18,"+3:30","Tehran"},
    {19,"+4:00","Abu Dhabi, Muscat, Baku, Tbilisi"},
    {20,"+4:30","Kabul"},
    {21,"+5:00","Ekaterinburg, Islamabad, Karachi, Tashkent"},
    {22,"+5:30","Bombay, Calcutta, Madras, New Delhi"},
    {23,"+5:45","Kathmandu"},
    {24,"+6:00","Almaty, Dhaka, Colombo"},
    {25,"+7:00","Bangkok, Hanoi, Jakarta"},
    {26,"+8:00","Beijing, Perth, Singapore, Hong Kong"},
    {27,"+9:00","Tokyo, Seoul, Osaka, Sapporo, Yakutsk"},
    {28,"+9:30","Adelaide, Darwin"},
    {29,"+10:00","Eastern Australia, Guam, Vladivostok"},
    {30,"+11:00","Magadan, Solomon Islands, New Caledonia"},
    {31,"+12:00","Auckland, Wellington, Fiji, Kamchatka"},
    {0,NULL,NULL}
};

/*
    return size of data
*/
int timeZoneInfoListGet(char *data, int size)
{
    int i;
    int len = 0;
    const timeZone_t *pTz;
    
    if (data && (size > 0))
    {
        pTz = &s_timeZones[0];
        for (i = 0; pTz->index && (i < sizeof(s_timeZones)/sizeof(s_timeZones[0])); i ++)
        {
            if (len < size)
            {
                len += snprintf(data + len, size - len, "(%d)[%s]%s\n", pTz->index, pTz->timeDiff, pTz->zoneInfo);
            }  
            pTz ++;
        }        
    }
    s_timeZoneSize = len;

    return len;
}


int timeZoneInfoListSize(void)
{
    int i;
    static int len = 0;
    const timeZone_t *pTz;

    if (s_timeZoneSize)
    {
        return s_timeZoneSize;
    }

    if (len == 0)
    {
        pTz = &s_timeZones[0];
        for (i = 0; pTz->index && (i < sizeof(s_timeZones)/sizeof(s_timeZones[0])); i ++)
        {
            len += (4 + 4 + strlen(pTz->timeDiff) + strlen(pTz->zoneInfo));
            pTz ++;
        }        
    }

    return len;
}

int timeZoneNum(void)
{
    return sizeof(s_timeZones)/ sizeof(s_timeZones[0]) - 1;
}


/* return seconds offset of time zone */
int32_t timeZoneOffsetSeconds(int zoneId)
{
    int i = 0, h, m, s;
    char *p;

    do 
    {
        if (s_timeZones[i].index == zoneId)
        {
            h = atoi(s_timeZones[i].timeDiff);
            m = 0;
            p = strchr(s_timeZones[i].timeDiff, ':');
            if (p && *(p + 1))
            {
                m = strtoul(p + 1, NULL, 0); 
            }
            s = (abs(h) * 60 * 60) + (m * 60);

            if (h < 0)
            {
                return s * -1;    
            }
            else 
            {
                return s;
            }
        } 
        i ++;
    }while(s_timeZones[i].index > 0);
    return 0;    
}

const char *timeZoneInfoGet(int zoneId, char *buffer, int length)
{
    int i = 0;

    buffer[0] = '\0';
    do 
    {
        if (s_timeZones[i].index == zoneId)
        {
            snprintf(buffer, length, "(%d)[%s]%s", s_timeZones[i].index, s_timeZones[i].timeDiff, s_timeZones[i].zoneInfo);
            return buffer;
        } 
        i ++;
    }while(s_timeZones[i].index > 0);    

    return buffer;
}


int validTimeZoneId(int zoneId)
{
    return ((zoneId > 0) && (zoneId <= timeZoneNum())) ? 1 : 0;
}


